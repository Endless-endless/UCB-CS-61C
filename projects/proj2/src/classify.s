.globl classify

.text
classify:
    # =====================================
    # COMMAND LINE ARGUMENTS
    # =====================================
    # Args:
    #   a0 (int)    argc
    #   a1 (char**) argv
    #   a2 (int)    print_classification, if this is zero, 
    #               you should print the classification. Otherwise,
    #               this function should not print ANYTHING.
    # Returns:
    #   a0 (int)    Classification
    # Exceptions:
    # - If there are an incorrect number of command line args,
    #   this function terminates the program with exit code 89.
    # - If malloc fails, this function terminats the program with exit code 88.
    #
    # Usage:
    #   main.s <M0_PATH> <M1_PATH> <INPUT_PATH> <OUTPUT_PATH>

    #
    # argv[0] = program
    # argv[1] = m0 path
    # argv[2] = m1 path
    # argv[3] = input path
    # argv[4] = output path

    # argc must be 5
    li t0, 5
    bne a0, t0, err_argc


    # =====================================
    # PROLOGUE
    # =====================================
    #
    # Register plan:
    #
    # s0 = argv
    # s1 = print flag
    # s2 = m0 pointer
    # s3 = m1 pointer
    # s4 = input pointer
    # s5 = hidden pointer
    # s6 = output/scores pointer
    # s7 = classification
    #
    # Stack locals:
    #
    # 40(sp) = m0_rows
    # 44(sp) = m0_cols
    # 48(sp) = m1_rows
    # 52(sp) = m1_cols
    # 56(sp) = input_rows
    # 60(sp) = input_cols
    #
    # 80 bytes keeps stack nicely aligned.

    addi sp, sp, -80

    sw ra, 0(sp)
    sw s0, 4(sp)
    sw s1, 8(sp)
    sw s2, 12(sp)
    sw s3, 16(sp)
    sw s4, 20(sp)
    sw s5, 24(sp)
    sw s6, 28(sp)
    sw s7, 32(sp)

    # argv and print flag must survive many jal calls
    mv s0, a1
    mv s1, a2

	# =====================================
    # LOAD MATRICES
    # =====================================

    # -------------------------------------
    # Load pretrained m0
    #
    # m0 = read_matrix(
    #     argv[1],
    #     &m0_rows,
    #     &m0_cols
    # )
    # -------------------------------------

    lw a0,4(s0)
    addi a1,sp,40
    addi a2,sp,44

    jal ra,read_matrix

    mv s2,a0

    # -------------------------------------
    # Load pretrained m1
    # -------------------------------------

    lw a0,8(s0)
    addi a1,sp,48
    addi a2,sp,52

    jal ra,read_matrix

    mv s3,a0

    # -------------------------------------
    # Load input matrix
    # -------------------------------------

    lw a0,12(s0)
    addi a1,sp,56
    addi a2,sp,60

    jal ra,read_matrix

    mv s4, a0

    # =====================================
    # RUN LAYERS
    # =====================================

    # -------------------------------------
    # 1. LINEAR:
    #
    # hidden = m0 * input
    #
    # m0:
    #   m0_rows x m0_cols
    #
    # input:
    #   input_rows x input_cols
    #
    # hidden:
    #   m0_rows x input_cols
    # -------------------------------------

        # malloc(m0_rows * input_cols * 4)

        lw t0,40(sp)  # m0_rows
        lw t1,60(sp)  # input_cols

        mul t0,t0,t1
        slli a0,t0,2

        jal ra,malloc

        beq a0,zero,err_malloc

        mv s5,a0  # hidden pointer


        # matmul(m0, input, hidden)

        mv a0,s2
        lw a1,40(sp)  # m0_rows
        lw a2,44(sp)  # m0_cols

        mv a3,s4
        lw a4,56(sp)  # input_rows
        lw a5,60(sp)  # input_cols

        mv a6,s5

        jal ra,matmul


    # -------------------------------------
    # 2. NONLINEAR:
    #
    # ReLU(hidden)
    #
    # length =
    # m0_rows * input_cols
    # -------------------------------------

        mv a0,s5

        lw t0,40(sp)
        lw t1,60(sp)

        mul a1,t0,t1

        jal ra,relu


    # -------------------------------------
    # 3. LINEAR:
    #
    # output = m1 * hidden
    #
    # m1:
    #   m1_rows x m1_cols
    #
    # hidden:
    #   m0_rows x input_cols
    #
    # output:
    #   m1_rows x input_cols
    # -------------------------------------

        # malloc output

        lw t0,48(sp)  # m1_rows
        lw t1,60(sp)  # input_cols

        mul t0,t0,t1
        slli a0,t0,2

        jal ra,malloc

        beq a0,zero,err_malloc

        mv s6,a0  # output / scores


        # matmul(m1, hidden, output)

        mv a0,s3
        lw a1,48(sp)  # m1_rows
        lw a2,52(sp)  # m1_cols

        mv a3,s5
        lw a4,40(sp)  # hidden rows = m0_rows
        lw a5,60(sp)  # hidden cols = input_cols

        mv a6, s6

        jal ra, matmul


# =====================================
# WRITE OUTPUT
# =====================================
#
# write_matrix(
#     argv[4],
#     output,
#     m1_rows,
#     input_cols
# )

    lw a0,16(s0)
    mv a1,s6
    lw a2,48(sp)
    lw a3,60(sp)

    jal ra,write_matrix


# =====================================
# CALCULATE CLASSIFICATION/LABEL
# =====================================
#
# classification = argmax(output)
#
# output length =
# m1_rows * input_cols
# =====================================

    mv a0,s6

    lw t0,48(sp)
    lw t1,60(sp)
    mul a1,t0,t1

    jal ra,argmax

    # classification must survive printing/free
    mv s7,a0


# =====================================
# PRINT CLASSIFICATION
# =====================================
#
# print iff print_flag == 0
# =====================================

    bne s1,zero,skip_print

    # print integer
    mv a1,s7
    li a0,1
    ecall

    # print newline
    li a1,'\n'
    li a0,11
    ecall


skip_print:

# =====================================
# FREE ALLOCATED MEMORY
# =====================================

    # m0
    mv a0,s2
    jal ra,free

    # m1
    mv a0,s3
    jal ra,free

    # input
    mv a0,s4
    jal ra,free

    # hidden
    mv a0,s5
    jal ra,free

    # output
    mv a0,s6
    jal ra,free


# =====================================
# RETURN CLASSIFICATION
# =====================================

    mv a0,s7


# =====================================
# EPILOGUE
# =====================================

    lw ra, 0(sp)
    lw s0, 4(sp)
    lw s1, 8(sp)
    lw s2, 12(sp)
    lw s3, 16(sp)
    lw s4, 20(sp)
    lw s5, 24(sp)
    lw s6, 28(sp)
    lw s7, 32(sp)

    addi sp, sp, 80

    ret


# =====================================
# ERROR HANDLERS
# =====================================

err_argc:
    li a1, 89
    li a0, 17
    ecall

err_malloc:
    li a1, 88
    li a0, 17
    ecall