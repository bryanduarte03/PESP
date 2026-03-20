// instruction size 32 bits
#define REGISTER_COUNT 32
#define instruct_mem_size 1024
#define data_mem_size 1024
#include <stdint.h>
#include <stdio.h>
typedef struct {
    uint32_t registers[REGISTER_COUNT];
    uint32_t instruct_mem[instruct_mem_size];
    uint8_t data_mem[data_mem_size];
    unsigned short HALT,PC;
    unsigned int machine_cycle, clock_cycle;
    uint32_t A,B,ALUout,rd,MAR,MDR,IR;
} PESP;

void initialize_pesp(PESP *pesp) {
    for (int i = 0; i < REGISTER_COUNT; i++) {
        pesp->registers[i] = 0;
    }
    for (int i = 0; i < instruct_mem_size; i++) {
        pesp->instruct_mem[i] = 0;
    }
    for (int i = 0; i < data_mem_size; i++) {
        pesp->data_mem[i] = 0;
    }
    pesp->HALT = 0;
    pesp->clock_cycle = 0;
    pesp->PC = 0;
    pesp->machine_cycle = 0;
  
}
void ALU(PESP *pesp, uint8_t opcode, uint32_t A,uint32_t B) {
    switch (opcode) {
        case 0: // ADD
            pesp->ALUout = A + B;
            break;
        case 1: // SUB
            pesp->ALUout = A - B;
            break;
        case 2: // AND
            pesp->ALUout = A & B;
            break;
        case 3: // OR
            pesp->ALUout = A | B;
            break;
        case 6: // MPY
            pesp->ALUout = A * B;
            break;        
    }
}
void print_registers(PESP *pesp) {
    for (int i = 0; i < REGISTER_COUNT; i++) {
        printf("R%d: %u\n", i, pesp->registers[i]);
    }
}

void get_opcode(uint32_t instruction, uint8_t *opcode) {
    *opcode = (instruction >> 28) & 0xF; // Get the top  4 bits
}   

 void get_operands(uint32_t instruction, PESP *pesp) {
    pesp->rd = (instruction >> 23) & 0x1F; // Get bits 25-21
    pesp->A = pesp->registers[(instruction >> 18) & 0x1F]; // Get bits 20-16
    pesp->B = pesp->registers[(instruction >> 13) & 0x1F]; // Get bits 15-11
}   

void get_immediate(uint32_t instruction, uint16_t *immediate) {
    *immediate = instruction & 0x1FFF; // Get the bottom 13 bits
}

void fetch (PESP *pesp) {
    pesp->MAR = pesp->PC; // Set MAR to the current PC
    pesp->PC=pesp->PC+1;
    pesp->clock_cycle += 1; // Increment clock cycle
    pesp->MDR = pesp->instruct_mem[pesp->MAR]; // Fetch instruction into MDR
    pesp->clock_cycle += 1; 
    pesp->IR = pesp->MDR; // Load instruction into IR
    pesp->clock_cycle += 1; 
}

void decode (PESP *pesp, uint8_t *opcode, uint16_t *immediate) {
    get_opcode(pesp->IR, opcode);
    get_operands(pesp->IR,pesp);
    get_immediate(pesp->IR, immediate);
    printf("Decoded instruction: opcode=%u, rd=R%u, rs1=R%u, rs2=R%u, immediate=%u\n", *opcode, pesp->rd, (pesp->IR >> 18) & 0x1F, (pesp->IR >> 13) & 0x1F, *immediate);
}
 void execute(PESP *pesp, uint8_t opcode, uint16_t immediate) {
    switch (opcode) {
        case 0: // ADD
                ALU(pesp, 0, pesp->A, pesp->B);
                  
                pesp->clock_cycle += 1; // Increment clock cycle
                 
            break;
        case 1: // SUB
                ALU(pesp, 1, pesp->A, pesp->B);
                
                pesp->clock_cycle += 1; // Increment clock cycle
                
            break;
        case 2: // AND
                ALU(pesp, 2, pesp->A, pesp->B);
                
                pesp->clock_cycle += 1; // Increment clock cycle
                 
            
            break;
        case 3: // OR
            ALU(pesp, 3, pesp->A, pesp->B);
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 4: //ADDI
            ALU(pesp, 0, pesp->A, immediate);
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 5: //SUBI
        ALU(pesp, 1, pesp->A, immediate);
        pesp->clock_cycle += 1; // Increment clock cycle
            
            break;
        case 6: //MPY
            ALU(pesp, 6, pesp->A, pesp->B);
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 7: //LD
         {  
            uint32_t address = pesp->A + immediate; // Calculate effective address
            if (address < data_mem_size) {
                ALU(pesp, 0, pesp->A, immediate); 
            } else {
                printf("Memory access out of bounds: %u\n", address);
            } 
            pesp->clock_cycle += 1; // Increment clock cycle
            break; 
         }
        case 8: //LDI
            pesp-> ALUout = immediate;         
            pesp->clock_cycle += 1; // Increment clock cycle
                    break;
        case 9: //ST
        {
            uint32_t address = pesp->A + immediate; // Calculate effective address
            if (address < data_mem_size) {
                ALU(pesp, 0, pesp->A, immediate); // Store data from register into memory
            } else {
                printf("Memory access out of bounds: %u\n", address);
            } 
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        }
        case 10: //STI
            pesp->ALUout = immediate; 
            pesp->clock_cycle += 1; 
            break;
        case 11: //BEQ
            if (pesp->A == pesp->B) {
                pesp->PC = immediate; // Branch to target address
                pesp->clock_cycle += 1; // Increment clock cycle
            
            }
            break;
        case 12: //BNE
            if (pesp->A != pesp->B) {
                pesp->PC = immediate; // Branch to target address
                pesp->clock_cycle += 1; // Increment clock cycle
            }
            break;
        case 13: //JUMP
            pesp->PC = immediate; // Jump to target address
            pesp->clock_cycle += 1; // Increment clock cycle
            break;    
        case 14: //JCD
            if (pesp->A == 0) {
                pesp->PC = immediate; // Jump to target address
                pesp->clock_cycle += 1; // Increment clock cycle
            }
            break;  
        case 15: //HALT
            pesp->HALT = 1; // Set HALT flag to stop execution
            pesp->clock_cycle += 1; // Increment clock cycle
            break; 

    }
}
    void memory_access(PESP *pesp, uint8_t opcode,uint32_t ALUout) {
         
            switch(opcode) {
                case 7: //LD
                        pesp->MDR= pesp->data_mem[ALUout ]; // Load data from memory into register
                        pesp->clock_cycle += 1; // Increment clock cycle
                    break;
                case 9: //ST
                    
                        pesp->data_mem[ALUout] = pesp->registers[pesp->rd]; // Store data from register into memory
                        pesp->clock_cycle += 1; // Increment clock cycle
                    break;
                case 10: //STI
                    
                        pesp->data_mem[pesp->registers[pesp->rd]] = ALUout ; // Store immediate value into memory
                        pesp->clock_cycle += 1; // Increment clock cycle
                    break;
            }
        


        }
