"""
Local testing utilities for numc.

This version replaces Berkeley's unavailable `dumbpy`
reference implementation with NumPy.
"""

import numc as nc
import numpy as np
import hashlib
import struct
import operator
import time

from typing import Union, List


# ============================================================
# Global configuration
# ============================================================

num_samples = 1000
decimal_places = 6


func_mapping = {
    "add": operator.add,
    "sub": operator.sub,
    "mul": operator.mul,
    "neg": operator.neg,
    "abs": operator.abs,
    "pow": operator.pow,
}


# ============================================================
# NumPy reference matrix creation
# ============================================================

def _numpy_matrix(*args, **kwargs):
    """
    Create a NumPy matrix that behaves similarly to nc.Matrix
    for the cases used in this project.
    """

    # Case:
    # Matrix([[1, 2], [3, 4]])
    if len(args) == 1 and isinstance(args[0], list):
        arr = np.array(args[0], dtype=float)

        if arr.ndim == 1:
            return arr

        return arr

    # Case:
    # Matrix(rows, cols)
    # Matrix(rows, cols, val)
    # Matrix(rows, cols, list)
    if len(args) >= 2:
        rows = args[0]
        cols = args[1]

        # Random initialization
        if kwargs.get("rand", False):
            seed = kwargs.get("seed", 0)

            # Important:
            # numc's rand_matrix uses C rand(), so NumPy random values
            # will NOT match exactly.
            #
            # Therefore random generation shared between NumPy and numc
            # is handled separately in rand_dp_nc_matrix().
            raise RuntimeError(
                "Use rand_dp_nc_matrix() for random matrices."
            )

        # Matrix(rows, cols)
        if len(args) == 2:
            return np.zeros((rows, cols), dtype=float)

        third = args[2]

        # Matrix(rows, cols, scalar)
        if isinstance(third, (int, float)):
            return np.full((rows, cols), float(third), dtype=float)

        # Matrix(rows, cols, flat list)
        if isinstance(third, list):
            arr = np.array(third, dtype=float)
            return arr.reshape(rows, cols)

    raise ValueError("Unsupported matrix constructor arguments")


def dp_nc_matrix(*args, **kwargs):
    """
    Local replacement for the original dp_nc_matrix().

    Returns:
        numpy_matrix, numc_matrix
    """

    np_mat = _numpy_matrix(*args, **kwargs)
    nc_mat = nc.Matrix(*args, **kwargs)

    return np_mat, nc_mat


# ============================================================
# Random matrix creation
# ============================================================

def rand_dp_nc_matrix(rows, cols, seed=0):
    """
    Create matching NumPy and numc matrices.

    We generate values ourselves, then initialize both implementations
    from exactly the same Python list.

    This avoids differences between:
        C rand()
        NumPy random generator
    """

    rng = np.random.default_rng(seed)

    values = rng.uniform(
        low=0.0,
        high=1.0,
        size=(rows, cols)
    )

    values_list = values.tolist()

    np_mat = np.array(values_list, dtype=float)
    nc_mat = nc.Matrix(values_list)

    return np_mat, nc_mat


# ============================================================
# Conversion helpers
# ============================================================

def nc_to_numpy(mat):
    """
    Convert a numc.Matrix into a NumPy array using indexing.
    """

    shape = mat.shape

    # 1D matrix
    if len(shape) == 1:
        result = np.zeros(shape[0], dtype=float)

        for i in range(shape[0]):
            result[i] = mat[i]

        return result

    # 2D matrix
    rows, cols = shape

    result = np.zeros((rows, cols), dtype=float)

    for i in range(rows):
        for j in range(cols):
            result[i, j] = mat[i, j]

    return result


# ============================================================
# Correctness checking
# ============================================================

def cmp_dp_nc_matrix(dp_mat, nc_mat):
    """
    Compare NumPy reference result against numc result.

    Uses floating-point tolerance instead of exact equality.
    """

    if isinstance(dp_mat, nc.Matrix):
        dp_mat = nc_to_numpy(dp_mat)

    if isinstance(nc_mat, nc.Matrix):
        nc_mat = nc_to_numpy(nc_mat)

    dp_arr = np.asarray(dp_mat, dtype=float)
    nc_arr = np.asarray(nc_mat, dtype=float)

    if dp_arr.shape != nc_arr.shape:
        return False

    return np.allclose(
        dp_arr,
        nc_arr,
        rtol=1e-6,
        atol=1e-6
    )


