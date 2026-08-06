.globl dot

.text
# =======================================================
# FUNCTION: Dot product of 2 int vectors
# Arguments:
#   a0 (int*) is the pointer to the start of v0
#   a1 (int*) is the pointer to the start of v1
#   a2 (int)  is the length of the vectors
#   a3 (int)  is the stride of v0
#   a4 (int)  is the stride of v1
# Returns:
#   a0 (int)  is the dot product of v0 and v1
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 75.
# - If the stride of either vector is less than 1,
#   this function terminates the program with error code 76.
# =======================================================
dot:
    # 异常检测
    ble a2, zero, error_exit_1
    ble a3, zero, error_exit_2
    ble a4, zero, error_exit_2

    li t0, 0    # loop counter i
    li t1, 0    # sum = 0
    li t3, 4    # sizeof(int) = 4 bytes
    
loop_start:
    bge t0, a2, loop_end

    # &v0[i] = a0 + i * stride0 * 4
    mul t2, t0, a3
    mul t2, t2, t3
    add t2, a0, t2
    lw t4, 0(t2)

    # &v1[i] = a1 + i * stride1 * 4
    mul t5, t0, a4
    mul t5, t5, t3
    add t5, a1, t5
    lw t6, 0(t5)

    # accumulate product
    mul t2, t4, t6
    add t1, t1, t2

    addi t0, t0, 1
    j loop_start

loop_end:
    mv a0, t1
    ret

error_exit_1:
    li a0, 75
    li a7, 93
    ecall

error_exit_2:
    li a0, 76
    li a7, 93
    ecall