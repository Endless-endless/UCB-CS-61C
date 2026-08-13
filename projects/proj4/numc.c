#include "numc.h"
#include <structmember.h>

PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
  if (rows == 1 || cols == 1) {
    return PyTuple_Pack(1, PyLong_FromLong(rows * cols));
  } else {
    return PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
  }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_XDECREF(self->shape);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError,
                        "Second operand must be a Matrix");
        return NULL;
    }

    Matrix61c *other = (Matrix61c *) args;

    if (self -> mat -> rows != other -> mat -> rows ||
        self -> mat -> cols != other -> mat -> cols) {
        PyErr_SetString(PyExc_ValueError,
                        "Matrices must have the same dimensions");
        return NULL;
    }

    matrix *result_mat;

    if (allocate_matrix(&result_mat,
                        self -> mat -> rows,
                        self -> mat -> cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Failed to allocate matrix");
        return NULL;
    }

    if (add_matrix(result_mat, self -> mat, other -> mat) != 0) {
        deallocate_matrix(result_mat);
        PyErr_SetString(PyExc_RuntimeError, "Matrix addition failed");
        return NULL;
    }

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);

    if (rv == NULL) {
        deallocate_matrix(result_mat);
        return NULL;
    }

    rv -> mat = result_mat;

    rv -> shape = get_shape(result_mat -> rows, result_mat -> cols);

    return (PyObject *) rv;
}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError,
                        "Second operand must be a Matrix");
        return NULL;
    }

    Matrix61c *other = (Matrix61c *) args;

    if (self -> mat -> rows != other -> mat -> rows ||
        self -> mat -> cols != other -> mat -> cols) {
        PyErr_SetString(PyExc_ValueError,
                        "Matrices must have the same dimensions");
        return NULL;
    }

    matrix *result_mat;

    if (allocate_matrix(&result_mat,
                        self -> mat -> rows,
                        self -> mat -> cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Failed to allocate matrix");
        return NULL;
    }

    if (sub_matrix(result_mat, self -> mat, other -> mat) != 0) {
        deallocate_matrix(result_mat);
        PyErr_SetString(PyExc_RuntimeError, "Matrix subtraction failed");
        return NULL;
    }

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);

    if (rv == NULL) {
        deallocate_matrix(result_mat);
        return NULL;
    }

    rv -> mat = result_mat;

    rv -> shape = get_shape(result_mat -> rows, result_mat -> cols);

    return (PyObject *) rv;
}

