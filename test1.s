.org 0x0000
.text
start:
   li   a0, 5          ; load 5 into a0
   li   a1, 5          ; load 5 into a1
   beq  a0, a1, equal  ; branch if a0 == a1
   li   t0, 10         ; if not equal, set t0 to 10
   j    end            ; Jump to end
equal:
   li   a0, 20         ; If equal, set a0 to 20
end:
   ecall 1             ; ECALL 1: Print the result (expected output: 20)
   ecall 3             ; ECALL 3: Terminate the program
