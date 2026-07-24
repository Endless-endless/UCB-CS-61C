/************************************************************************
**
** NAME:        imageloader.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**              Justin Yokota - Starter Code
**				YOUR NAME HERE
**				Jensen Wu
**
** DATE:        2020-08-15
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "imageloader.h"

//Opens a .ppm P3 image file, and constructs an Image object. 
//You may find the function fscanf useful.
//Make sure that you close the file with fclose before returning.
Image *readData(char *filename) 
{
	//YOUR CODE HERE
	FILE*  fp = fopen(filename,"r");
	if (fp == NULL) return NULL;

	char format[4];
	uint32_t width,height;
	int maxval;
	fscanf(fp,"%s",format);
	fscanf(fp,"%u %u",&width,&height);
	fscanf(fp,"%d",&maxval);

	Image *img = malloc(sizeof(Image));
	if (img == NULL) {
		fclose(fp);
		return NULL;
	}
	img -> cols = width;
	img -> rows	= height;
	img -> image = malloc(height * sizeof(Color*));
		if (img->image == NULL) {
		free(img);
		fclose(fp);
		return NULL;
	}

	for (uint32_t y = 0; y < height; y++)
	{
		img->image[y] = malloc(width*sizeof(Color));
		for (uint32_t x = 0; x < width; x++)
		{
			int r,g,b;
			fscanf(fp,"%d %d %d",&r,&g,&b);
			img->image[y][x].R = (uint8_t)r;
			img->image[y][x].G = (uint8_t)g;
			img->image[y][x].B = (uint8_t)b;
		}
	}

	fclose(fp);
	return img;
}

//Given an image, prints to stdout (e.g. with printf) a .ppm P3 file with the image's data.
void writeData(Image *image)
{
	//YOUR CODE HERE
	if (image == NULL) return;

	uint32_t width = image -> cols;
	uint32_t height = image -> rows;

	printf("P3\n");
	printf("%d %d\n",width,height);
	printf("255\n");

	for (uint32_t y = 0; y < height; y++)
	{
		Color p0 = image->image[y][0];
		printf("%d %d %d", p0.R, p0.G, p0.B);
		for (uint32_t x = 1; x < width; x++)
		{
			Color p = image->image[y][x];
			printf(" %d %d %d", p.R, p.G, p.B);
		}
		printf("\n");
	}
}

//Frees an image
void freeImage(Image *image)
{
	//YOUR CODE HERE
	if (image == NULL) return;
	
	for (uint32_t y = 0; y < image -> rows; y++)
	{
		free(image->image[y]);
	}

	free(image->image);
	free(image);
}