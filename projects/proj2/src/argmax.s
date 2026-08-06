.globl argmax

.text
# =================================================================
# FUNCTION: Given a int vector, return the index of the largest
#	element. If there are multiple, return the one
#	with the smallest index.
# Arguments:
# 	a0 (int*) is the pointer to the start of the vector
#	a1 (int)  is the # of elements in the vector
# Returns:
#	a0 (int)  is the first index of the largest element
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 77.
# =================================================================
argmax:

    # Prologue
    ble a1,zero,error_exit
    li t0,0
    li t1,0

loop_start:
    bge t0,a1,loop_end

    li t3,4
    mul t2,t0,t3
    add t2,a0,t2
    lw t5,0(t2)

    li t3,4
    mul t4,t1,t3
    add t4,a0,t4
    lw  t6,0(t4)

    bgt t5,t6,update_idx
    j loop_continue

update_idx:
    mv t1,t0

loop_continue:
    addi t0,t0,1
    j loop_start

loop_end:
    # Epilogue
    mv a0,t1
    ret

error_exit:
    li a0,77
    li a7,93
    ecall
