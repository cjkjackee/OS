// Student ID: 0416249
// Name      : Chan Jack Kee
// Date      : 2017.11.03

#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond;
int imgWidth[5], imgHeight[5];
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_G;

const char *inputfile_name[5] = {
	"input1.bmp",
	"input2.bmp",
	"input3.bmp",
	"input4.bmp",
	"input5.bmp"
};
const char *outputBlur_name[5] = {
	"Blur1.bmp",
	"Blur2.bmp",
	"Blur3.bmp",
	"Blur4.bmp",
	"Blur5.bmp"
};
/*
const char *outputSobel_name[5] = {
	"Sobel1.bmp",
	"Sobel2.bmp",
	"Sobel3.bmp",
	"Sobel4.bmp",
	"Sobel5.bmp"
};*/

unsigned char RGB2grey(int w, int h, int k, unsigned char* pic_in)
{
	int tmp = (
		pic_in[3 * (h*imgWidth[k] + w) + MYRED] +
		pic_in[3 * (h*imgWidth[k] + w) + MYGREEN] +
		pic_in[3 * (h*imgWidth[k] + w) + MYBLUE] )/3;

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char GaussianFilter(int w, int h, int k, unsigned char* pic_grey)
{
	int tmp = 0;
	int a, b;
	int ws = (int)sqrt((float)FILTER_SIZE);
	for (int j = 0; j<ws; j++)
	for (int i = 0; i<ws; i++)
	{
		a = w + i - (ws / 2);
		b = h + j - (ws / 2);

		// detect for borders of the image
		if (a<0 || b<0 || a>=imgWidth[k] || b>=imgHeight[k]) continue;

		tmp += filter_G[j*ws + i] * pic_grey[b*imgWidth[k] + a];
	};
	tmp /= FILTER_SCALE;
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

struct thread_arg
{
	int beginH, endH;
	int k;
	int imgW;
	unsigned char* pic_in;
	unsigned char* pic_tmp;
	unsigned char* pic_out;
	sem_t* semaphore;
};

void* thread_rgb2grey(void* arg)
{
	struct thread_arg x = *((struct thread_arg*)arg);	

	sem_wait(x.semaphore);

	for (int j = x.beginH; j<x.endH; j++) {
		for (int i = 0; i<x.imgW; i++){
			x.pic_tmp[j*x.imgW + i] = RGB2grey(i, j, x.k, x.pic_in);
		}
	}

	//apply the Gaussian filter to the image	
	for (int j = x.beginH; j<x.endH; j++) {
		for (int i = 0; i<x.imgW; i++){
			x.pic_out[j*x.imgW + i] = GaussianFilter(i, j, x.k, x.pic_tmp);
		}
	}

	sem_post(x.semaphore);
	
	pthread_exit(NULL);
}

void* thread_output(void* arg)
{
	struct thread_arg x = *((struct thread_arg*)arg);

	sem_wait(x.semaphore);
	//extend the size form WxHx1 to WxHx3
	for (int j = x.beginH; j<x.endH; j++) {
		for (int i = 0; i<x.imgW; i++){
			x.pic_out[3 * (j*x.imgW + i) + MYRED] = x.pic_in[j*x.imgW + i];
			x.pic_out[3 * (j*x.imgW + i) + MYGREEN] = x.pic_in[j*x.imgW + i];
			x.pic_out[3 * (j*x.imgW + i) + MYBLUE] = x.pic_in[j*x.imgW + i];
		}
	}
	sem_post(x.semaphore);


	pthread_exit(NULL);
}

void* thread_func (void* arg)
{
	sem_t sem1, sem2, sem3;
	sem_t fout;
	pthread_t rgb1, rgb2, rgb3;
	pthread_t out1, out2, out3;
	unsigned char *pic_in, *pic_grey, *pic_blur, *pic_final;
	int *x = (int *)arg;
	int k = *x;
	
	sem_init(&sem1,1,1);
	sem_init(&sem2,1,1);
	sem_init(&sem3,1,1);
	BmpReader* bmpReader = new BmpReader();
	// read input BMP file
	pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth[k], &imgHeight[k]);
	// allocate space for output image
	pic_grey = (unsigned char*)malloc(imgWidth[k]*imgHeight[k]*sizeof(unsigned char));
	pic_blur = (unsigned char*)malloc(imgWidth[k]*imgHeight[k]*sizeof(unsigned char));
	pic_final = (unsigned char*)malloc(3 * imgWidth[k]*imgHeight[k]*sizeof(unsigned char));
	
	//convert RGB image to grey image
	int ew = imgWidth[k]/3;
	int eh = imgHeight[k]/3;
	struct thread_arg x1, x2, x3;

	x1.beginH = 0;
	x1.endH = eh;
	x1.k = k;
	x1.imgW = imgWidth[k];
	x1.pic_in = pic_in;
	x1.pic_tmp = pic_grey;
	x1.pic_out = pic_blur;
	x1.semaphore = &sem1;
	pthread_create(&rgb1, NULL, thread_rgb2grey, &x1);

	memcpy(&x2,&x1,sizeof(x2));
	x2.beginH = x1.endH;
	x2.endH = x2.beginH + eh;
	x2.semaphore = &sem2;
	pthread_create(&rgb2, NULL, thread_rgb2grey, &x2);

	memcpy(&x3,&x1,sizeof(x3));
	x3.beginH = x2.endH;
	x3.endH = imgHeight[k];
	x3.semaphore = &sem3;
	pthread_create(&rgb3, NULL, thread_rgb2grey, &x3);

	pthread_join(rgb1, NULL);

	struct thread_arg n1, n2, n3;
	memcpy(&n1, &x1, sizeof(n1));
	n1.pic_in = pic_blur;
	n1.pic_out = pic_final;
	pthread_create(&out1, NULL, thread_output, &n1);

	memcpy(&n2, &x2, sizeof(n2));
	n2.pic_in = pic_blur;
	n2.pic_out = pic_final;
	pthread_create(&out2, NULL, thread_output, &n2);

	memcpy(&n3, &x3, sizeof(n3));
	n3.pic_in = pic_blur;
	n3.pic_out = pic_final;
	pthread_create(&out3, NULL, thread_output, &n3);
	
	pthread_join(out3, NULL);
	pthread_join(out2,NULL);
	pthread_join(out1, NULL);
	// write output BMP file
	bmpReader->WriteBMP(outputBlur_name[k], imgWidth[k], imgHeight[k], pic_final);

	//free memory space
	free(pic_in);
	free(pic_blur);
	free(pic_final);
	pthread_exit(NULL);
}

int main()
{
	pthread_t thread0, thread1, thread2, thread3, thread4;
	int a1, a2, a3, a4, a5;
	// read mask file
	FILE* mask;
	mask = fopen("mask_Gaussian.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	fscanf(mask, "%d", &FILTER_SCALE);

	filter_G = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);


	a1 = 0;
	a2 = 1;
	a3 = 2;
	a4 = 3;
	a5 = 4;
	pthread_create(&thread0, NULL, thread_func, &a1);
	pthread_create(&thread1, NULL, thread_func, &a2);
	pthread_create(&thread2, NULL, thread_func, &a3);
	pthread_create(&thread3, NULL, thread_func, &a4);
	pthread_create(&thread4, NULL, thread_func, &a5);

	pthread_join(thread4,NULL);
	pthread_join(thread3,NULL);
	pthread_join(thread2,NULL);
	pthread_join(thread1,NULL);
	pthread_join(thread0,NULL);

	return 0;
}