# ============================================================
# Compute / benchmark
# ============================================================

def compute(
    dp_mat_lst: List,
    nc_mat_lst: List,
    op: str
):
    """
    Run the same operation using NumPy and numc.

    Returns:
        is_correct, speed_up

    speed_up =
        numpy_time / numc_time

    So:
        > 1  => numc faster
        < 1  => NumPy faster
    """

    assert op in func_mapping

    # --------------------------------------------------------
    # numc timing
    # --------------------------------------------------------

    nc_start = time.perf_counter()

    if op == "neg":
        assert len(nc_mat_lst) == 1
        nc_result = -nc_mat_lst[0]

    elif op == "abs":
        assert len(nc_mat_lst) == 1
        nc_result = abs(nc_mat_lst[0])

    elif op == "pow":
        assert len(nc_mat_lst) == 2
        nc_result = nc_mat_lst[0] ** nc_mat_lst[1]

    else:
        assert len(nc_mat_lst) >= 2

        nc_result = nc_mat_lst[0]

        for mat in nc_mat_lst[1:]:
            nc_result = func_mapping[op](nc_result, mat)

    nc_end = time.perf_counter()

    # --------------------------------------------------------
    # NumPy timing
    # --------------------------------------------------------

    dp_start = time.perf_counter()

    if op == "neg":
        assert len(dp_mat_lst) == 1
        dp_result = -dp_mat_lst[0]

    elif op == "abs":
        assert len(dp_mat_lst) == 1
        dp_result = np.abs(dp_mat_lst[0])

    elif op == "pow":
        assert len(dp_mat_lst) == 2

        base = dp_mat_lst[0]
        power = dp_mat_lst[1]

        # Matrix power, NOT element-wise power.
        dp_result = np.linalg.matrix_power(base, power)

    elif op == "mul":
        assert len(dp_mat_lst) >= 2

        dp_result = dp_mat_lst[0]

        for mat in dp_mat_lst[1:]:
            # NumPy * is element-wise,
            # but numc * is matrix multiplication.
            dp_result = dp_result @ mat

    else:
        assert len(dp_mat_lst) >= 2

        dp_result = dp_mat_lst[0]

        for mat in dp_mat_lst[1:]:
            dp_result = func_mapping[op](dp_result, mat)

    dp_end = time.perf_counter()

    # --------------------------------------------------------
    # Correctness
    # --------------------------------------------------------

    is_correct = cmp_dp_nc_matrix(dp_result, nc_result)

    nc_time = nc_end - nc_start
    dp_time = dp_end - dp_start

    if nc_time == 0:
        speed_up = float("inf")
    else:
        speed_up = dp_time / nc_time

    return is_correct, speed_up


# ============================================================
# Printing
# ============================================================

def print_speedup(speed_up):
    print(f"Speed up vs NumPy: {speed_up:.4f}x")


# ============================================================
# Hash helper
# Retained for compatibility with original staff utils.py
# ============================================================

def rand_md5(mat):
    """
    Generate an MD5 hash by sampling matrix values.

    Mostly retained so existing staff-style code does not break.
    """

    if isinstance(mat, nc.Matrix):
        mat = nc_to_numpy(mat)

    arr = np.asarray(mat, dtype=float)

    np.random.seed(1)

    m = hashlib.md5()

    if arr.ndim > 1:
        rows, cols = arr.shape
        total_cnt = rows * cols

        if total_cnt < num_samples:
            for i in range(rows):
                for j in range(cols):
                    value = round(float(arr[i, j]), decimal_places)
                    m.update(struct.pack("f", value))

        else:
            for _ in range(num_samples):
                i = np.random.randint(rows)
                j = np.random.randint(cols)

                value = round(float(arr[i, j]), decimal_places)
                m.update(struct.pack("f", value))

    else:
        total_cnt = arr.shape[0]

        if total_cnt < num_samples:
            for i in range(total_cnt):
                value = round(float(arr[i]), decimal_places)
                m.update(struct.pack("f", value))

        else:
            for _ in range(num_samples):
                i = np.random.randint(total_cnt)

                value = round(float(arr[i]), decimal_places)
                m.update(struct.pack("f", value))

    return m.digest()