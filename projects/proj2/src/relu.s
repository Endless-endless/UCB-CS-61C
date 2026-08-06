.globl relu

.text
# ==============================================================================
# FUNCTION: Performs an inplace element-wise ReLU on an array of ints
# Arguments:
# 	a0 (int*) is the pointer to the array
#	a1 (int)  is the # of elements in the array
# Returns:
#	None
# Exceptions:
# - If the length of the vector is less than 1,
#   this function terminates the program with error code 78.
# ==============================================================================
relu:
    # Prologue
    ble a1,zero,error_exit
    li t0,0

loop_start:
    bge t0,a1,loop_end

    li t3,4
    mul t2,t0,t3
    add t2,a0,t2
    lw t1,0(t2)

    bgt t1,zero,skip_zero
    li t1,0

skip_zero:
    sw t1,0(t2)

loop_continue:
    addi t0,t0,1
    j loop_start

loop_end:
    # Epilogue
	ret

error_exit:
    li a0, 78
    li a7, 93
    ecall