/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError,
                        "Second operand must be a Matrix");
        return NULL;
    }

    Matrix61c *other = (Matrix61c *) args;

    if (self -> mat -> cols != other -> mat -> rows) {
        PyErr_SetString(PyExc_ValueError,
                        "First matrix columns must equal second matrix rows");
        return NULL;
    }

    matrix *result_mat;

    if (allocate_matrix(&result_mat,
                        self -> mat -> rows,
                        other -> mat -> cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Failed to allocate matrix");
        return NULL;
    }

    if (mul_matrix(result_mat, self -> mat, other -> mat) != 0) {
        deallocate_matrix(result_mat);
        PyErr_SetString(PyExc_RuntimeError, "Matrix multiplication failed");
        return NULL;
    }

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);

    if (rv == NULL) {
        deallocate_matrix(result_mat);
        return NULL;
    }

    rv -> mat = result_mat;

    rv -> shape = get_shape(result_mat -> rows, result_mat -> cols);

    return (PyObject *) rv;
}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    matrix *result_mat;

    if (allocate_matrix(&result_mat,
                        self -> mat -> rows,
                        self -> mat -> cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Failed to allocate matrix");
        return NULL;
    }

    if (neg_matrix(result_mat, self -> mat) != 0) {
        deallocate_matrix(result_mat);
        PyErr_SetString(PyExc_RuntimeError, "Matrix negation failed");
        return NULL;
    }

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);

    if (rv == NULL) {
        deallocate_matrix(result_mat);
        return NULL;
    }

    rv -> mat = result_mat;

    rv -> shape = get_shape(result_mat -> rows, result_mat -> cols);

    return (PyObject *) rv;
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    matrix *result_mat;

    if (allocate_matrix(&result_mat,
                        self -> mat -> rows,
                        self -> mat -> cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Failed to allocate matrix");
        return NULL;
    }

    if (abs_matrix(result_mat, self -> mat) != 0) {
        deallocate_matrix(result_mat);
        PyErr_SetString(PyExc_RuntimeError, "Matrix absolute value failed");
        return NULL;
    }

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);

    if (rv == NULL) {
        deallocate_matrix(result_mat);
        return NULL;
    }

    rv -> mat = result_mat;

    rv -> shape = get_shape(result_mat -> rows, result_mat -> cols);

    return (PyObject *) rv;
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    if (!PyLong_Check(pow)) {
        PyErr_SetString(PyExc_TypeError, "Power must be an integer");
        return NULL;
    }

    long exponent = PyLong_AsLong(pow);


    if (exponent < 0) {
        PyErr_SetString(PyExc_ValueError, "Power must be non-negative");
        return NULL;
    }

    if (self -> mat -> rows != self -> mat -> cols) {
        PyErr_SetString(PyExc_ValueError,
                        "Matrix must be square for exponentiation");
        return NULL;
    }

    matrix *result_mat;

    if (allocate_matrix(&result_mat,
                        self -> mat -> rows,
                        self -> mat -> cols) != 0) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Failed to allocate matrix");
        return NULL;
    }

    if (pow_matrix(result_mat, self -> mat, (int) exponent) != 0) {
        deallocate_matrix(result_mat);
        PyErr_SetString(PyExc_RuntimeError, "Matrix power failed");
        return NULL;
    }

    Matrix61c *rv = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);

    if (rv == NULL) {
        deallocate_matrix(result_mat);
        return NULL;
    }

    rv -> mat = result_mat;

    rv -> shape = get_shape(result_mat -> rows, result_mat -> cols);

    return (PyObject *) rv;
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */
PyNumberMethods Matrix61c_as_number = {
    .nb_add = (binaryfunc) Matrix61c_add,
    .nb_subtract = (binaryfunc) Matrix61c_sub,
    .nb_multiply = (binaryfunc) Matrix61c_multiply,
    .nb_negative = (unaryfunc) Matrix61c_neg,
    .nb_absolute = (unaryfunc) Matrix61c_abs,
    .nb_power = (ternaryfunc) Matrix61c_pow,
};


/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
    PyObject *row_obj = NULL;
    PyObject *col_obj = NULL;
    PyObject *val_obj = NULL;

    if (!PyArg_UnpackTuple(args, "set", 3, 3,
                        &row_obj, &col_obj, &val_obj)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }

    if (!PyLong_Check(row_obj) || !PyLong_Check(col_obj)) {
        PyErr_SetString(PyExc_TypeError, "Row and column must be integers");
        return NULL;
    }

    int row = (int) PyLong_AsLong(row_obj);
    int col = (int) PyLong_AsLong(col_obj);

    if (row < 0 || row >= self->mat->rows ||
        col < 0 || col >= self->mat->cols) {

        PyErr_SetString(
            PyExc_IndexError,
            "Matrix index out of range"
        );

        return NULL;
    }

    double val;

    if (PyFloat_Check(val_obj)) {
        val = PyFloat_AsDouble(val_obj);
    } else if (PyLong_Check(val_obj)) {
        val = (double) PyLong_AsLong(val_obj);
    } else {
        PyErr_SetString(PyExc_TypeError, "Value must be int or float");
        return NULL;
    }

    set(self->mat, row, col, val);

    Py_RETURN_NONE;
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    PyObject *row_obj = NULL;
    PyObject *col_obj = NULL;

    if (!PyArg_UnpackTuple(args, "get", 2, 2,
                        &row_obj, &col_obj)) {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }

    if (!PyLong_Check(row_obj) || !PyLong_Check(col_obj)) {
        PyErr_SetString(PyExc_TypeError, "Row and column must be integers");
        return NULL;
    }

    int row = (int) PyLong_AsLong(row_obj);
    int col = (int) PyLong_AsLong(col_obj);

    if (row < 0 || row >= self->mat->rows ||
        col < 0 || col >= self->mat->cols) {

        PyErr_SetString(
            PyExc_IndexError,
            "Matrix index out of range"
        );

        return NULL;
    }

    double value = get(self->mat, row, col);

    return PyFloat_FromDouble(value);
}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = {
    {
        "get",
        (PyCFunction) Matrix61c_get_value,
        METH_VARARGS,
        "Get a matrix value"
    },
    {
        "set",
        (PyCFunction) Matrix61c_set_value,
        METH_VARARGS,
        "Set a matrix value"
    },
    {NULL, NULL, 0, NULL}
};

