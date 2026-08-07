.globl write_matrix

.text
# ==============================================================================
# FUNCTION: Writes a matrix of integers into a binary file
# FILE FORMAT:
#   The first 8 bytes of the file will be two 4 byte ints representing the
#   numbers of rows and columns respectively. Every 4 bytes thereafter is an
#   element of the matrix in row-major order.
# Arguments:
#   a0 (char*) is the pointer to string representing the filename
#   a1 (int*)  is the pointer to the start of the matrix in memory
#   a2 (int)   is the number of rows in the matrix
#   a3 (int)   is the number of columns in the matrix
# Returns:
#   None
# Exceptions:
# - If you receive an fopen error or eof,
#   this function terminates the program with error code 93.
# - If you receive an fwrite error or eof,
#   this function terminates the program with error code 94.
# - If you receive an fclose error or eof,
#   this function terminates the program with error code 95.
# ==============================================================================
write_matrix:

    # Prologue
    addi sp,sp,-32

    sw ra,0(sp)
    sw s0,4(sp)
    sw s1,8(sp)
    sw s2,12(sp)
    sw s3,16(sp)

    mv s1,a1
    mv s2,a2
    mv s3,a3

    mv a1,a0
    li a2,1
    jal ra,fopen
    mv s0,a0
    li t0,-1
    beq s0,t0,err_fopen

    sw s2,20(sp)
    sw s3,24(sp)

    mv a1,s0
    addi a2,sp,20
    li a3,1
    li a4,4
    jal ra,fwrite
    li t0,1
    bne a0,t0,err_fwrite

    mv a1,s0
    addi a2,sp,24
    li a3,1
    li a4,4
    jal ra,fwrite
    li t0,1
    bne a0,t0,err_fwrite

    mul t0,s2,s3
    mv a1,s0
    mv a2,s1
    mv a3,t0
    li a4,4
    jal ra,fwrite
    mul t0,s2,s3
    bne a0,t0,err_fwrite

    mv a1,s0
    jal ra,fclose
    bne a0,zero,err_fclose

    # Epilogue
    lw ra,0(sp)
    lw s0,4(sp)
    lw s1,8(sp)
    lw s2,12(sp)
    lw s3,16(sp)
    addi sp,sp,32

    ret

# ==============================================================================
# Error handlers
# ==============================================================================
err_fopen:
    li a1, 93
    li a0, 17
    ecall

err_fwrite:
    li a1, 94
    li a0, 17
    ecall

err_fclose:
    li a1, 95
    li a0, 17
    ecall