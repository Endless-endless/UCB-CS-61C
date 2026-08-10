#include <time.h>
#include <stdio.h>
#include <x86intrin.h>
#include "simd.h"

long long int sum(int vals[NUM_ELEMS]) {
	clock_t start = clock();

	long long int sum = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS; i++) {
			if(vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	long long int sum = 0;

	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		for(unsigned int i = 0; i < NUM_ELEMS / 4 * 4; i += 4) {
			if(vals[i] >= 128) sum += vals[i];
			if(vals[i + 1] >= 128) sum += vals[i + 1];
			if(vals[i + 2] >= 128) sum += vals[i + 2];
			if(vals[i + 3] >= 128) sum += vals[i + 3];
		}

		//This is what we call the TAIL CASE
		//For when NUM_ELEMS isn't a multiple of 4
		//NONTRIVIAL FACT: NUM_ELEMS / 4 * 4 is the largest multiple of 4 less than NUM_ELEMS
		for(unsigned int i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++) {
			if (vals[i] >= 128) {
				sum += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return sum;
}

long long int sum_simd(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);		// This is a vector with 127s in it... Why might you need this?
	long long int result = 0;				   // This is where you should put your final result!
	/* DO NOT DO NOT DO NOT DO NOT WRITE ANYTHING ABOVE THIS LINE. */
	
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		/* YOUR CODE GOES HERE */
		__m128i sum_vec = _mm_setzero_si128();

		unsigned int i = 0;
		for (i = 0; i < NUM_ELEMS / 4 * 4; i += 4)
		{
			__m128i vals_vec = _mm_loadu_si128((__m128i*)&vals[i]);

			__m128i mask = _mm_cmpgt_epi32(vals_vec, _127);

			__m128i filtered = _mm_and_si128(vals_vec, mask);

			sum_vec = _mm_add_epi32(sum_vec, filtered);
		}

		int temp[4];
		_mm_storeu_si128((__m128i*)temp, sum_vec);
		
		result += temp[0];
		result += temp[1];
		result += temp[2];
		result += temp[3];

		/* You'll need a tail case. */
		for (i = NUM_ELEMS / 4 * 4; i < NUM_ELEMS; i++)
		{
			if (vals[i] >= 128)
			{
				result += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}

long long int sum_simd_unrolled(int vals[NUM_ELEMS]) {
	clock_t start = clock();
	__m128i _127 = _mm_set1_epi32(127);
	long long int result = 0;
	for(unsigned int w = 0; w < OUTER_ITERATIONS; w++) {
		/* COPY AND PASTE YOUR sum_simd() HERE */
		/* MODIFY IT BY UNROLLING IT */
		__m128i sum_vec = _mm_setzero_si128();
		__m128i sum_vec0 = _mm_setzero_si128();
		__m128i sum_vec1 = _mm_setzero_si128();
		__m128i sum_vec2 = _mm_setzero_si128();
		__m128i sum_vec3 = _mm_setzero_si128();

		unsigned int i = 0;
		for (i = 0; i < NUM_ELEMS / 16 * 16; i += 16)
		{
			__m128i vals0_vec = _mm_loadu_si128((__m128i*)&vals[i]);
			__m128i mask0 = _mm_cmpgt_epi32(vals0_vec, _127);
			__m128i filtered0 = _mm_and_si128(vals0_vec, mask0);
			sum_vec0 = _mm_add_epi32(sum_vec0, filtered0);

			__m128i vals1_vec = _mm_loadu_si128((__m128i*)&vals[i+4]);
			__m128i mask1 = _mm_cmpgt_epi32(vals1_vec, _127);
			__m128i filtered1 = _mm_and_si128(vals1_vec, mask1);
			sum_vec1 = _mm_add_epi32(sum_vec1, filtered1);

			__m128i vals2_vec = _mm_loadu_si128((__m128i*)&vals[i+8]);
			__m128i mask2 = _mm_cmpgt_epi32(vals2_vec, _127);
			__m128i filtered2 = _mm_and_si128(vals2_vec, mask2);
			sum_vec2 = _mm_add_epi32(sum_vec2, filtered2);

			__m128i vals3_vec = _mm_loadu_si128((__m128i*)&vals[i+12]);
			__m128i mask3 = _mm_cmpgt_epi32(vals3_vec, _127);
			__m128i filtered3 = _mm_and_si128(vals3_vec, mask3);
			sum_vec3 = _mm_add_epi32(sum_vec3, filtered3);
		}

		sum_vec0 = _mm_add_epi32(sum_vec0, sum_vec1);
        sum_vec2 = _mm_add_epi32(sum_vec2, sum_vec3);
        sum_vec0 = _mm_add_epi32(sum_vec0, sum_vec2);

		/* You'll need 1 or maybe 2 tail cases here. */
		for (; i < NUM_ELEMS / 4 * 4; i += 4)
		{
			__m128i vals_vec = _mm_loadu_si128((__m128i*)&vals[i]);
			__m128i mask = _mm_cmpgt_epi32(vals_vec, _127);
			__m128i filtered = _mm_and_si128(vals_vec, mask);
			sum_vec = _mm_add_epi32(sum_vec, filtered);
		}
		sum_vec0 = _mm_add_epi32(sum_vec0, sum_vec);
			
		int temp[4];
		_mm_storeu_si128((__m128i*)temp, sum_vec0);
			
		result += temp[0];
		result += temp[1];
		result += temp[2];
		result += temp[3];

		for (; i < NUM_ELEMS; i++)
		{
			if (vals[i] >= 128)
			{
				result += vals[i];
			}
		}
	}
	clock_t end = clock();
	printf("Time taken: %Lf s\n", (long double)(end - start) / CLOCKS_PER_SEC);
	return result;
}