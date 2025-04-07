#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MEM_SIZE 65536 // 64KB memory

// Simulated memory and register file
unsigned char memory[MEM_SIZE];
uint16_t regs[8]; // 8 registers: x0 - x7
uint16_t pc = 0; // Program counter

// Register names
const char *regNames[8] = {"t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1"};

// Function to disassemble instructions
void disassemble(uint16_t inst, uint16_t pc, char *buf, size_t bufSize) {
    uint8_t opcode = inst & 0x7;

    switch (opcode) {
        case 0x0: { // R-Type
            uint8_t funct4 = (inst >> 12) & 0xF;
            uint8_t rs2 = (inst >> 9) & 0x7;
            uint8_t rs1 = (inst >> 6) & 0x7;
            uint8_t funct3 = (inst >> 3) & 0x7;
            snprintf(buf, bufSize, "R-Type (%X) %s, %s", funct4, regNames[rs1], regNames[rs2]);
            break;
        }
        case 0x1: { // I-Type
            uint16_t imm = (inst >> 9) & 0x7F; // 7-bit immediate
            uint8_t rs1 = (inst >> 6) & 0x7;   // rs1 (bits 6-8)
            uint8_t funct3 = (inst >> 3) & 0x7; // funct3 (bits 3-5)
            snprintf(buf, bufSize, "I-Type imm: %d, rs1: %s", imm, regNames[rs1]);
            break;
        }
        case 0x2: { // B-Type (Branch)
            int16_t imm = ((inst >> 12) & 0xF) | ((inst >> 1) & 0xE);
            if (imm & 0x10) imm |= 0xFFE0;  // Sign-extend if necessary
            uint8_t rs2 = (inst >> 9) & 0x7;
            uint8_t rs1 = (inst >> 6) & 0x7;
            snprintf(buf, bufSize, "B-Type offset: %d, %s, %s", imm, regNames[rs1], regNames[rs2]);
            break;
        }
        case 0x3: { // S-Type (Store)
            uint8_t imm = (inst >> 12) & 0xF;  // Immediate (bits 12-15)
            uint8_t rs2 = (inst >> 9) & 0x7;   // rs2 (bits 9-11)
            uint8_t rs1 = (inst >> 6) & 0x7;   // rs1 (bits 6-8)
            snprintf(buf, bufSize, "S-Type Store %s -> [%s + %d]", regNames[rs2], regNames[rs1], imm);
            break;
        }
        case 0x4: { // L-Type (Load)
            uint8_t imm = (inst >> 12) & 0xF;  // Immediate (bits 12-15)
            uint8_t rd = (inst >> 9) & 0x7;    // rd (bits 6-8)
            uint8_t rs1 = (inst >> 6) & 0x7;   // rs1 (bits 3-5)
            snprintf(buf, bufSize, "L-Type Load %s <- [%s + %d]", regNames[rd], regNames[rs1], imm);
            break;
        }
        case 0x5: { // J-Type (Jump)
            uint16_t imm = ((inst >> 4) & 0x3F) | ((inst >> 1) & 0xE);
            uint8_t rd = (inst >> 9) & 0x7;    // rd (bits 6-8)
            snprintf(buf, bufSize, "J-Type Jump to %d", imm);
            break;
        }
        case 0x6: { // U-Type (Upper Immediate)
            uint8_t imm = (inst >> 10) & 0x7;   // I[9:7] (bits 9-7)
            uint8_t rd = (inst >> 9) & 0x7;     // rd (bits 6-8)
            snprintf(buf, bufSize, "U-Type LUI/AUIPC %s = %d", regNames[rd], imm);
            break;
        }
        case 0x7: { // System Instructions (e.g., ECALL)
            snprintf(buf, bufSize, "ecall");
            break;
        }

        default:
            snprintf(buf, bufSize, "Unknown Instruction");
    }
}