/* INDEXING */

/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
/*
 * Wrap a matrix view as a Python Matrix61c object.
 * If the view is 1x1, return a Python float instead.
 */
static PyObject *matrix_view_to_pyobject(matrix *view) {
    if (view == NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to create matrix view");
        return NULL;
    }

    /* Spec: a resulting 1x1 slice should return a number */
    if (view->rows == 1 && view->cols == 1) {
        double value = get(view, 0, 0);
        deallocate_matrix(view);
        return PyFloat_FromDouble(value);
    }

    Matrix61c *rv =
        (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);

    if (rv == NULL) {
        deallocate_matrix(view);
        return NULL;
    }

    rv->mat = view;
    rv->shape = get_shape(view->rows, view->cols);

    if (rv->shape == NULL) {
        Py_DECREF((PyObject *) rv);
        return NULL;
    }

    return (PyObject *) rv;
}


/*
 * Parse a Python slice for a dimension of length `length`.
 *
 * Return:
 *   0  success
 *  -1  error
 */
static int parse_slice(
    PyObject *slice,
    Py_ssize_t length,
    Py_ssize_t *start,
    Py_ssize_t *slice_length
) {
    Py_ssize_t stop;
    Py_ssize_t step;

    if (PySlice_GetIndicesEx(
            slice,
            length,
            start,
            &stop,
            &step,
            slice_length) < 0) {
        return -1;
    }

    /* Project spec only supports step == 1 and non-empty slices */
    if (step != 1 || *slice_length < 1) {
        PyErr_SetString(PyExc_ValueError, "Slice info not valid");
        return -1;
    }

    return 0;
}


/*
 * Given a numc.Matrix `self`, index into it with `key`.
 * Return the indexed result.
 */
