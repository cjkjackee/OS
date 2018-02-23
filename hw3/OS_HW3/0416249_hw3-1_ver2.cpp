// Student ID: 0416249	
// Name      : Chan Jack Kee
// Date      : 2017.11.03

#include "bmpReader.h"
#include "bmpReader.cpp"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;

#define MYRED	2
#define MYGREEN 1
#define MYBLUE	0

BmpReader* bmpReader = new BmpReader();
int imgWidth[5], imgHeight[5];
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_G;
pthread_mutex_t inLock, greyLock, blurLock, finalLock;

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

unsigned char *pic_in[5], *pic_grey[5], *pic_blur[5], *pic_final[5];

unsigned char RGB2grey(int w, int h, int k)
{
	int tmp = (
		pic_in[3 * (h*imgWidth[k] + w) + MYRED] +
		pic_in[3 * (h*imgWidth[k] + w) + MYGREEN] +
		pic_in[3 * (h*imgWidth[k] + w) + MYBLUE] )/3;

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char GaussianFilter(int w, int h, int k)
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

void* thread_func (void* arg)
{
	int k = *((int*)arg);
	
	// read input BMP file		
	pthread_mutex_lock(&inLock);
	cout << "read pic now :" << k << endl;
	pic_in = bmpReader->ReadBMP(inputfile_name[k], &imgWidth[k], &imgHeight[k]);	

	pthread_mutex_lock(&greyLock);
	cout << "rgb to grey :" << k << endl;
	//convert RGB image to grey image
	for (int j = 0; j<imgHeight[k]; j++) {
		for (int i = 0; i<imgWidth[k]; i++){
			pic_grey[j*imgWidth[k] + i] = RGB2grey(i, j, k);
		}
	}
	pthread_mutex_unlock(&inLock);

	pthread_mutex_lock(&blurLock);
	cout << "grey to blur :" << k << endl;
	//apply the Gaussian filter to the image
	for (int j = 0; j<imgHeight[k]; j++) {
		for (int i = 0; i<imgWidth[k]; i++){
			pic_blur[j*imgWidth[k] + i] = GaussianFilter(i, j, k);
		}
	}
	pthread_mutex_unlock(&greyLock);

	pthread_mutex_lock(&finalLock);
	cout << "blur to final :"  << k << endl;
	//extend the size form WxHx1 to WxHx3
	for (int j = 0; j<imgHeight[k]; j++) {
		for (int i = 0; i<imgWidth[k]; i++){
			pic_final[3 * (j*imgWidth[k] + i) + MYRED] = pic_blur[j*imgWidth[k] + i];
			pic_final[3 * (j*imgWidth[k] + i) + MYGREEN] = pic_blur[j*imgWidth[k] + i];
			pic_final[3 * (j*imgWidth[k] + i) + MYBLUE] = pic_blur[j*imgWidth[k] + i];
		}
	}
	pthread_mutex_unlock(&blurLock);

	// write output BMP file
	cout << "final to output :" << k << endl;
	bmpReader->WriteBMP(outputBlur_name[k], imgWidth[k], imgHeight[k], pic_final);
	pthread_mutex_unlock(&finalLock);
	
	pthread_exit(NULL);
}

#define max 20000

int main()
{
	//thread
	pthread_t p0, p1, p2, p3, p4;
	int n0=0, n1=1, n2=2, n3=3, n4=4;
	// read mask file
	FILE* mask;
	mask = fopen("mask_Gaussian.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	fscanf(mask, "%d", &FILTER_SCALE);

	filter_G = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);

	// allocate space for output image
	pic_grey = (unsigned char*)malloc(max*max*sizeof(unsigned char));
	pic_blur = (unsigned char*)malloc(max*max*sizeof(unsigned char));
	pic_final = (unsigned char*)malloc(3 *max*max*sizeof(unsigned char));
	
	pthread_mutex_unlock(&inLock);
	pthread_mutex_unlock(&greyLock);
	pthread_mutex_unlock(&blurLock);
	pthread_mutex_unlock(&finalLock);

	pthread_create(&p0, NULL, thread_func, &n0);
	pthread_create(&p1, NULL, thread_func, &n1);
	pthread_create(&p2, NULL, thread_func, &n2);
	pthread_create(&p3, NULL, thread_func, &n3);
	pthread_create(&p4, NULL, thread_func, &n4);
	pthread_join(p4,NULL);
	pthread_join(p3,NULL);
	pthread_join(p2,NULL);
	pthread_join(p1,NULL);
	pthread_join(p0,NULL);

	//free memory space
	free(pic_in);
	free(pic_grey);
	free(pic_blur);
	free(pic_final);
	return 0;
}