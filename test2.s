.org 0x0000
.text
main:
    li a0, 0              # Load immediate 0 into register a0
    li t0, 5              # Load immediate 5 into register t0
    bz a0, else           # Branch to 'else' if a0 == 0 (which it is, so it will branch)

    ecall 1               # This won't be executed because of the branch to 'else'
label1:
    addi t0, t0, -5       # This won't be executed because of the branch to 'else'

else:
    addi t0, 2            # Add 2 to t0 (t0 becomes 5 + 2 = 7)
    ecall 1               # Invoke system call 1 (prints the value of a0, which is still 0)
    ecall 3               # Invoke system call 3 (terminate the program)
