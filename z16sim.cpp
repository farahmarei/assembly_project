/*
 * Z16 Instruction Set Simulator (ISS)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Mohamed Shalan
 *
 * This simulator accepts a Z16 binary machine code file (with a .bin extension) and assumes that
 * the first instruction is located at memory address 0x0000. It decodes each 16-bit instruction into a
 * human-readable string and prints it, then executes the instruction by updating registers, memory,
 * or performing I/O via ecall.
 *
 * Supported ecall services:
 *   - ecall 1: Print an integer (value in register a0).
 *   - ecall 5: Print a NULL-terminated string (address in register a0).
 *   - ecall 3: Terminate the simulation.
 *
 * Usage:
 *   z16sim <machine_code_file_name>
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MEM_SIZE 65536  // 64KB memory

 // Global simulated memory and register file.
unsigned char memory[MEM_SIZE];
uint16_t regs[8];      // 8 registers (16-bit each): x0, x1, x2, x3, x4, x5, x6, x7
uint16_t pc = 0;       // Program counter (16-bit)

// Register ABI names for display (x0 = t0, x1 = ra, x2 = sp, x3 = s0, x4 = s1, x5 = t1, x6 = a0, x7 = a1)
const char* regNames[8] = { "t0", "ra", "sp", "s0", "s1", "t1", "a0", "a1" };

// -----------------------
// Disassembly Function
// -----------------------
void disassemble(uint16_t inst, uint16_t pc, char* buf, size_t bufSize) {
    uint8_t opcode = inst & 0x7;
    switch (opcode) {
    case 0x0: { // R-type
        uint8_t funct4 = (inst >> 12) & 0xF;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t rd_rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;

        if (funct4 == 0x0 && funct3 == 0x0)
            snprintf(buf, bufSize, "add %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x1 && funct3 == 0x0)
            snprintf(buf, bufSize, "sub %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x2 && funct3 == 0x0)
            snprintf(buf, bufSize, "and %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x3 && funct3 == 0x0)
            snprintf(buf, bufSize, "or %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x4 && funct3 == 0x0)
            snprintf(buf, bufSize, "xor %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x5 && funct3 == 0x0)
            snprintf(buf, bufSize, "sll %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x6 && funct3 == 0x0)
            snprintf(buf, bufSize, "srl %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x7 && funct3 == 0x0)
            snprintf(buf, bufSize, "sra %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x8 && funct3 == 0x0)
            snprintf(buf, bufSize, "slt %s, %s", regNames[rd_rs1], regNames[rs2]);
        else if (funct4 == 0x9 && funct3 == 0x0)
            snprintf(buf, bufSize, "sltu %s, %s", regNames[rd_rs1], regNames[rs2]);
        else
            snprintf(buf, bufSize, "unknown R-type instruction");
        break;
    }
    case 0x1: { // I-type
        uint8_t imm7 = (inst >> 9) & 0x7F;
        uint8_t rd_rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;
        int16_t simm = (imm7 & 0x40) ? (imm7 | 0xFF80) : imm7;

        switch (funct3) {
        case 0x0: snprintf(buf, bufSize, "addi %s, %d", regNames[rd_rs1], simm); break;
        case 0x1: snprintf(buf, bufSize, "andi %s, %d", regNames[rd_rs1], imm7); break;
        case 0x2: snprintf(buf, bufSize, "ori %s, %d", regNames[rd_rs1], imm7); break;
        case 0x3: snprintf(buf, bufSize, "xori %s, %d", regNames[rd_rs1], imm7); break;
        case 0x4: snprintf(buf, bufSize, "slli %s, %d", regNames[rd_rs1], imm7 & 0xF); break;
        case 0x5: snprintf(buf, bufSize, "srli %s, %d", regNames[rd_rs1], imm7 & 0xF); break;
        case 0x6: snprintf(buf, bufSize, "srai %s, %d", regNames[rd_rs1], imm7 & 0xF); break;
        case 0x7: snprintf(buf, bufSize, "slti %s, %d", regNames[rd_rs1], simm); break;
        default: snprintf(buf, bufSize, "unknown I-type instruction"); break;
        }
        break;
    }
    case 0x2: { // B-type
        uint8_t offset4 = (inst >> 12) & 0xF;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;
        int16_t offset = (offset4 << 1) | ((inst >> 8) & 0x1);
        offset = (offset << 7) >> 7; // sign extend

        const char* mnemonic = "";
        switch (funct3) {
        case 0x0: mnemonic = "beq"; break;
        case 0x1: mnemonic = "bne"; break;
        case 0x2: mnemonic = "blt"; break;
        case 0x3: mnemonic = "bge"; break;
        case 0x4: mnemonic = "bltu"; break;
        case 0x5: mnemonic = "bgeu"; break;
        default: mnemonic = "unknown"; break;
        }
        snprintf(buf, bufSize, "%s %s, %s, %d", mnemonic, regNames[rs1], regNames[rs2], offset);
        break;
    }
    case 0x3: { // L-type (load/store)
        uint8_t offset4 = (inst >> 12) & 0xF;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;
        int16_t offset = (offset4 << 1) | ((inst >> 8) & 0x1);
        offset = (offset << 7) >> 7; // sign extend

        if (funct3 == 0x0)
            snprintf(buf, bufSize, "sb %s, %d(%s)", regNames[rs2], offset, regNames[rs1]);
        else if (funct3 == 0x1)
            snprintf(buf, bufSize, "sh %s, %d(%s)", regNames[rs2], offset, regNames[rs1]);
        else if (funct3 == 0x2)
            snprintf(buf, bufSize, "lb %s, %d(%s)", regNames[rs2], offset, regNames[rs1]);
        else if (funct3 == 0x3)
            snprintf(buf, bufSize, "lh %s, %d(%s)", regNames[rs2], offset, regNames[rs1]);
        else if (funct3 == 0x4)
            snprintf(buf, bufSize, "lbu %s, %d(%s)", regNames[rs2], offset, regNames[rs1]);
        else if (funct3 == 0x5)
            snprintf(buf, bufSize, "lhu %s, %d(%s)", regNames[rs2], offset, regNames[rs1]);
        else
            snprintf(buf, bufSize, "unknown L-type instruction");
        break;
    }
    case 0x4: { // J-type (jal)
        uint16_t offset = (inst >> 3) & 0x1FFF;
        offset = (offset << 3) >> 3; // sign extend
        snprintf(buf, bufSize, "jal %s, %d", regNames[(inst >> 6) & 0x7], offset);
        break;
    }
    case 0x5: { // J-type (jalr)
        uint8_t offset7 = (inst >> 9) & 0x7F;
        uint8_t rd = (inst >> 6) & 0x7;
        uint8_t rs1 = (inst >> 3) & 0x7;
        int16_t offset = (offset7 & 0x40) ? (offset7 | 0xFF80) : offset7;
        snprintf(buf, bufSize, "jalr %s, %s, %d", regNames[rd], regNames[rs1], offset);
        break;
    }
    case 0x6: { // U-type (lui)
        uint16_t imm12 = (inst >> 4) & 0xFFF;
        uint8_t rd = (inst >> 6) & 0x7;
        snprintf(buf, bufSize, "lui %s, %d", regNames[rd], imm12 << 4);
        break;
    }
    case 0x7: { // System (ecall)
        uint8_t funct3 = (inst >> 3) & 0x7;
        snprintf(buf, bufSize, "ecall %d", funct3);
        break;
    }
    default:
        snprintf(buf, bufSize, "Unknown opcode");
        break;
    }
}

// -----------------------
// Instruction Execution
// -----------------------
int executeInstruction(uint16_t inst) {
    uint8_t opcode = inst & 0x7;
    int pcUpdated = 0;

    switch (opcode) {
    case 0x0: { // R-type
        uint8_t funct4 = (inst >> 12) & 0xF;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t rd_rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;

        if (funct4 == 0x0 && funct3 == 0x0) // add
            regs[rd_rs1] = regs[rd_rs1] + regs[rs2];
        else if (funct4 == 0x1 && funct3 == 0x0) // sub
            regs[rd_rs1] = regs[rd_rs1] - regs[rs2];
        else if (funct4 == 0x2 && funct3 == 0x0) // and
            regs[rd_rs1] = regs[rd_rs1] & regs[rs2];
        else if (funct4 == 0x3 && funct3 == 0x0) // or
            regs[rd_rs1] = regs[rd_rs1] | regs[rs2];
        else if (funct4 == 0x4 && funct3 == 0x0) // xor
            regs[rd_rs1] = regs[rd_rs1] ^ regs[rs2];
        else if (funct4 == 0x5 && funct3 == 0x0) // sll
            regs[rd_rs1] = regs[rd_rs1] << regs[rs2];
        else if (funct4 == 0x6 && funct3 == 0x0) // srl
            regs[rd_rs1] = (uint16_t)(regs[rd_rs1] >> regs[rs2]);
        else if (funct4 == 0x7 && funct3 == 0x0) // sra
            regs[rd_rs1] = (int16_t)regs[rd_rs1] >> regs[rs2];
        else if (funct4 == 0x8 && funct3 == 0x0) // slt
            regs[rd_rs1] = ((int16_t)regs[rd_rs1] < (int16_t)regs[rs2]) ? 1 : 0;
        else if (funct4 == 0x9 && funct3 == 0x0) // sltu
            regs[rd_rs1] = (regs[rd_rs1] < regs[rs2]) ? 1 : 0;
        break;
    }
    case 0x1: { // I-type
        uint8_t imm7 = (inst >> 9) & 0x7F;
        uint8_t rd_rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;
        int16_t simm = (imm7 & 0x40) ? (imm7 | 0xFF80) : imm7;

        switch (funct3) {
        case 0x0: regs[rd_rs1] += simm; break;  // addi
        case 0x1: regs[rd_rs1] &= imm7; break;  // andi
        case 0x2: regs[rd_rs1] |= imm7; break;  // ori
        case 0x3: regs[rd_rs1] ^= imm7; break;  // xori
        case 0x4: regs[rd_rs1] <<= (imm7 & 0xF); break;  // slli
        case 0x5: regs[rd_rs1] = (uint16_t)(regs[rd_rs1] >> (imm7 & 0xF)); break;  // srli
        case 0x6: regs[rd_rs1] = (int16_t)regs[rd_rs1] >> (imm7 & 0xF); break;  // srai
        case 0x7: regs[rd_rs1] = ((int16_t)regs[rd_rs1] < simm) ? 1 : 0; break;  // slti
        }
        break;
    }
    case 0x2: { // B-type
        uint8_t offset4 = (inst >> 12) & 0xF;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;
        int16_t offset = (offset4 << 1) | ((inst >> 8) & 0x1);
        offset = (offset << 7) >> 7; // sign extend

        int takeBranch = 0;
        switch (funct3) {
        case 0x0: takeBranch = (regs[rs1] == regs[rs2]); break;  // beq
        case 0x1: takeBranch = (regs[rs1] != regs[rs2]); break;  // bne
        case 0x2: takeBranch = ((int16_t)regs[rs1] < (int16_t)regs[rs2]); break;  // blt
        case 0x3: takeBranch = ((int16_t)regs[rs1] >= (int16_t)regs[rs2]); break;  // bge
        case 0x4: takeBranch = (regs[rs1] < regs[rs2]); break;  // bltu
        case 0x5: takeBranch = (regs[rs1] >= regs[rs2]); break;  // bgeu
        }

        if (takeBranch) {
            pc += offset * 2;
            pcUpdated = 1;
        }
        break;
    }
    case 0x3: { // L-type (load/store)
        uint8_t offset4 = (inst >> 12) & 0xF;
        uint8_t rs2 = (inst >> 9) & 0x7;
        uint8_t rs1 = (inst >> 6) & 0x7;
        uint8_t funct3 = (inst >> 3) & 0x7;
        int16_t offset = (offset4 << 1) | ((inst >> 8) & 0x1);
        offset = (offset << 7) >> 7; // sign extend
        uint16_t addr = regs[rs1] + offset;

        if (funct3 == 0x0) // sb
            memory[addr] = regs[rs2] & 0xFF;
        else if (funct3 == 0x1) // sh
            *(uint16_t*)&memory[addr] = regs[rs2];
        else if (funct3 == 0x2) // lb
            regs[rs2] = (int8_t)memory[addr];
        else if (funct3 == 0x3) // lh
            regs[rs2] = (int16_t) * (uint16_t*)&memory[addr];
        else if (funct3 == 0x4) // lbu
            regs[rs2] = memory[addr];
        else if (funct3 == 0x5) // lhu
            regs[rs2] = *(uint16_t*)&memory[addr];
        break;
    }
    case 0x4: { // J-type (jal)
        uint16_t offset = (inst >> 3) & 0x1FFF;
        offset = (offset << 3) >> 3; // sign extend
        uint8_t rd = (inst >> 6) & 0x7;

        regs[rd] = pc + 2;
        pc += offset * 2;
        pcUpdated = 1;
        break;
    }
    case 0x5: { // J-type (jalr)
        uint8_t offset7 = (inst >> 9) & 0x7F;
        uint8_t rd = (inst >> 6) & 0x7;
        uint8_t rs1 = (inst >> 3) & 0x7;
        int16_t offset = (offset7 & 0x40) ? (offset7 | 0xFF80) : offset7;

        uint16_t temp = pc + 2;
        pc = (regs[rs1] + offset) & ~1;
        regs[rd] = temp;
        pcUpdated = 1;
        break;
    }
    case 0x6: { // U-type (lui)
        uint16_t imm12 = (inst >> 4) & 0xFFF;
        uint8_t rd = (inst >> 6) & 0x7;
        regs[rd] = imm12 << 4;
        break;
    }
    case 0x7: { // System (ecall)
        uint8_t funct3 = (inst >> 3) & 0x7;

        switch (funct3) {
        case 1: // Print integer
            printf("%d", (int16_t)regs[6]); // a0 is x6
            break;
        case 5: { // Print string
            uint16_t addr = regs[6]; // a0 is x6
            while (memory[addr]) {
                putchar(memory[addr++]);
            }
            break;
        }
        case 3: // Terminate
            return 0;
        default:
            printf("Unknown ecall %d\n", funct3);
            break;
        }
        break;
    }
    default:
        printf("Unknown instruction opcode 0x%X\n", opcode);
        break;
    }

    if (!pcUpdated)
        pc += 2;
    return 1;
}

// -----------------------
// Memory Loading
// -----------------------
void loadMemoryFromFile(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("Error opening binary file");
        exit(1);
    }
    size_t n = fread(memory, 1, MEM_SIZE, fp);
    fclose(fp);
    printf("Loaded %zu bytes into memory\n", n);
}

// -----------------------
// Main Simulation Loop
// -----------------------
int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <machine_code_file>\n", argv[0]);
        exit(1);
    }
    loadMemoryFromFile(argv[1]);
    memset(regs, 0, sizeof(regs));
    pc = 0;
    char disasmBuf[128];

    while (pc < MEM_SIZE) {
        uint16_t inst = memory[pc] | (memory[pc + 1] << 8);
        disassemble(inst, pc, disasmBuf, sizeof(disasmBuf));
        printf("0x%04X: %04X    %s\n", pc, inst, disasmBuf);
        if (!executeInstruction(inst))
            break;
        if (pc >= MEM_SIZE) break;
    }
    return 0;
}