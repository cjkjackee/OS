// Student ID: 0416111
// Name      : 李欣洳 LEE SIN LU
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

int imgWidth[5], imgHeight[5];
int FILTER_SIZE;
int FILTER_SCALE;
int *filter_G;

pthread_t tid[5];
pthread_mutex_t lock;

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

unsigned char RGB2grey(int w, int h, int c, unsigned char* pic_in)
{
	int tmp = (
		pic_in[3 * (h*imgWidth[c] + w) + MYRED] +
		pic_in[3 * (h*imgWidth[c] + w) + MYGREEN] +
		pic_in[3 * (h*imgWidth[c] + w) + MYBLUE] )/3;

	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}

unsigned char GaussianFilter(int w, int h , int c, unsigned char* pic_grey)
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
		if (a<0 || b<0 || a>imgWidth[c] || b>imgHeight[c]) continue;

		tmp += filter_G[j*ws + i] * pic_grey[b*imgWidth[c] + a];
	};
	tmp /= FILTER_SCALE;
	if (tmp < 0) tmp = 0;
	if (tmp > 255) tmp = 255;
	return (unsigned char)tmp;
}


void* Threading(void* rank)
{
	pthread_mutex_trylock(&lock);

	unsigned char *pic_in, *pic_grey, *pic_blur, *pic_final;
	
	int c = *(int*)rank;
	BmpReader* bmpReader = new BmpReader();
	// read input BMP file
	pic_in = bmpReader->ReadBMP(inputfile_name[c], &imgWidth[c], &imgHeight[c]);
	// allocate space for output image
	pic_grey = (unsigned char*)malloc(imgWidth[c]*imgHeight[c]*sizeof(unsigned char));
	pic_blur = (unsigned char*)malloc(imgWidth[c]*imgHeight[c]*sizeof(unsigned char));
	pic_final = (unsigned char*)malloc(3 * imgWidth[c]*imgHeight[c]*sizeof(unsigned char));
	
	//convert RGB image to grey image
	for (int j = 0; j<imgHeight[c]; j++) {
		for (int i = 0; i<imgWidth[c]; i++){
			pic_grey[j*imgWidth[c] + i] = RGB2grey(i, j , c, pic_in);
		}
	}

	//apply the Gaussian filter to the image
	for (int j = 0; j<imgHeight[c]; j++) {
		for (int i = 0; i<imgWidth[c]; i++){
			pic_blur[j*imgWidth[c] + i] = GaussianFilter(i, j , c, pic_grey);
		}
	}

	//extend the size form WxHx1 to WxHx3
	for (int j = 0; j<imgHeight[c]; j++) {
		for (int i = 0; i<imgWidth[c]; i++){
			pic_final[3 * (j*imgWidth[c] + i) + MYRED] = pic_blur[j*imgWidth[c] + i];
			pic_final[3 * (j*imgWidth[c] + i) + MYGREEN] = pic_blur[j*imgWidth[c] + i];
			pic_final[3 * (j*imgWidth[c] + i) + MYBLUE] = pic_blur[j*imgWidth[c] + i];
		}
	}

	// write output BMP file
	bmpReader->WriteBMP(outputBlur_name[c], imgWidth[c], imgHeight[c], pic_final);

	//free memory space
	free(pic_in);
	free(pic_grey);
	free(pic_blur);
	free(pic_final);

	pthread_mutex_unlock(&lock);
	pthread_exit(0);

}

int main()
{		// read mask file
	FILE* mask;
	mask = fopen("mask_Gaussian.txt", "r");
	fscanf(mask, "%d", &FILTER_SIZE);
	fscanf(mask, "%d", &FILTER_SCALE);

	filter_G = new int[FILTER_SIZE];
	for (int i = 0; i<FILTER_SIZE; i++)
		fscanf(mask, "%d", &filter_G[i]);
	fclose(mask);

	int a[5];
	for(int i=0;i<5;i++) a[i]=i;

	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

	for(int i=0;i<5;i++) pthread_create(&tid[i], NULL, Threading, &a[i]);
	for(int i=4;i>=0;i--) pthread_join(tid[i],NULL);
	pthread_mutex_destroy(&lock);

	return 0;
}