int executeInstruction(uint16_t inst) {
    if (inst == 0x0000) {
        return 0;  // Stopping infinite loop (error)
    }

    uint8_t opcode = inst & 0x7;  // Extract opcode (last 3 bits)
    int pcUpdated = 0;

    switch (opcode) {
case 0x0: { // R-Type (opcode 000)
    uint8_t opcode = (inst >> 0) & 0x7;   // Extract opcode from bits 0-2 (3 bits)
    uint8_t funct3 = (inst >> 3) & 0x7;   // Extract funct3 from bits 3-5 (3 bits)
    uint8_t rs1 = (inst >> 6) & 0x7;      // Extract rs1 from bits 6-8 (3 bits)
    uint8_t rs2 = (inst >> 9) & 0x7;      // Extract rs2 from bits 9-11 (3 bits)
    uint8_t funct4 = (inst >> 12) & 0xF;   // Extract funct4 from bits 12-15 (4 bits)

    // Ensure the opcode is as expected for R-Type (should be 000)
    if (opcode != 0x0) {
        printf("⚠️ Invalid R-Type instruction: opcode=%X\n", opcode);
        return 0;
    }

    // Print the R-Type instruction (for debugging)
    char *op = "";
    switch (funct3) {
        case 0x0:
            switch (funct4) {
                case 0x0: op = "ADD"; break;
                case 0x1: op = "SUB"; break;
                case 0x4: op = "JR"; break;
                case 0x8: op = "JALR"; break;
                default: op = "Unknown"; break;
            }
            break;
        case 0x3:
            switch (funct4) {
                case 0x2: op = "SLL"; break;
                case 0x4: op = "SRL"; break;
                case 0x8: op = "SRA"; break;
                default: op = "Unknown"; break;
            }
            break;
        case 0x4: op = "OR"; break;
        case 0x5: op = "AND"; break;
        case 0x6: op = "XOR"; break;
        case 0x7: op = "SLT"; break;
        default: op = "Unknown"; break;
    }

    // Print the R-Type instruction with the operation
    printf("R-Type: %s %s %s\n", regNames[rs1], op, regNames[rs2]);

    // Handle different R-Type operations based on funct3 and funct4
    switch (funct3) {
        case 0x0:
            switch (funct4) {
                case 0x0: { // ADD
                    regs[rs1] += regs[rs2]; // Add rs2 to rs1 and store in rs1
                    break;
                }
                case 0x1: { // SUB
                    regs[rs1] -= regs[rs2]; // Subtract rs2 from rs1 and store in rs1
                    break;
                }
                case 0x4: { // JR (Jump register)
                    // Example: jumping to address in rs1
                    pc = regs[rs1]; // Update PC to the value in rs1
                    break;
                }
                case 0x8: { // JALR (Jump and link register)
                    // Example: jump to address in rs1 and store return address in rs2
                    regs[rs2] = pc;  // Store the current PC value in rs2
                    pc = regs[rs1];  // Jump to address in rs1
                    break;
                }
                default:
                    printf("⚠️ Unknown R-Type instruction: funct4=%X\n", funct4);
                    return 0;
            }
            break;

        case 0x3: {
            switch (funct4) {
                case 0x2: { // SLL (Shift left logical)
                    regs[rs1] <<= regs[rs2]; // Shift rs1 left by rs2 bits
                    break;
                }
                case 0x4: { // SRL (Shift right logical)
                    regs[rs1] >>= regs[rs2]; // Shift rs1 right by rs2 bits
                    break;
                }
                case 0x8: { // SRA (Shift right arithmetic)
                    regs[rs1] = (int32_t)regs[rs1] >> regs[rs2]; // Arithmetic shift right
                    break;
                }
                default:
                    printf("⚠️ Unknown R-Type instruction: funct4=%X\n", funct4);
                    return 0;
            }
            break;
        }

        case 0x4: { // OR
            regs[rs1] |= regs[rs2]; // Bitwise OR between rs1 and rs2
            break;
        }

        case 0x5: { // AND
            regs[rs1] &= regs[rs2]; // Bitwise AND between rs1 and rs2
            break;
        }

        case 0x6: { // XOR
            regs[rs1] ^= regs[rs2]; // Bitwise XOR between rs1 and rs2
            break;
        }

        case 0x7: { // SLT (Set less than)
            regs[rs1] = (regs[rs1] < regs[rs2]) ? 1 : 0; // Set rs1 to 1 if rs1 < rs2, else 0
            break;
        }

        default:
            printf("⚠️ Unknown R-Type instruction: funct3=%X\n", funct3);
            return 0;
    }

    break;
}

case 0x1: { // I-Type (opcode 001)
    uint8_t funct3 = (inst >> 3) & 0x7;
    uint8_t rs1 = (inst >> 6) & 0x7;
    uint16_t imm7 = (inst >> 9) & 0x7F;

    switch (funct3) {
        case 0x0: { // ADDI
            regs[rs1] += imm7;
            printf("ADDI: %s += %d → %d\n", regNames[rs1], imm7, regs[rs1]);
            break;
        }

        case 0x1: { // SLTI
            regs[rs1] = (regs[rs1] < imm7) ? 1 : 0;
            printf("SLTI: %s = %d (if %s < %d)\n", regNames[rs1], regs[rs1], regNames[rs1], imm7);
            break;
        }

        case 0x2: { // SLTUI
            regs[rs1] = ((unsigned)regs[rs1] < (unsigned)imm7) ? 1 : 0;
            printf("SLTUI: %s = %d (if %s < %d unsigned)\n", regNames[rs1], regs[rs1], regNames[rs1], imm7);
            break;
        }

        case 0x3: { // Shift instructions (SLLI, SRLI, SRAI) - disambiguate with shiftType
            uint8_t shiftType = (imm7 >> 5) & 0x3;
            uint8_t shamt = imm7 & 0x1F;

            switch (shiftType) {
                case 0x1: // SLLI
                    regs[rs1] <<= shamt;
                    printf("SLLI: %s <<= %d → %d\n", regNames[rs1], shamt, regs[rs1]);
                    break;
                case 0x2: // SRLI
                    regs[rs1] >>= shamt;
                    printf("SRLI: %s >>= %d → %d\n", regNames[rs1], shamt, regs[rs1]);
                    break;
                case 0x3: // SRAI
                    regs[rs1] = (int32_t)regs[rs1] >> shamt;
                    printf("SRAI: (int32_t)%s >> %d → %d\n", regNames[rs1], shamt, regs[rs1]);
                    break;
                default:
                    printf("⚠️ Unknown shift type (shiftType=%d) in funct3=0x3\n", shiftType);
                    return 0;
            }
            break;
        }

        case 0x4: { // ORI
            regs[rs1] |= imm7;
            printf("ORI: %s |= %d → %d\n", regNames[rs1], imm7, regs[rs1]);
            break;
        }

        case 0x5: { // ANDI
            regs[rs1] &= imm7;
            printf("ANDI: %s &= %d → %d\n", regNames[rs1], imm7, regs[rs1]);
            break;
        }

        case 0x6: { // XORI
            regs[rs1] ^= imm7;
            printf("XORI: %s ^= %d → %d\n", regNames[rs1], imm7, regs[rs1]);
            break;
        }

        case 0x7: { // LI (custom: load immediate, funct3 = 111)
            regs[rs1] = imm7;
            printf("LI: %s = %d\n", regNames[rs1], regs[rs1]);
            break;
        }

        default:
            printf("⚠️ Unknown I-Type funct3=%X\n", funct3);
            return 0;
    }

    break;

}case 0x2: { // Branch-Type (opcode 010)
        uint8_t funct3 = (inst >> 3) & 0x7;
        uint8_t rs1 = (inst >> 6) & 0x7;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t imm4_1 = (inst >> 12) & 0xF;

        int8_t offset = imm4_1 << 1;
        if (imm4_1 & 0x8) offset |= 0xF0;  // Sign-extend negative

    switch (funct3) {
        case 0x0: { // BEQ
            if (regs[rs1] == regs[rs2]) {
                pc += offset;
                printf("BEQ: %s == %s → PC += %d → %d\n", regNames[rs1], regNames[rs2], offset, pc);
            } else {
                printf("BEQ: %s != %s → no branch\n", regNames[rs1], regNames[rs2]);
            }
            break;
        }

        case 0x1: { // BNE
            if (regs[rs1] != regs[rs2]) {
                pc += offset;
                printf("BNE: %s != %s → PC += %d → %d\n", regNames[rs1], regNames[rs2], offset, pc);
            } else {
                printf("BNE: %s == %s → no branch\n", regNames[rs1], regNames[rs2]);
            }
            break;
        }

        case 0x2: { // BZ (branch if zero)
            if (regs[rs1] == 0) {
                pc += offset;
                printf("BZ: %s == 0 → PC += %d → %d\n", regNames[rs1], offset, pc);
            } else {
                printf("BZ: %s != 0 → no branch\n", regNames[rs1]);
            }
            break;
        }

        case 0x3: { // BNZ (branch if not zero)
            if (regs[rs1] != 0) {
                pc += offset;
                printf("BNZ: %s != 0 → PC += %d → %d\n", regNames[rs1], offset, pc);
            } else {
                printf("BNZ: %s == 0 → no branch\n", regNames[rs1]);
            }
            break;
        }

        case 0x4: { // BLT (signed)
            if ((int32_t)regs[rs1] < (int32_t)regs[rs2]) {
                pc += offset;
                printf("BLT: %s < %s → PC += %d → %d\n", regNames[rs1], regNames[rs2], offset, pc);
            } else {
                printf("BLT: %s >= %s → no branch\n", regNames[rs1], regNames[rs2]);
            }
            break;
        }

        case 0x5: { // BGE (signed)
            if ((int32_t)regs[rs1] >= (int32_t)regs[rs2]) {
                pc += offset;
                printf("BGE: %s >= %s → PC += %d → %d\n", regNames[rs1], regNames[rs2], offset, pc);
            } else {
                printf("BGE: %s < %s → no branch\n", regNames[rs1], regNames[rs2]);
            }
            break;
        }

        case 0x6: { // BLTU (unsigned)
            if ((uint32_t)regs[rs1] < (uint32_t)regs[rs2]) {
                pc += offset;
                printf("BLTU: %s < %s (unsigned) → PC += %d → %d\n", regNames[rs1], regNames[rs2], offset, pc);
            } else {
                printf("BLTU: %s >= %s (unsigned) → no branch\n", regNames[rs1], regNames[rs2]);
            }
            break;
        }

        case 0x7: { // BGEU (unsigned)
            if ((uint32_t)regs[rs1] >= (uint32_t)regs[rs2]) {
                pc += offset;
                printf("BGEU: %s >= %s (unsigned) → PC += %d → %d\n", regNames[rs1], regNames[rs2], offset, pc);
            } else {
                printf("BGEU: %s < %s (unsigned) → no branch\n", regNames[rs1], regNames[rs2]);
            }
            break;
        }

        default:
            printf("⚠️ Unknown Branch-Type funct3=%X\n", funct3);
            return 0;
    }

    break;
}

        case 0x3: { // Store-Type (opcode 011)
        uint8_t funct3 = (inst >> 3) & 0x7;
        uint8_t rs1 = (inst >> 6) & 0x7;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t imm4_1 = (inst >> 12) & 0xF; // Immediate [3:0]

        // Sign-extend the immediate (I[3:0] -> 12 bits)
        int16_t offset = imm4_1;  // In our case, I[3:0] is just 4 bits
        if (imm4_1 & 0x8) offset |= 0xF0;  // Sign-extend to 12 bits if negative

        // Base address is in rs1, data to store is in rs2
        switch (funct3) {
            case 0x0: // SB (Store Byte)
                printf("SB: Storing byte to address in a0 (rs1) from a1 (rs2), offset = %d\n", offset);
            // Memory store operation for byte, assume memory is represented as an array `memory`
            memory[regs[rs1] + offset] = regs[rs2] & 0xFF; // Store byte from rs2
            break;

            case 0x1: // SW (Store Word)
                printf("SW: Storing word to address in a0 (rs1) from a1 (rs2), offset = %d\n", offset);
            // Memory store operation for word (4 bytes)
            *(uint32_t*)&memory[regs[rs1] + offset] = regs[rs2]; // Store word from rs2
            break;

            default:
                printf("⚠️ Unknown Store funct3: %X\n", funct3);
            return 0;
        }
        break;
        }

        case 0x4: { // Load-Type (opcode 100)
        uint8_t funct3 = (inst >> 3) & 0x7;
        uint8_t rd = (inst >> 6) & 0x7;
        uint8_t rs2 = (inst >> 9) & 0x7;  // Not used, for consistency
        uint8_t imm4_1 = (inst >> 12) & 0xF; // Immediate [3:0]

        // Sign-extend the immediate (I[3:0] -> 12 bits)
        int16_t offset = imm4_1;  // In our case, I[3:0] is just 4 bits
        if (imm4_1 & 0x8) offset |= 0xF0;  // Sign-extend to 12 bits if negative

        // Load from memory based on funct3
        switch (funct3) {
            case 0x0: // LB (Load Byte)
                printf("LB: Loading byte from address in a0 (rs1) with offset = %d\n", offset);
            // Memory load operation for byte
            regs[rd] = (int8_t)memory[regs[rs2] + offset]; // Load signed byte from memory
            break;

            case 0x1: // LW (Load Word)
                printf("LW: Loading word from address in a0 (rs1) with offset = %d\n", offset);
            // Memory load operation for word (4 bytes)
            regs[rd] = *(int32_t*)&memory[regs[rs2] + offset]; // Load word from memory
            break;

            case 0x4: // LBU (Load Byte Unsigned)
                printf("LBU: Loading byte unsigned from address in a0 (rs1) with offset = %d\n", offset);
            // Memory load operation for unsigned byte
            regs[rd] = (uint8_t)memory[regs[rs2] + offset]; // Load unsigned byte from memory
            break;

            default:
                printf("⚠️ Unknown Load funct3: %X\n", funct3);
            return 0;
        }
        break;
        }
        case 0x5: { // J-Type (opcode 101)
        uint8_t I_3_1 = (inst >> 3) & 0x7;    // Bits 3-5: I[3:1]
        uint8_t rd = (inst >> 6) & 0x7;        // Bits 6-8: rd (destination register, not used for J)
        uint8_t I_9_4 = (inst >> 9) & 0x3F;    // Bits 9-14: I[9:4]
        uint8_t f = (inst >> 15) & 0x1;         // Bit 15: flag (0 for J, 1 for JAL)

        // Combine I[9:4] and I[3:1] to form the complete immediate (JAL offset)
        int32_t offset = ((int32_t)I_9_4 << 3) | I_3_1; // Shift I[9:4] and OR with I[3:1]

        // Sign-extend the immediate to 32-bit
        if (offset & 0x100) { // If bit 8 (sign bit) is set
            offset |= 0xFFFFFFE0; // Sign-extend to 32-bit
        }

        // Compute the jump target (address) from the current PC
        int32_t target = pc + (offset << 1);  // Multiply offset by 2 (since RISC-V uses byte addresses)

        if (f == 0) {  // J (Jump)
            printf("J: Jumping to address %X\n", target);
            pc = target; // Set the PC to the target address
        } else {  // JAL (Jump and Link)
            printf("JAL: Jumping to address %X and storing return address in rd\n", target);
            regs[rd] = pc + 4;  // Store the address of the next instruction (return address) in rd
            pc = target; // Set the PC to the target address
        }
        break;
        }
        case 0x6: { // U-Type (opcode 110)
        uint8_t I_9_7 = (inst >> 3) & 0x7;    // Bits 3-5: I[9:7]
        uint8_t rd = (inst >> 6) & 0x7;        // Bits 6-8: rd (destination register)
        uint8_t I_15_10 = (inst >> 9) & 0x3F;  // Bits 9-14: I[15:10]
        uint8_t f = (inst >> 15) & 0x1;        // Bit 15: flag (0 for JLUI and AUIPC)

        // Combine I[15:10] and I[9:7] to form the complete immediate
        int32_t immediate = (I_15_10 << 7) | I_9_7; // Shift I[15:10] and OR with I[9:7]

        // Sign-extend the immediate to 32-bit
        if (immediate & 0x1000) { // If bit 12 (sign bit) is set
            immediate |= 0xFFFFE000; // Sign-extend to 32-bit
        }

        // Perform the appropriate operation based on the flag (f)
        if (f == 0) {  // JLUI (Load Upper Immediate)
            printf("JLUI: Setting rd to upper 20-bit immediate %X\n", immediate << 12);
            regs[rd] = immediate << 12;  // Set rd to the upper 20 bits of the immediate
        } else {  // AUIPC (Add Upper Immediate to PC)
            printf("AUIPC: Adding upper 20-bit immediate %X to PC\n", immediate << 12);
            regs[rd] = pc + (immediate << 12);  // Add the immediate shifted by 12 to the current PC
        }
        break;
        }

        case 0x7: {  // SYS-Type: ECALL instruction (system call)
            uint8_t service = (inst >> 3) & 0xF;  // ECALL service code is in register a7 (regs[7])
            printf("Decoded ECALL service: %d\n", service);  // Debugging line to track service number

            if (service == 1) {  // ECALL 1: Print the integer in a0
                printf("Printing integer from a0: %d\n", regs[6]);  // Debugging print for the integer
                fflush(stdout);  // Ensure the output is printed immediately
            } else if (service == 5) {  // ECALL 5: Print a NULL-terminated string (address in a0)
                char *str = (char*)regs[6];  // a0 is typically in regs[6], pointing to string address
                printf("Printing string from a0: %s\n", str);  // Print the string
                fflush(stdout);
            } else if (service == 3) {  // ECALL 3: Terminate the program
                printf("Program terminated successfully!\n");
                return 0;  // Exit the program
            } else {
                printf("Unknown ECALL service: %d\n", service);  // Handle unknown services
            }
            break;
        }
        default:
            printf("Unknown opcode: %X at PC=0x%04X\n", opcode, pc);
            return 0;
    }

    printf("Registers: ");
    for (int i = 0; i < 8; i++) {
        printf("r%d=%d ", i, regs[i]);
    }
    printf("\n");

    if (!pcUpdated) {
        pc += 2;
    }

    return 1;
}




// -----------------------
// Memory Loading
// -----------------------
//

void loadMemoryFromFile(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if(!fp) {
        perror("Error opening binary file");
        exit(1);
    }
    size_t n = fread(memory, 1, MEM_SIZE, fp);
    fclose(fp);
    //printf("Loaded %zu bytes into memory\n", n);
}
// -----------------------
// Main Simulation Loop
// -----------------------
int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <machine_code_file>\n", argv[0]);
        exit(1);
    }
    loadMemoryFromFile(argv[1]);
    memset(regs, 0, sizeof(regs)); // initialize registers to 0
    pc = 0; // starting at address 0
    char disasmBuf[128];
    while(pc < MEM_SIZE) {
        // Fetch a 16-bit instruction from memory (little-endian)
        uint16_t inst = memory[pc] | (memory[pc+1] << 8);
        disassemble(inst, pc, disasmBuf, sizeof(disasmBuf));
        //printf("0x%04X: %04X %s\n", pc, inst, disasmBuf);
        if(!executeInstruction(inst))
            break;
        // Terminate if PC goes out of bounds
        if(pc >= MEM_SIZE) break;
    }
    return 0;
}
