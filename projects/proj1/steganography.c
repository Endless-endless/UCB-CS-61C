/************************************************************************
**
** NAME:        steganography.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Dan Garcia  -  University of California at Berkeley
**              Copyright (C) Dan Garcia, 2020. All rights reserved.
**				Justin Yokota - Starter Code
**				YOUR NAME HERE Jemsem Wu
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

//Determines what color the cell at the given row/col should be. This should not affect Image, and should allocate space for a new Color.
Color *evaluateOnePixel(Image *image, int row, int col)
{
	//YOUR CODE HERE
	uint8_t bit = image->image[row][col].B & 1;

    Color *res = malloc(sizeof(Color));
    if (bit == 0)
    {
        res->R = 0;
        res->G = 0;
        res->B = 0;
    }
    else
    {
        res->R = 255;
        res->G = 255;
        res->B = 255;
    }
    return res;
}

//Given an image, creates a new image extracting the LSB of the B channel.
Image *steganography(Image *image)
{
	//YOUR CODE HERE
    uint32_t width = image->cols;
    uint32_t height = image->rows;

    Image *ret = malloc(sizeof(Image));
    ret -> cols = width;
    ret -> rows = height;
    ret -> image = malloc(height*sizeof(Color*));

    for (uint32_t y = 0; y < height; y++)
    {
        ret->image[y] = malloc(width * sizeof(Color));
        for (uint32_t x = 0; x < width; x++)
        {
            Color* new_pixel = evaluateOnePixel(image,y,x);
            ret -> image[y][x] = *new_pixel;
            free(new_pixel);
        }
    }
    return ret;
}

/*
Loads a file of ppm P3 format from a file, and prints to stdout (e.g. with printf) a new image, 
where each pixel is black if the LSB of the B channel is 0, 
and white if the LSB of the B channel is 1.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a file of ppm P3 format (not necessarily with .ppm file extension).
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!
*/
int main(int argc, char **argv)
{
	//YOUR CODE HERE
    if (argc != 2)
    {
        return -1;
    }

    Image* origin_img = readData(argv[1]);
    if (origin_img == NULL)
    {
        return -1;
    }

    Image* secret_img = steganography(origin_img);
    if (secret_img == NULL)
    {
        freeImage(origin_img);
        return -1;
    }   

    writeData(secret_img);

    freeImage(origin_img);
    freeImage(secret_img);

    return 0;
}
