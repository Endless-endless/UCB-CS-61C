.globl abs

.text
# =================================================================
# FUNCTION: Given an int return its absolute value.
# Arguments:
# 	a0 (int) is input integer
# Returns:
#	a0 (int) the absolute value of the input
# =================================================================
abs:
    # branch if positive
    bge a0, x0, done
    # invert a if negative
    sub a0, x0, a0
done:
    ret