PyObject *Matrix61c_subscript(Matrix61c *self, PyObject *key) {

    /*
     * ============================================================
     * Case 1: integer
     *
     * A[2]
     * ============================================================
     */
    if (PyLong_Check(key)) {
        int index = (int) PyLong_AsLong(key);

        /*
         * 1D matrix:
         *
         * 1 x N:
         *     A[i] -> data[0][i]
         *
         * N x 1:
         *     A[i] -> data[i][0]
         */
        if (self -> mat -> is_1d) {
            int length = self -> mat -> rows * self -> mat -> cols;

            if (index < 0 || index >= length) {
                PyErr_SetString(PyExc_IndexError, "Index out of range");
                return NULL;
            }

            double value;

            if (self -> mat -> rows == 1) {
                value = get(self -> mat, 0, index);
            } else {
                value = get(self -> mat, index, 0);
            }

            return PyFloat_FromDouble(value);
        }

        /*
         * 2D matrix:
         *
         * A[i]
         *
         * means:
         *
         * A[i, :]
         *
         * so return row i as a view.
         */
        if (index < 0 || index >= self -> mat -> rows) {
            PyErr_SetString(PyExc_IndexError, "Index out of range");
            return NULL;
        }

        matrix *new_mat = NULL;

        if (allocate_matrix_ref(
                &new_mat,
                self -> mat,
                index,
                0,
                1,
                self -> mat -> cols) != 0) {

            if (!PyErr_Occurred()) {
                PyErr_SetString(
                    PyExc_RuntimeError,
                    "Failed to create matrix slice"
                );
            }

            return NULL;
        }

        return matrix_view_to_pyobject(new_mat);
    }


    /*
     * ============================================================
     * Case 2: single slice
     *
     * A[1:3]
     * ============================================================
     */
    else if (PySlice_Check(key)) {

        Py_ssize_t start;
        Py_ssize_t slice_length;

        /*
         * 1D matrix
         */
        if (self -> mat -> is_1d) {

            Py_ssize_t length =
                self -> mat -> rows * self -> mat -> cols;

            if (parse_slice(
                    key,
                    length,
                    &start,
                    &slice_length) < 0) {
                return NULL;
            }

            matrix *new_mat = NULL;

            /*
             * 1 x N
             */
            if (self -> mat -> rows == 1) {

                if (allocate_matrix_ref(
                        &new_mat,
                        self -> mat,
                        0,
                        (int) start,
                        1,
                        (int) slice_length) != 0) {

                    if (!PyErr_Occurred()) {
                        PyErr_SetString(
                            PyExc_RuntimeError,
                            "Failed to create matrix slice"
                        );
                    }

                    return NULL;
                }
            }

            /*
             * N x 1
             */
            else {

                if (allocate_matrix_ref(
                        &new_mat,
                        self -> mat,
                        (int) start,
                        0,
                        (int) slice_length,
                        1) != 0) {

                    if (!PyErr_Occurred()) {
                        PyErr_SetString(
                            PyExc_RuntimeError,
                            "Failed to create matrix slice"
                        );
                    }

                    return NULL;
                }
            }

            return matrix_view_to_pyobject(new_mat);
        }

        /*
         * 2D matrix:
         *
         * A[a:b]
         *
         * means:
         *
         * A[a:b, :]
         */
        else {

            if (parse_slice(
                    key,
                    self -> mat -> rows,
                    &start,
                    &slice_length) < 0) {
                return NULL;
            }

            matrix *new_mat = NULL;

            if (allocate_matrix_ref(
                    &new_mat,
                    self->mat,
                    (int) start,
                    0,
                    (int) slice_length,
                    self -> mat -> cols) != 0) {

                if (!PyErr_Occurred()) {
                    PyErr_SetString(
                        PyExc_RuntimeError,
                        "Failed to create matrix slice"
                    );
                }

                return NULL;
            }

            return matrix_view_to_pyobject(new_mat);
        }
    }


    /*
     * ============================================================
     * Case 3: tuple
     *
     * A[row, col]
     * A[row, c1:c2]
     * A[r1:r2, col]
     * A[r1:r2, c1:c2]
     * ============================================================
     */
    else if (PyTuple_Check(key)) {

        /*
         * Important:
         * 1D matrices only support a single int or slice.
         */
        if (self -> mat -> is_1d) {
            PyErr_SetString(
                PyExc_TypeError,
                "1D matrices only support single index or slice"
            );
            return NULL;
        }

        if (PyTuple_Size(key) != 2) {
            PyErr_SetString(
                PyExc_TypeError,
                "2D indexing requires two indices"
            );
            return NULL;
        }

        PyObject *row_obj = PyTuple_GetItem(key, 0);
        PyObject *col_obj = PyTuple_GetItem(key, 1);


        /*
         * --------------------------------
         * A[int, int]
         * --------------------------------
         */
        if (PyLong_Check(row_obj) &&
            PyLong_Check(col_obj)) {

            int row = (int) PyLong_AsLong(row_obj);
            int col = (int) PyLong_AsLong(col_obj);

            if (row < 0 || row >= self->mat->rows ||
                col < 0 || col >= self->mat->cols) {

                PyErr_SetString(
                    PyExc_IndexError,
                    "Index out of range"
                );

                return NULL;
            }

            return PyFloat_FromDouble(
                get(self->mat, row, col)
            );
        }


        /*
         * --------------------------------
         * A[int, slice]
         * --------------------------------
         */
        else if (PyLong_Check(row_obj) &&
                 PySlice_Check(col_obj)) {

            int row = (int) PyLong_AsLong(row_obj);

            if (row < 0 || row >= self->mat->rows) {
                PyErr_SetString(
                    PyExc_IndexError,
                    "Row index out of range"
                );
                return NULL;
            }

            Py_ssize_t col_start;
            Py_ssize_t col_length;

            if (parse_slice(
                    col_obj,
                    self -> mat -> cols,
                    &col_start,
                    &col_length) < 0) {
                return NULL;
            }

            matrix *new_mat = NULL;

            if (allocate_matrix_ref(
                    &new_mat,
                    self->mat,
                    row,
                    (int) col_start,
                    1,
                    (int) col_length) != 0) {

                if (!PyErr_Occurred()) {
                    PyErr_SetString(
                        PyExc_RuntimeError,
                        "Failed to create matrix slice"
                    );
                }

                return NULL;
            }

            return matrix_view_to_pyobject(new_mat);
        }


        /*
         * --------------------------------
         * A[slice, int]
         * --------------------------------
         */
        else if (PySlice_Check(row_obj) &&
                 PyLong_Check(col_obj)) {

            int col = (int) PyLong_AsLong(col_obj);

            if (col < 0 || col >= self->mat->cols) {
                PyErr_SetString(
                    PyExc_IndexError,
                    "Column index out of range"
                );
                return NULL;
            }

            Py_ssize_t row_start;
            Py_ssize_t row_length;

            if (parse_slice(
                    row_obj,
                    self -> mat -> rows,
                    &row_start,
                    &row_length) < 0) {
                return NULL;
            }

            matrix *new_mat = NULL;

            if (allocate_matrix_ref(
                    &new_mat,
                    self -> mat,
                    (int) row_start,
                    col,
                    (int) row_length,
                    1) != 0) {

                if (!PyErr_Occurred()) {
                    PyErr_SetString(
                        PyExc_RuntimeError,
                        "Failed to create matrix slice"
                    );
                }

                return NULL;
            }

            return matrix_view_to_pyobject(new_mat);
        }


        /*
         * --------------------------------
         * A[slice, slice]
         * --------------------------------
         */
        else if (PySlice_Check(row_obj) &&
                 PySlice_Check(col_obj)) {

            Py_ssize_t row_start;
            Py_ssize_t row_length;

            Py_ssize_t col_start;
            Py_ssize_t col_length;

            if (parse_slice(
                    row_obj,
                    self -> mat -> rows,
                    &row_start,
                    &row_length) < 0) {
                return NULL;
            }

            if (parse_slice(
                    col_obj,
                    self -> mat -> cols,
                    &col_start,
                    &col_length) < 0) {
                return NULL;
            }

            matrix *new_mat = NULL;

            if (allocate_matrix_ref(
                    &new_mat,
                    self->mat,
                    (int) row_start,
                    (int) col_start,
                    (int) row_length,
                    (int) col_length) != 0) {

                if (!PyErr_Occurred()) {
                    PyErr_SetString(
                        PyExc_RuntimeError,
                        "Failed to create matrix slice"
                    );
                }

                return NULL;
            }

            return matrix_view_to_pyobject(new_mat);
        }


        /*
         * tuple contains unsupported things
         */
        else {
            PyErr_SetString(
                PyExc_TypeError,
                "Tuple indices must be integers or slices"
            );
            return NULL;
        }
    }


    /*
     * ============================================================
     * Invalid key
     * ============================================================
     */
    else {
        PyErr_SetString(
            PyExc_TypeError,
            "Invalid index type"
        );
        return NULL;
    }
}



