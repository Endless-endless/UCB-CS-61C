#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
*/

/*
 * Generates a random double between `low` and `high`.
 */
double rand_double(double low, double high) {
    double range = (high - low);
    double div = RAND_MAX / range;
    return low + (rand() / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void rand_matrix(matrix *result, unsigned int seed, double low, double high) {
    srand(seed);
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            set(result, i, j, rand_double(low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data array
 * and initialize all entries to be zeros. Remember to set all fieds of the matrix struct.
 * `parent` should be set to NULL to indicate that this matrix is not a slice.
 * You should return -1 if either `rows` or `cols` or both have invalid values, or if any
 * call to allocate memory in this function fails. If you don't set python error messages here upon
 * failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix(matrix **mat, int rows, int cols) {
    /* TODO: YOUR CODE HERE */
    if (rows <= 0 || cols <= 0)
    {
        return -1;
    }

    *mat = malloc(sizeof(matrix));
    if (*mat == NULL)
    {
        return -1;
    }

    (*mat) -> rows = rows;
    (*mat) -> cols = cols;
    (*mat) -> is_1d = (rows == 1 || cols == 1);
    (*mat) -> ref_cnt = 1;
    (*mat) -> parent = NULL;

    (*mat) -> data = malloc(rows*sizeof(double*));
    if ((*mat) -> data == NULL)
    {
        free(*mat);
        *mat = NULL;
        return -1;
    }

    for (int i = 0; i < rows; i++)
    {
        (*mat) -> data[i] = calloc(cols,sizeof(double));
        if ((*mat) -> data[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free((*mat) -> data[j]);
            }

            free((*mat) -> data);
            free(*mat);
            *mat = NULL;

            return -1;
        }
    }

    return 0;
}

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and `cols` columns.
 * This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix_ref(matrix **mat, matrix *from, int row_offset, int col_offset,
                        int rows, int cols) {
    if (from == NULL || rows <= 0 || cols <= 0)
    {
        return -1;
    }

    if (row_offset < 0 || col_offset < 0)
    {
        return -1;
    }

    if (row_offset + rows > from -> rows || col_offset + cols > from->cols)
    {
        return -1;
    }

    *mat = malloc(sizeof(matrix));
    if (*mat == NULL)
    {
        return -1;
    }

    (*mat) -> rows = rows;
    (*mat) -> cols = cols;
    (*mat) -> is_1d = (rows == 1 || cols == 1);
    (*mat) -> ref_cnt = 1;
    (*mat) -> parent = from;

    (*mat) -> data = malloc(rows*sizeof(double*));
    if ((*mat) -> data == NULL)
    {
        free(*mat);
        *mat = NULL;
        return -1;
    }

    for (int i = 0; i < rows; i++)
    {
        (*mat) -> data[i] = from -> data[row_offset + i] + col_offset;
    }

    from -> ref_cnt++;

    return 0;
}

/*
 * This function will be called automatically by Python when a numc matrix loses all of its
 * reference pointers.
 * You need to make sure that you only free `mat->data` if no other existing matrices are also
 * referring this data array.
 * See the spec for more information.
 */
void deallocate_matrix(matrix *mat) {
    if (mat == NULL)
    {
        return;
    }

    mat -> ref_cnt--;

    while (mat != NULL && mat -> ref_cnt == 0)
    {
        matrix* parent = mat -> parent;

        if (parent == NULL)
        {
            for (int i = 0; i < mat -> rows; i++)
            {
                free(mat -> data[i]);
            }
        }

        free(mat -> data);
        free(mat);

        if (parent != NULL)
        {
            parent -> ref_cnt--;
        }

        mat = parent;
    }
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
double get(matrix *mat, int row, int col) {
    return mat -> data[row][col];
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
void set(matrix *mat, int row, int col, double val) {
    mat -> data[row][col] = val;
}

/*
 * Set all entries in mat to val
 */
void fill_matrix(matrix *mat, double val) {
    int row = mat -> rows;
    int col = mat -> cols;

    #pragma omp parallel for
    for (int i = 0; i < row; i++)
    {
        __m256d values = _mm256_set1_pd(val);
        int j = 0;
        for (; j + 3 < col; j += 4)
        {
            _mm256_storeu_pd(mat -> data[i] + j, values);
        }

        for (; j < col; j++)
        {
            mat -> data[i][j] = val;
        }
    }
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int add_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1 == NULL || mat2 == NULL) return -1;
    if (mat1 -> rows != mat2 -> rows || mat1 -> cols != mat2 -> cols)
    {
        return -1;
    }

    #pragma omp parallel for
    for (int i = 0; i < result -> rows; i++)
    {
        int j = 0;
        for (; j + 3 < result -> cols; j += 4)
        {
            __m256d a = _mm256_loadu_pd(mat1 -> data[i] + j);
            __m256d b = _mm256_loadu_pd(mat2 -> data[i] + j);

            __m256d c = _mm256_add_pd(a,b);

            _mm256_storeu_pd(result -> data[i] + j,c);
        }

        for (; j < result -> cols; j++)
        {
            result -> data[i][j] = mat1 -> data[i][j] + mat2 -> data[i][j];
        }
    }

    return 0;
}

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int sub_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1 == NULL || mat2 == NULL) return -1;
    if (mat1 -> rows != mat2 -> rows || mat1 -> cols != mat2 -> cols)
    {
        return -1;
    }

    #pragma omp parallel for
    for (int i = 0; i < result -> rows; i++)
    {
        int j = 0;
        for (; j + 3 < result -> cols; j += 4)
        {
            __m256d a = _mm256_loadu_pd(mat1 -> data[i] + j);
            __m256d b = _mm256_loadu_pd(mat2 -> data[i] + j);

            __m256d c = _mm256_sub_pd(a,b);

            _mm256_storeu_pd(result -> data[i] + j,c);
        }

        for (; j < result -> cols; j++)
        {
            result -> data[i][j] = mat1 -> data[i][j] - mat2 -> data[i][j];
        }
    }

    return 0;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying individual elements.
 */
int mul_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1 == NULL || mat2 == NULL) return -1;
    if (mat1 -> cols != mat2 -> rows)
    {
        return -1;
    }

    #pragma omp parallel for
    for (int i = 0; i < mat1->rows; i++) {

        double *result_row = result->data[i];
        double *mat1_row = mat1->data[i];

        for (int k = 0; k < mat1->cols; k++) {

            double a = mat1_row[k];
            double *mat2_row = mat2->data[k];

            __m256d a_vec = _mm256_set1_pd(a);

            int j = 0;

            for (; j + 3 < mat2->cols; j += 4) {

                __m256d b =
                    _mm256_loadu_pd(mat2_row + j);

                __m256d c =
                    _mm256_loadu_pd(result_row + j);

                c = _mm256_fmadd_pd(a_vec, b, c);

                _mm256_storeu_pd(result_row + j, c);
            }

            for (; j < mat2->cols; j++) {
                result_row[j] += a * mat2_row[j];
            }
        }
    }
    return 0;
}

/*
 * Store the result of raising mat to the (pow)th power to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that pow is defined with matrix multiplication, not element-wise multiplication.
 */
int pow_matrix(matrix *result, matrix *mat, int pow) {
    if (result == NULL || mat == NULL) {
        return -1;
    }

    if (mat -> rows != mat -> cols || pow < 0) {
        return -1;
    }

    int n = mat -> rows;

    matrix *base = NULL;
    matrix *temp = NULL;

    if (allocate_matrix(&temp, n, n) != 0) {
        return -1;
    }

    if (allocate_matrix(&base, n, n) != 0) {
        deallocate_matrix(temp);
        return -1;
    }

    // result = identity matrix
    fill_matrix(result, 0);

    for (int i = 0; i < n; i++) {
        result -> data[i][i] = 1.0;
    }

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            base -> data[i][j] = mat -> data[i][j];
        }
    }

    while (pow > 0)
    {
        if (pow % 2 == 1)
        {
            fill_matrix(temp, 0.0);

            if (mul_matrix(temp, result, base) != 0) {
                deallocate_matrix(base);
                deallocate_matrix(temp);
                return -1;
            }

            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    result -> data[i][j] = temp -> data[i][j];
                }
            }
        }

        pow /= 2;

        if (pow > 0)
        {
            fill_matrix(temp, 0.0);

            if (mul_matrix(temp, base, base) != 0) {
                deallocate_matrix(base);
                deallocate_matrix(temp);
                return -1;
            }

            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    base -> data[i][j] = temp -> data[i][j];
                }
            }
        }
    }

    deallocate_matrix(base);
    deallocate_matrix(temp);

    return 0;
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int neg_matrix(matrix *result, matrix *mat) {
    if (mat == NULL) return -1;

    #pragma omp parallel for
    for (int i = 0; i < mat -> rows; i++)
    {
        int j = 0;
        __m256d zero = _mm256_setzero_pd();
        for (; j + 3 < mat -> cols; j += 4)
        {
            __m256d x = _mm256_loadu_pd(mat -> data[i] + j);
            __m256d r = _mm256_sub_pd(zero, x);
            _mm256_storeu_pd(result -> data[i] + j,r);
        }

        for (; j < mat -> cols; j++)
        {
            result -> data[i][j] = - mat -> data[i][j];
        }
    }

    return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int abs_matrix(matrix *result, matrix *mat) {
    if (mat == NULL) return -1;
    for (int i = 0; i < mat -> rows; i++)
    {
        for (int j = 0; j < mat -> cols; j++)
        {
            if (mat -> data[i][j] >= 0)
            {
                result -> data[i][j] = mat -> data[i][j];
            }
            else
            {
                result -> data[i][j] = - mat -> data[i][j];
            }
        }
    }

    return 0;
}

