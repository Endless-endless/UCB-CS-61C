/************************************************************************
**
** NAME:        gameoflife.c
**
** DESCRIPTION: CS61C Fall 2020 Project 1
**
** AUTHOR:      Justin Yokota - Starter Code
**				YOUR NAME HERE
**				Jensen WU
**
** DATE:        2020-08-23
**
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "imageloader.h"

//Determines what color the cell at the given row/col should be. This function allocates space for a new Color.
//Note that you will need to read the eight neighbors of the cell in question. The grid "wraps", so we treat the top row as adjacent to the bottom row
//and the left column as adjacent to the right column.
Color *evaluateOneCell(Image *image, int row, int col, uint32_t rule)
{
	uint32_t w = image->cols;
	uint32_t h = image->rows;
	// 8个邻居偏移量不变
	int dir[8][2] = {{-1,-1},{-1,0},{-1,1},
					 {0,-1},        {0,1},
					 {1,-1}, {1,0}, {1,1}};

	// 取出当前像素原始RGB 8bit数值
	uint8_t selfR = image->image[row][col].R;
	uint8_t selfG = image->image[row][col].G;
	uint8_t selfB = image->image[row][col].B;

	uint8_t newR = 0, newG = 0, newB = 0;

	// 遍历0~7每一个bit位，分别计算R/G/B通道该bit下一代状态
	for (int bitPos = 0; bitPos < 8; bitPos++)
	{
		// ========== 1. 计算R通道当前bit ==========
		// 统计8邻居R通道同一bit为1的数量
		int rAliveCnt = 0;
		for (int d = 0; d < 8; d++)
		{
			int nr = (row + dir[d][0] + h) % h;
			int nc = (col + dir[d][1] + w) % w;
			uint8_t neiR = image->image[nr][nc].R;
			if ((neiR >> bitPos) & 1)
				rAliveCnt++;
		}
		// 当前bit存活状态
		int rSelfAlive = (selfR >> bitPos) & 1;
		int rNextBit = 0;
		if (rSelfAlive)
			rNextBit = (rule >> (9 + rAliveCnt)) & 1;
		else
			rNextBit = (rule >> rAliveCnt) & 1;
		// 把新bit写入newR对应位置
		if (rNextBit)
			newR |= (1 << bitPos);

		// ========== 2. 计算G通道当前bit ==========
		int gAliveCnt = 0;
		for (int d = 0; d < 8; d++)
		{
			int nr = (row + dir[d][0] + h) % h;
			int nc = (col + dir[d][1] + w) % w;
			uint8_t neiG = image->image[nr][nc].G;
			if ((neiG >> bitPos) & 1)
				gAliveCnt++;
		}
		int gSelfAlive = (selfG >> bitPos) & 1;
		int gNextBit = 0;
		if (gSelfAlive)
			gNextBit = (rule >> (9 + gAliveCnt)) & 1;
		else
			gNextBit = (rule >> gAliveCnt) & 1;
		if (gNextBit)
			newG |= (1 << bitPos);

		// ========== 3. 计算B通道当前bit ==========
		int bAliveCnt = 0;
		for (int d = 0; d < 8; d++)
		{
			int nr = (row + dir[d][0] + h) % h;
			int nc = (col + dir[d][1] + w) % w;
			uint8_t neiB = image->image[nr][nc].B;
			if ((neiB >> bitPos) & 1)
				bAliveCnt++;
		}
		int bSelfAlive = (selfB >> bitPos) & 1;
		int bNextBit = 0;
		if (bSelfAlive)
			bNextBit = (rule >> (9 + bAliveCnt)) & 1;
		else
			bNextBit = (rule >> bAliveCnt) & 1;
		if (bNextBit)
			newB |= (1 << bitPos);
	}

	// 分配新像素，填充计算完成的RGB
	Color* next_cell = malloc(sizeof(Color));
	next_cell->R = newR;
	next_cell->G = newG;
	next_cell->B = newB;
	return next_cell;
}


//The main body of Life; given an image and a rule, computes one iteration of the Game of Life.
//You should be able to copy most of this from steganography.c
Image *life(Image *image, uint32_t rule)
{
	//YOUR CODE HERE
	Image* next_img = malloc(sizeof(Image));
	uint32_t w = image->cols;
	uint32_t h = image->rows;
	next_img->cols = w;
	next_img->rows = h;
	next_img->image = malloc(h * sizeof(Color*));

	for (uint32_t y = 0; y < h; y++)
	{
		next_img->image[y] = malloc(w*sizeof(Color));
		for (uint32_t x = 0; x < w; x++)
		{
			Color* cur_cell = evaluateOneCell(image,y,x,rule);
			next_img->image[y][x] = *cur_cell;
			free(cur_cell);
		}
	}

	return next_img;
}

/*
Loads a .ppm from a file, computes the next iteration of the game of life, then prints to stdout the new image.

argc stores the number of arguments.
argv stores a list of arguments. Here is the expected input:
argv[0] will store the name of the program (this happens automatically).
argv[1] should contain a filename, containing a .ppm.
argv[2] should contain a hexadecimal number (such as 0x1808). Note that this will be a string.
You may find the function strtol useful for this conversion.
If the input is not correct, a malloc fails, or any other error occurs, you should exit with code -1.
Otherwise, you should return from main with code 0.
Make sure to free all memory before returning!

You may find it useful to copy the code from steganography.c, to start.
*/
int main(int argc, char **argv)
{
	//YOUR CODE HERE
	if (argc != 3)
	{
		printf("usage: ./gameOfLife filename rule\n");
        printf("filename is an ASCII PPM file (type P3) with maximum value 255.\n");
        printf("rule is a hex number beginning with 0x; Life is 0x1808.\n");
		return -1;
	}

	Image* origin = readData(argv[1]);
	if (origin == NULL)
	{
		return-1;
	}

	uint32_t rule = strtol(argv[2], NULL, 16);
	
	Image* next_gen = life(origin,rule);
	if (next_gen == NULL)
	{
		freeImage(origin);
		return -1;
	}

	writeData(next_gen);

	freeImage(origin);
	freeImage(next_gen);

	return 0;
}
