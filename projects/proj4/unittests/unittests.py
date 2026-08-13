from utils import *
from unittest import TestCase


"""
For each operation, test matrices of different sizes.

We use rand_dp_nc_matrix() to generate an identical dumbpy matrix
and numc matrix, then use compute() to compare correctness and
measure speedup.
"""


class TestAdd(TestCase):
    def test_small_add(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(2, 2, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(2, 2, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "add"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_medium_add(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(100, 100, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(100, 100, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "add"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_large_add(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(1000, 1000, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(1000, 1000, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "add"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)


class TestSub(TestCase):
    def test_small_sub(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(2, 2, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(2, 2, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "sub"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_medium_sub(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(100, 100, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(100, 100, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "sub"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_large_sub(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(1000, 1000, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(1000, 1000, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "sub"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)


class TestAbs(TestCase):
    def test_small_abs(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(2, 2, seed=0)

        is_correct, speed_up = compute(
            [dp_mat],
            [nc_mat],
            "abs"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_medium_abs(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(100, 100, seed=0)

        is_correct, speed_up = compute(
            [dp_mat],
            [nc_mat],
            "abs"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_large_abs(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(1000, 1000, seed=0)

        is_correct, speed_up = compute(
            [dp_mat],
            [nc_mat],
            "abs"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)


class TestNeg(TestCase):
    def test_small_neg(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(2, 2, seed=0)

        is_correct, speed_up = compute(
            [dp_mat],
            [nc_mat],
            "neg"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_medium_neg(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(100, 100, seed=0)

        is_correct, speed_up = compute(
            [dp_mat],
            [nc_mat],
            "neg"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_large_neg(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(1000, 1000, seed=0)

        is_correct, speed_up = compute(
            [dp_mat],
            [nc_mat],
            "neg"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)


class TestMul(TestCase):
    def test_small_mul(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(2, 2, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(2, 2, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "mul"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_medium_mul(self):
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(100, 100, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(100, 100, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "mul"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_large_mul(self):
        # Matrix multiplication is O(n^3), so don't make this too huge
        # or dumbpy may take a very long time.
        dp_mat1, nc_mat1 = rand_dp_nc_matrix(300, 300, seed=0)
        dp_mat2, nc_mat2 = rand_dp_nc_matrix(300, 300, seed=1)

        is_correct, speed_up = compute(
            [dp_mat1, dp_mat2],
            [nc_mat1, nc_mat2],
            "mul"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)


class TestPow(TestCase):
    def test_small_pow(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(2, 2, seed=0)

        is_correct, speed_up = compute(
            [dp_mat, 3],
            [nc_mat, 3],
            "pow"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_medium_pow(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(50, 50, seed=0)

        is_correct, speed_up = compute(
            [dp_mat, 5],
            [nc_mat, 5],
            "pow"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)

    def test_large_pow(self):
        # pow repeatedly performs matrix multiplication,
        # so use a moderate matrix size.
        dp_mat, nc_mat = rand_dp_nc_matrix(100, 100, seed=0)

        is_correct, speed_up = compute(
            [dp_mat, 5],
            [nc_mat, 5],
            "pow"
        )

        self.assertTrue(is_correct)
        print_speedup(speed_up)


class TestGet(TestCase):
    def test_get(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(10, 10, seed=0)

        rand_row = np.random.randint(dp_mat.shape[0])
        rand_col = np.random.randint(dp_mat.shape[1])

        self.assertEqual(
            round(dp_mat[rand_row][rand_col], decimal_places),
            round(nc_mat[rand_row][rand_col], decimal_places)
        )


class TestSet(TestCase):
    def test_set(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(10, 10, seed=0)

        rand_row = np.random.randint(dp_mat.shape[0])
        rand_col = np.random.randint(dp_mat.shape[1])

        value = 123.456

        dp_mat[rand_row][rand_col] = value
        nc_mat.set(rand_row, rand_col, value)

        self.assertEqual(
            round(dp_mat[rand_row][rand_col], decimal_places),
            round(nc_mat[rand_row][rand_col], decimal_places)
        )


class TestShape(TestCase):
    def test_shape(self):
        dp_mat, nc_mat = rand_dp_nc_matrix(10, 20, seed=0)

        self.assertTrue(
            dp_mat.shape == nc_mat.shape
        )