/*
 * Convert a Python int/float to C double.
 * Return 0 on success, -1 on failure.
 */
static int py_number_to_double(PyObject *obj, double *out) {
    if (PyFloat_Check(obj)) {
        *out = PyFloat_AsDouble(obj);
        return 0;
    }

    if (PyLong_Check(obj)) {
        *out = (double) PyLong_AsLong(obj);

        if (PyErr_Occurred()) {
            return -1;
        }

        return 0;
    }

    return -1;
}


/*
 * Given a numc.Matrix `self`, index into it with `key`,
 * and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(
    Matrix61c *self,
    PyObject *key,
    PyObject *v
) {
    int row_offset = 0;
    int col_offset = 0;
    int rows = 0;
    int cols = 0;

    /*
     * ============================================================
     * Case 1:
     *
     * A[index] = ...
     * ============================================================
     */
    if (PyLong_Check(key)) {
        int index = (int) PyLong_AsLong(key);

        if (PyErr_Occurred()) {
            return -1;
        }

        /*
         * 1D:
         *
         * A[index] = scalar
         */
        if (self -> mat -> is_1d) {
            int length =
                self -> mat -> rows * self -> mat -> cols;

            if (index < 0 || index >= length) {
                PyErr_SetString(
                    PyExc_IndexError,
                    "Index out of range"
                );
                return -1;
            }

            double value;

            if (py_number_to_double(v, &value) != 0) {
                PyErr_SetString(
                    PyExc_TypeError,
                    "Value must be an int or float"
                );
                return -1;
            }

            if (self -> mat -> rows == 1) {
                set(self->mat, 0, index, value);
            } else {
                set(self->mat, index, 0, value);
            }

            return 0;
        }

        /*
         * 2D:
         *
         * A[index] means entire row:
         *
         * A[index, :]
         *
         * result shape = 1 x cols
         */
        if (index < 0 || index >= self->mat->rows) {
            PyErr_SetString(
                PyExc_IndexError,
                "Index out of range"
            );
            return -1;
        }

        row_offset = index;
        col_offset = 0;
        rows = 1;
        cols = self->mat->cols;
    }


    /*
     * ============================================================
     * Case 2:
     *
     * A[start:stop] = ...
     * ============================================================
     */
    else if (PySlice_Check(key)) {
        Py_ssize_t start;
        Py_ssize_t slice_length;

        /*
         * 1D slice
         */
        if (self -> mat -> is_1d) {
            int length =
                self->mat->rows * self->mat->cols;

            if (parse_slice(
                    key,
                    length,
                    &start,
                    &slice_length) != 0) {
                return -1;
            }

            /*
             * 1 x N
             */
            if (self->mat->rows == 1) {
                row_offset = 0;
                col_offset = (int) start;
                rows = 1;
                cols = (int) slice_length;
            }

            /*
             * N x 1
             */
            else {
                row_offset = (int) start;
                col_offset = 0;
                rows = (int) slice_length;
                cols = 1;
            }
        }

        /*
         * 2D:
         *
         * A[a:b] means A[a:b, :]
         */
        else {
            if (parse_slice(
                    key,
                    self -> mat -> rows,
                    &start,
                    &slice_length) != 0) {
                return -1;
            }

            row_offset = (int) start;
            col_offset = 0;
            rows = (int) slice_length;
            cols = self->mat->cols;
        }
    }


    /*
     * ============================================================
     * Case 3:
     *
     * A[row, col]
     * A[row, slice]
     * A[slice, col]
     * A[slice, slice]
     * ============================================================
     */
    else if (PyTuple_Check(key)) {

        /*
         * 1D matrices do NOT support tuple indexing.
         */
        if (self -> mat -> is_1d) {
            PyErr_SetString(
                PyExc_TypeError,
                "1D matrices only support a single index or slice"
            );
            return -1;
        }

        if (PyTuple_Size(key) != 2) {
            PyErr_SetString(
                PyExc_TypeError,
                "2D indexing requires two indices"
            );
            return -1;
        }

        PyObject *row_obj =
            PyTuple_GetItem(key, 0);

        PyObject *col_obj =
            PyTuple_GetItem(key, 1);


        /*
         * --------------------------------------------------------
         * A[int, int]
         * --------------------------------------------------------
         */
        if (PyLong_Check(row_obj) &&
            PyLong_Check(col_obj)) {

            int row =
                (int) PyLong_AsLong(row_obj);

            int col =
                (int) PyLong_AsLong(col_obj);

            if (PyErr_Occurred()) {
                return -1;
            }

            if (row < 0 ||
                row >= self->mat->rows ||
                col < 0 ||
                col >= self->mat->cols) {

                PyErr_SetString(
                    PyExc_IndexError,
                    "Index out of range"
                );
                return -1;
            }

            double value;

            if (py_number_to_double(v, &value) != 0) {
                PyErr_SetString(
                    PyExc_TypeError,
                    "Value must be an int or float"
                );
                return -1;
            }

            set(
                self->mat,
                row,
                col,
                value
            );

            return 0;
        }


        /*
         * --------------------------------------------------------
         * A[int, slice]
         * --------------------------------------------------------
         */
        else if (
            PyLong_Check(row_obj) &&
            PySlice_Check(col_obj)
        ) {
            int row =
                (int) PyLong_AsLong(row_obj);

            if (PyErr_Occurred()) {
                return -1;
            }

            if (row < 0 ||
                row >= self->mat->rows) {

                PyErr_SetString(
                    PyExc_IndexError,
                    "Row index out of range"
                );
                return -1;
            }

            Py_ssize_t start;
            Py_ssize_t length;

            if (parse_slice(
                    col_obj,
                    self -> mat -> cols,
                    &start,
                    &length) != 0) {
                return -1;
            }

            row_offset = row;
            col_offset = (int) start;
            rows = 1;
            cols = (int) length;
        }


        /*
         * --------------------------------------------------------
         * A[slice, int]
         * --------------------------------------------------------
         */
        else if (
            PySlice_Check(row_obj) &&
            PyLong_Check(col_obj)
        ) {
            int col =
                (int) PyLong_AsLong(col_obj);

            if (PyErr_Occurred()) {
                return -1;
            }

            if (col < 0 ||
                col >= self -> mat -> cols) {

                PyErr_SetString(
                    PyExc_IndexError,
                    "Column index out of range"
                );
                return -1;
            }

            Py_ssize_t start;
            Py_ssize_t length;

            if (parse_slice(
                    row_obj,
                    self -> mat -> rows,
                    &start,
                    &length) != 0) {
                return -1;
            }

            row_offset = (int) start;
            col_offset = col;
            rows = (int) length;
            cols = 1;
        }


        /*
         * --------------------------------------------------------
         * A[slice, slice]
         * --------------------------------------------------------
         */
        else if (
            PySlice_Check(row_obj) &&
            PySlice_Check(col_obj)
        ) {
            Py_ssize_t row_start;
            Py_ssize_t row_length;

            Py_ssize_t col_start;
            Py_ssize_t col_length;

            if (parse_slice(
                    row_obj,
                    self -> mat -> rows,
                    &row_start,
                    &row_length) != 0) {
                return -1;
            }

            if (parse_slice(
                    col_obj,
                    self -> mat -> cols,
                    &col_start,
                    &col_length) != 0) {
                return -1;
            }

            row_offset = (int) row_start;
            col_offset = (int) col_start;

            rows = (int) row_length;
            cols = (int) col_length;
        }


        else {
            PyErr_SetString(
                PyExc_TypeError,
                "Indices must be integers or slices"
            );
            return -1;
        }
    }


    /*
     * Invalid key type
     */
    else {
        PyErr_SetString(
            PyExc_TypeError,
            "Invalid index type"
        );
        return -1;
    }


    /*
     * ============================================================
     *
     * At this point:
     *
     * row_offset
     * col_offset
     * rows
     * cols
     *
     * describe the region being assigned.
     *
     * ============================================================
     */


    /*
     * ------------------------------------------------------------
     * Resulting region is 1 x 1
     *
     * Example:
     *
     * A[0:1, 0:1] = 3
     * ------------------------------------------------------------
     */
    if (rows == 1 && cols == 1) {
        double value;

        if (py_number_to_double(v, &value) != 0) {
            PyErr_SetString(
                PyExc_TypeError,
                "Value must be an int or float"
            );
            return -1;
        }

        set(
            self -> mat,
            row_offset,
            col_offset,
            value
        );

        return 0;
    }


    /*
     * ------------------------------------------------------------
     * Resulting region is 1D
     *
     * Either:
     *
     * 1 x N
     *
     * or
     *
     * N x 1
     *
     * v must be a Python list.
     * ------------------------------------------------------------
     */
    if (rows == 1 || cols == 1) {

        if (!PyList_Check(v)) {
            PyErr_SetString(
                PyExc_TypeError,
                "Value must be a list for a 1D slice"
            );
            return -1;
        }

        int expected_length =
            rows * cols;

        if (PyList_Size(v) != expected_length) {
            PyErr_SetString(
                PyExc_ValueError,
                "List has incorrect length"
            );
            return -1;
        }

        for (int i = 0;
             i < expected_length;
             i++) {

            PyObject *item =
                PyList_GetItem(v, i);

            double value;

            if (py_number_to_double(
                    item,
                    &value) != 0) {

                PyErr_SetString(
                    PyExc_ValueError,
                    "All list elements must be int or float"
                );

                return -1;
            }

            if (rows == 1) {
                set(
                    self -> mat,
                    row_offset,
                    col_offset + i,
                    value
                );
            } else {
                set(
                    self -> mat,
                    row_offset + i,
                    col_offset,
                    value
                );
            }
        }

        return 0;
    }


    /*
     * ------------------------------------------------------------
     * Resulting region is 2D
     *
     * v must look like:
     *
     * [
     *   [1, 2],
     *   [3, 4]
     * ]
     * ------------------------------------------------------------
     */
    if (!PyList_Check(v)) {
        PyErr_SetString(
            PyExc_TypeError,
            "Value must be a 2D list for a 2D slice"
        );
        return -1;
    }

    /*
     * Number of rows must match.
     */
    if (PyList_Size(v) != rows) {
        PyErr_SetString(
            PyExc_ValueError,
            "Incorrect number of rows"
        );
        return -1;
    }


    /*
     * First validate the ENTIRE input.
     *
     * Important:
     * Do not start modifying the matrix before discovering
     * that row 2 contains invalid data.
     */
    for (int i = 0; i < rows; i++) {

        PyObject *row_list =
            PyList_GetItem(v, i);

        if (!PyList_Check(row_list)) {
            PyErr_SetString(
                PyExc_ValueError,
                "Each row must be a list"
            );
            return -1;
        }

        if (PyList_Size(row_list) != cols) {
            PyErr_SetString(
                PyExc_ValueError,
                "Incorrect row length"
            );
            return -1;
        }

        for (int j = 0; j < cols; j++) {

            PyObject *item =
                PyList_GetItem(
                    row_list,
                    j
                );

            double dummy;

            if (py_number_to_double(
                    item,
                    &dummy) != 0) {

                PyErr_SetString(
                    PyExc_ValueError,
                    "Matrix entries must be int or float"
                );

                return -1;
            }
        }
    }


    /*
     * Validation succeeded.
     * Now actually modify the matrix.
     */
    for (int i = 0; i < rows; i++) {

        PyObject *row_list =
            PyList_GetItem(v, i);

        for (int j = 0; j < cols; j++) {

            PyObject *item =
                PyList_GetItem(
                    row_list,
                    j
                );

            double value = 0.0;

            py_number_to_double(
                item,
                &value
            );

            set(
                self -> mat,
                row_offset + i,
                col_offset + j,
                value
            );
        }
    }

    return 0;
}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor)Matrix61c_dealloc,
    .tp_repr = (reprfunc)Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc)Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("CS61C Fall 2020 Project 4: numc imported!\n");
    fflush(stdout);
    return m;
}