void write_back(PESP *pesp, uint8_t opcode) {
    switch (opcode) {
        case 0: // ADD
            pesp->registers[pesp->rd] = pesp->ALUout; // Write ALU result back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;

        case 1: // SUB
            pesp->registers[pesp->rd] = pesp->ALUout; // Write ALU result back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;  
        case 2: // AND
            pesp->registers[pesp->rd] = pesp->ALUout; // Write ALU result back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 3: // OR
            pesp->registers[pesp->rd] = pesp->ALUout; // Write ALU result back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 4: //ADDI
            pesp->registers[pesp->rd] = pesp->ALUout; // Write ALU result back to register
            pesp->clock_cycle += 1; // Increment clock cycle    
            break;
        case 5: //SUBI
            pesp->registers[pesp->rd] = pesp->ALUout; // Write ALU result back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 6: //MPY
            pesp->registers[pesp->rd] = pesp->ALUout; // Write ALU result back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 7: //LD
            pesp->registers[pesp->rd] = pesp->MDR; // Write data from memory back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
        case 8: //LDI
            pesp->registers[pesp->rd] = pesp->ALUout; // Write immediate value back to register
            pesp->clock_cycle += 1; // Increment clock cycle
            break;
    }
   
}

void CPU_start(PESP *pesp){  
while (!pesp->HALT && pesp->machine_cycle < 100) { // Limit to 100 machine cycles to prevent infinite loops
    uint8_t opcode;
    uint16_t immediate;
    fetch(pesp);
    decode(pesp,&opcode,&immediate);
    execute(pesp, opcode,immediate);
    memory_access(pesp, opcode,pesp->ALUout);
    write_back(pesp, opcode);
    pesp->machine_cycle += 1; // Increment machine cycle after each instruction


}
}
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* assumes these already exist somewhere in your file:
   - typedef struct PESP ...
   - initialize_pesp(PESP *pesp);
   - load_instruction(PESP *pesp, uint32_t instruction, int index);
   - CPU_start(PESP *pesp);
   - REGISTER_COUNT
   - instruct_mem_size
*/

int load_program(PESP *pesp, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("fopen");
        return 0;
    }

    uint32_t word;
    int index = 0;

    while (fread(&word, sizeof(uint32_t), 1, f) == 1) {
        if (index >= instruct_mem_size) {
            fprintf(stderr, "Program too large for instruction memory\n");
            fclose(f);
            return 0;
        }
        pesp->instruct_mem[index++] = word;
    }

    fclose(f);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <program.bin>\n", argv[0]);
        return 1;
    }

    PESP pesp;
    initialize_pesp(&pesp);

    if (!load_program(&pesp, argv[1])) {
        fprintf(stderr, "Failed to load program: %s\n", argv[1]);
        return 1;
    }

    CPU_start(&pesp);

    printf("\nFinal CPU State\n");
    printf("----------------\n");

    for (int i = 0; i < REGISTER_COUNT; i++) {
        printf("R%d = %u\n", i, pesp.registers[i]);
    }

    printf("\nMachine cycles: %u\n", pesp.machine_cycle);
    printf("Clock cycles: %u\n", pesp.clock_cycle);
    printf("PC: %u\n", pesp.PC);

    return 0;
}