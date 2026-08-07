.globl read_matrix

.text
# ==============================================================================
# FUNCTION: Allocates memory and reads in a binary file as a matrix of integers
#
# FILE FORMAT:
#   The first 8 bytes are two 4 byte ints representing the # of rows and columns
#   in the matrix. Every 4 bytes afterwards is an element of the matrix in
#   row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is a pointer to an integer, we will set it to the number of rows
#   a2 (int*)  is a pointer to an integer, we will set it to the number of columns
# Returns:
#   a0 (int*)  is the pointer to the matrix in memory
# Exceptions:
# - If malloc returns an error,
#   this function terminates the program with error code 88.
# - If you receive an fopen error or eof, 
#   this function terminates the program with error code 90.
# - If you receive an fread error or eof,
#   this function terminates the program with error code 91.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 92.
# ==============================================================================
read_matrix:

    # --------------------------------------------------
    # Prologue
    #
    # s0 = file descriptor
    # s1 = out_rows pointer
    # s2 = out_cols pointer
    # s3 = matrix pointer
    # s4 = rows
    # s5 = cols
    # --------------------------------------------------

    addi sp,sp,-32

    sw ra,0(sp)
    sw s0,4(sp)
    sw s1,8(sp)
    sw s2,12(sp)
    sw s3,16(sp)
    sw s4,20(sp)
    sw s5,24(sp)

    mv s1,a1
    mv s2,a2


    # --------------------------------------------------
    # fopen(filename, 0)
    # --------------------------------------------------

    mv a1,a0
    li a2,0

    jal ra,fopen

    mv s0,a0

    li t0,-1
    beq s0,t0,err_fopen


    # --------------------------------------------------
    # Read rows and cols
    #
    # Temporarily use 8 bytes on stack:
    #
    # sp+0 : rows
    # sp+4 : cols
    # --------------------------------------------------

    addi sp,sp,-8

    mv a1,s0
    mv a2,sp
    li a3,8

    jal ra,fread

    li t0,8
    bne a0,t0,err_fread

    # save dimensions into saved registers
    lw s4,0(sp)
    lw s5,4(sp)

    addi sp,sp,8


    # --------------------------------------------------
    # Write dimensions back to caller
    #
    # *out_rows = rows
    # *out_cols = cols
    # --------------------------------------------------

    sw s4,0(s1)
    sw s5,0(s2)


    # --------------------------------------------------
    # malloc(rows * cols * 4)
    # --------------------------------------------------

    mul t0,s4,s5
    slli a0,t0,2

    jal ra,malloc

    beq a0,zero,err_malloc

    mv s3,a0


    # --------------------------------------------------
    # fread(fd, matrix, rows * cols * 4)
    # --------------------------------------------------

    mul t0,s4,s5
    slli t0,t0,2

    mv a1,s0
    mv a2,s3
    mv a3,t0

    jal ra,fread

    # t0 may have been destroyed by fread,
    # so recompute expected byte count

    mul t0,s4,s5
    slli t0,t0,2

    bne a0,t0,err_fread


    # --------------------------------------------------
    # fclose(fd)
    # --------------------------------------------------

    mv a1,s0

    jal ra,fclose

    bne a0,zero,err_fclose


    # --------------------------------------------------
    # Return matrix pointer
    # --------------------------------------------------

    mv a0,s3


    # --------------------------------------------------
    # Epilogue
    # --------------------------------------------------

    lw ra,0(sp)
    lw s0,4(sp)
    lw s1,8(sp)
    lw s2,12(sp)
    lw s3,16(sp)
    lw s4,20(sp)
    lw s5,24(sp)

    addi sp,sp,32

    ret


# ==============================================================================
# Error handlers
# ==============================================================================

err_malloc:
    li a1,88
    li a0,17
    ecall

err_fopen:
    li a1,90
    li a0,17
    ecall

err_fread:
    li a1,91
    li a0,17
    ecall

err_fclose:
    li a1,92
    li a0,17
    ecall
