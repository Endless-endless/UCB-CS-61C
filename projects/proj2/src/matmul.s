.globl matmul

.text
# =======================================================
# FUNCTION: Matrix Multiplication of 2 integer matrices
# 	d = matmul(m0, m1)
# Arguments:
# 	a0 (int*)  is the pointer to the start of m0 
#	a1 (int)   is the # of rows (height) of m0
#	a2 (int)   is the # of columns (width) of m0
#	a3 (int*)  is the pointer to the start of m1
# 	a4 (int)   is the # of rows (height) of m1
#	a5 (int)   is the # of columns (width) of m1
#	a6 (int*)  is the pointer to the the start of d
# Returns:
#	None (void), sets d = matmul(m0, m1)
# Exceptions:
#   Make sure to check in top to bottom order!
#   - If the dimensions of m0 do not make sense,
#     this function terminates the program with exit code 72.
#   - If the dimensions of m1 do not make sense,
#     this function terminates the program with exit code 73.
#   - If the dimensions of m0 and m1 don't match,
#     this function terminates the program with exit code 74.
# =======================================================
matmul:
    # Error checks, in required top-to-bottom order
    ble a1, zero, err72
    ble a2, zero, err72

    ble a4, zero, err73
    ble a5, zero, err73

    bne a2, a4, err74

    # 9 registers × 4 bytes = 36 bytes.
    # Allocate 48 bytes to keep the stack 16-byte aligned.
    addi sp, sp, -48
    sw ra, 0(sp)
    sw s0, 4(sp)
    sw s1, 8(sp)
    sw s2, 12(sp)
    sw s3, 16(sp)
    sw s4, 20(sp)
    sw s5, 24(sp)
    sw s6, 28(sp)
    sw s7, 32(sp)

    mv s0, a0              # m0 base
    mv s1, a3              # m1 base
    mv s2, a6              # output base
    mv s3, a1              # R: rows of m0
    mv s4, a2              # K: cols of m0 / rows of m1
    mv s5, a5              # C: cols of m1

    li s6, 0               # i = 0

outer_loop:
    bge s6, s3, outer_done

    li s7, 0               # j = 0

inner_loop:
    bge s7, s5, inner_done

    # a0 = address of row i in m0
    # offset = i * K * 4
    mul t0, s6, s4
    slli t0, t0, 2
    add a0, s0, t0

    # a1 = address of element 0,j in m1
    # offset = j * 4
    slli t0, s7, 2
    add a1, s1, t0

    # dot(row i of m0, column j of m1, K, 1, C)
    mv a2, s4
    li a3, 1
    mv a4, s5

    jal ra, dot

    # d[i][j] address = d + (i * C + j) * 4
    # Recompute after dot because t-registers are caller-saved.
    mul t0, s6, s5
    add t0, t0, s7
    slli t0, t0, 2
    add t0, s2, t0

    sw a0, 0(t0)

    addi s7, s7, 1
    j inner_loop

inner_done:
    addi s6, s6, 1
    j outer_loop

outer_done:
    lw ra, 0(sp)
    lw s0, 4(sp)
    lw s1, 8(sp)
    lw s2, 12(sp)
    lw s3, 16(sp)
    lw s4, 20(sp)
    lw s5, 24(sp)
    lw s6, 28(sp)
    lw s7, 32(sp)
    addi sp, sp, 48
    ret

err72:
    li a0, 17
    li a1, 72
    ecall

err73:
    li a0, 17
    li a1, 73
    ecall

err74:
    li a0, 17
    li a1, 74
    ecall