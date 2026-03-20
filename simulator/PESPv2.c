// instruction size 32 bits
#define REGISTER_COUNT 32
#define instruct_mem_size 1024
#define data_mem_size 1024
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct{
    uint32_t IR;
    uint32_t PC;
    uint8_t  valid;
}IF_ID;
typedef struct{
    uint8_t opcode;
    uint32_t A;
    uint32_t B;
    uint8_t rd;
    uint16_t immediate;
    uint8_t valid;
    uint32_t rs1;
    uint32_t rs2;
    
}ID_EX;
typedef struct{
    uint8_t opcode;
    uint32_t ALUout;
    uint32_t B;
    uint8_t rd;
    uint8_t valid;
    int reg_write; // Flag to indicate if this instruction writes to a register

} EX_MEM;
typedef struct{
    uint8_t opcode;
    uint32_t ALUout;
    uint8_t rd;
    uint32_t MDR;
    uint8_t valid;
    int reg_write; // Flag to indicate if this instruction writes to a register

} MEM_WB;

typedef struct {
    uint32_t registers[REGISTER_COUNT];
    uint32_t instruct_mem[instruct_mem_size];
    uint8_t data_mem[data_mem_size];
    uint8_t flag;
    IF_ID if_id;
    ID_EX id_ex;
    EX_MEM ex_mem;
    MEM_WB mem_wb;
    unsigned short HALT,PC;
    unsigned int machine_cycle, clock_cycle;
    uint32_t A,B,ALUout,rd,MAR,MDR,IR;
    int stall;
    int flush;
} PESP;
void execute(PESP *pesp,EX_MEM *new_ex_mem);
void memory_access(PESP *pesp, MEM_WB *new_mem_wb);
void write_back(PESP *pesp);
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
    pesp->if_id = (IF_ID){0};
    pesp->flag = 1;
    pesp->id_ex = (ID_EX){0};
    pesp->ex_mem = (EX_MEM){0};
    pesp->mem_wb = (MEM_WB){0};
    pesp->HALT = 0;
    pesp->clock_cycle = 0;
    pesp->PC = 0;
    pesp->machine_cycle = 0;
    pesp->stall = 0;
    pesp->flush = 0;
  
}
void ALU(EX_MEM *new_ex_mem, uint8_t opcode, uint32_t A,uint32_t B) {
    switch (opcode) {
        case 0: // ADD
            new_ex_mem->ALUout = A + B;
            break;
        case 1: // SUB
            new_ex_mem->ALUout = A - B;
            break;
        case 2: // AND
            new_ex_mem->ALUout = A & B;
            break;
        case 3: // OR
            new_ex_mem->ALUout = A | B;
            break;
        case 6: // MPY
            new_ex_mem->ALUout = A * B;
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

 void get_operands(uint32_t instruction, ID_EX *new_id_ex, PESP *pesp) {
    new_id_ex->rd = (instruction >> 23) & 0x1F; // Get bits 25-21
    new_id_ex->A = pesp->registers[(instruction >> 18) & 0x1F]; // Get bits 20-16
    new_id_ex->B = pesp->registers[(instruction >> 13) & 0x1F]; // Get bits 15-11
    if(new_id_ex->opcode == 0 || new_id_ex->opcode == 1 || new_id_ex->opcode == 2 || new_id_ex->opcode == 3 ||
         new_id_ex->opcode == 6 || new_id_ex->opcode == 0xB || new_id_ex->opcode == 0xC) { // R-type and branch instructions
        new_id_ex->rs1 = (instruction >> 18) & 0x1F; // Get bits 20-16
        new_id_ex->rs2 = (instruction >> 13) & 0x1F; // Get bits 15-11
    }
    else if(new_id_ex->opcode == 4 || new_id_ex->opcode == 5 || new_id_ex->opcode == 7 || 
            new_id_ex->opcode == 9  || new_id_ex->opcode == 0xE) { // I-type instructions
        new_id_ex->rs1 = (instruction >> 18) & 0x1F; // Get bits 20-16
        new_id_ex->rs2 = -1; // No second source register
    }
    else{
        new_id_ex->rs1 = -1; // No source registers
        new_id_ex->rs2 = -1;
    }
}   

void get_immediate(uint32_t instruction, uint16_t *immediate) {
    *immediate = instruction & 0x1FFF; // Get the bottom 13 bits
}

void fetch (PESP *pesp, IF_ID *new_if_id) {
    
    if(pesp->if_id.valid && pesp->id_ex.valid && pesp->id_ex.opcode == 0x7 &&
         (pesp->id_ex.rd == ((pesp->if_id.IR >> 18) & 0x1F) || pesp->id_ex.rd == ((pesp->if_id.IR >> 13) & 0x1F))) { // Check for load-use hazard
        printf("STALL: load-use hazard on R%d\n", pesp->id_ex.rd);
        pesp->stall = 1; // Set stall flag to indicate a stall is in progress
        return;

    }
    if(pesp->flag) {
        new_if_id->IR = pesp->instruct_mem[pesp->PC]; // Fetch instruction into IR
        new_if_id->PC = pesp->PC;
        new_if_id->valid = 1;
        pesp->PC += 1; // Increment PC to point to next instruction
    }
}

void decode (PESP *pesp, ID_EX *new_id_ex) {
    if (pesp->if_id.valid) {
    new_id_ex->valid = 1;
    get_opcode(pesp->if_id.IR, &new_id_ex->opcode);
    get_operands(pesp->if_id.IR, new_id_ex, pesp);
    if(new_id_ex->opcode == 15) { // HALT
        pesp->flag = 0; // Set HALT flag to stop fetching new instructions
    }
    if (new_id_ex->opcode == 9 || new_id_ex->opcode == 0xA) { // ST or STI
         new_id_ex->B= pesp->registers[new_id_ex->rd]; // Get value to be stored in memory
    }
    get_immediate(pesp->if_id.IR, &new_id_ex->immediate);
    printf("Decoded instruction: opcode=%u, rd=R%u, rs1=R%u, rs2=R%u, immediate=%u\n", new_id_ex->opcode, new_id_ex->rd, (pesp->if_id.IR >> 18) & 0x1F, (pesp->if_id.IR >> 13) & 0x1F, new_id_ex->immediate);
    }
}
 void execute(PESP *pesp,EX_MEM *new_ex_mem) {
    if(pesp->id_ex.valid) {
    new_ex_mem->opcode = pesp->id_ex.opcode;
    new_ex_mem->rd = pesp->id_ex.rd;
    new_ex_mem->B = pesp->id_ex.B;
    new_ex_mem->valid = 1;
    new_ex_mem->reg_write= 0;
    uint32_t A = pesp->id_ex.A;
    uint32_t B = pesp->id_ex.B;
    if(pesp->mem_wb.valid && pesp->mem_wb.reg_write){
        if(pesp->mem_wb.opcode == 7) { // LD
            if(pesp->mem_wb.rd == pesp->id_ex.rs1){
                A = pesp->mem_wb.MDR; // Forward loaded data to A
            }
            if(pesp->mem_wb.rd == pesp->id_ex.rs2){
                B = pesp->mem_wb.MDR; // Forward loaded data to B
            }
        } else {
            if(pesp->mem_wb.rd == pesp->id_ex.rs1){
                A = pesp->mem_wb.ALUout; // Forward ALU result to A
            }
            if(pesp->mem_wb.rd == pesp->id_ex.rs2){
                B = pesp->mem_wb.ALUout; // Forward ALU result to B
            }
        }

    }
    if(pesp->ex_mem.valid && pesp->ex_mem.reg_write){
        if(pesp->ex_mem.rd == pesp->id_ex.rs1){
            A = pesp->ex_mem.ALUout; // Forward ALU result to A
        }
        if(pesp->ex_mem.rd == pesp->id_ex.rs2){
            B = pesp->ex_mem.ALUout; // Forward ALU result to B
        }

    }
    switch (pesp->id_ex.opcode) {
        case 0: // ADD
                ALU(new_ex_mem, 0, A, B);
                new_ex_mem->reg_write = 1; // This instruction writes to a register  
            break;
        case 1: // SUB
                ALU(new_ex_mem, 1, A, B);
                new_ex_mem->reg_write = 1; // This instruction writes to a register
                
                
            break;
        case 2: // AND
                ALU(new_ex_mem, 2, A, B);
                new_ex_mem->reg_write = 1; // This instruction writes to a register
            break;
        case 3: // OR
            ALU(new_ex_mem, 3, A, B);
            new_ex_mem->reg_write = 1; // This instruction writes to a register
            break;
        case 4: //ADDI
            ALU(new_ex_mem, 0, A, pesp->id_ex.immediate);
            new_ex_mem->reg_write = 1; // This instruction writes to a register
            break;
        case 5: //SUBI
        ALU(new_ex_mem, 1, A, pesp->id_ex.immediate);
        new_ex_mem->reg_write = 1; // This instruction writes to a register
        
            
            break;
        case 6: //MPY
            ALU(new_ex_mem, 6, A, B);
            new_ex_mem->reg_write = 1; // This instruction writes to a register
            break;
        case 7: //LD
         {  
            uint32_t address = A + pesp->id_ex.immediate; // Calculate effective address
            if (address < data_mem_size) {
                ALU(new_ex_mem, 0, A, pesp->id_ex.immediate); 
            } else {
                printf("Memory access out of bounds: %u\n", address);
            } 
            new_ex_mem->reg_write = 1; // This instruction writes to a register
            break; 
         }
        case 8: //LDI
            new_ex_mem->ALUout = pesp->id_ex.immediate;         
            new_ex_mem->reg_write = 1; // This instruction writes to a register
                    break;
        case 9: //ST
        {
            uint32_t address = A + pesp->id_ex.immediate; // Calculate effective address
            if (address < data_mem_size) {
                ALU(new_ex_mem, 0, A, pesp->id_ex.immediate); // Store data from register into memory
                new_ex_mem->B = B; // Store value to be written in memory
            } else {
                printf("Memory access out of bounds: %u\n", address);
            } 
            
            break;
        }
        case 10: //STI
            new_ex_mem->ALUout = pesp->id_ex.immediate; 
            
            break;
        case 11: //BEQ
        printf("BEQ check: A=%u B=%u imm=%u rs1=%u rs2=%u rd=%u\n",
       A, B, pesp->id_ex.immediate,
       pesp->id_ex.rs1, pesp->id_ex.rs2, pesp->id_ex.rd);
            if (A == B) {
                pesp->PC = pesp->id_ex.immediate; // Branch to target address
                pesp->flush = 1; // Set flush flag to indicate a flush is in progress
                printf("Flush on BEQ to address %u\n", pesp->PC);
            
            }
            break;
        case 12: //BNE
            if (A != B) {
                pesp->PC = pesp->id_ex.immediate; // Branch to target address
                pesp->flush = 1; // Set flush flag to indicate a flush is in progress
                
            }
            break;
        case 13: //JUMP
            pesp->PC = pesp->id_ex.immediate; // Jump to target address
            pesp->flush = 1; // Set flush flag to indicate a flush is in progress
            break;    
        case 14: //JCD
            if (A == 0) {
                pesp->PC = pesp->id_ex.immediate; // Jump to target address
                pesp->flush = 1; // Set flush flag to indicate a flush is in progress
            }
            break;  
        case 15: //HALT
             // HALT at wb
             
            break; 


    }
} 
}
    void memory_access(PESP *pesp, MEM_WB *new_mem_wb) {
        if(pesp->ex_mem.valid) {
            new_mem_wb->opcode = pesp->ex_mem.opcode;
            new_mem_wb->ALUout = pesp->ex_mem.ALUout;
            new_mem_wb->rd = pesp->ex_mem.rd;
            new_mem_wb->MDR = 0; // Default value for MDR
            new_mem_wb->valid = 1;
            new_mem_wb->reg_write = pesp->ex_mem.reg_write; // Pass reg_write flag to MEM/WB
            switch(pesp->ex_mem.opcode) {
                case 7: //LD
                        new_mem_wb->MDR= pesp->data_mem[pesp->ex_mem.ALUout]; // Load data from memory into register
                        
                    break;
                case 9: //ST
                    
                        pesp->data_mem[pesp->ex_mem.ALUout] = pesp->ex_mem.B; // Store data from register into memory
                        
                    break;
                case 10: //STI
                    
                        pesp->data_mem[pesp->ex_mem.B] = pesp->ex_mem.ALUout ; // Store immediate value into memory
                        
                    break;
            }
        }


        }
void write_back(PESP *pesp) {
 if(pesp->mem_wb.valid) {
    switch (pesp->mem_wb.opcode) {
        case 0: // ADD
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write ALU result back to register
           
            break;

        case 1: // SUB
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write ALU result back to register
            
            break;  
        case 2: // AND
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write ALU result back to register
            
            break;
        case 3: // OR
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write ALU result back to register
            
            break;
        case 4: //ADDI
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write ALU result back to register
              
            break;
        case 5: //SUBI
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write ALU result back to register
           
            break;
        case 6: //MPY
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write ALU result back to register
           
            break;
        case 7: //LD
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.MDR; // Write data from memory back to register
            
            break;
        case 8: //LDI
            pesp->registers[pesp->mem_wb.rd] = pesp->mem_wb.ALUout; // Write immediate value back to register
            
            break;
        case 15: //HALT
            pesp->HALT = 1; // Set HALT flag to stop CPU
            break;
    }
       pesp->machine_cycle += 1; // Increment machine cycle after each instruction
 }
}
void cpu_cycle(PESP *pesp) {
IF_ID next_if_id={0};
ID_EX next_id_ex={0};
EX_MEM next_ex_mem={0};
MEM_WB next_mem_wb={0};
write_back(pesp);
memory_access(pesp, &next_mem_wb);
execute(pesp,&next_ex_mem);
decode(pesp, &next_id_ex);
fetch(pesp, &next_if_id);
if(pesp->flush){
    next_id_ex = (ID_EX){0};   // squash wrong-path decoded instruction
    pesp->flush = 0;
}
else if(pesp->stall){
    next_if_id=pesp->if_id; // Invalidate the instruction in ID/EX to create a bubble
    next_id_ex=(ID_EX){0};
    pesp->stall = 0; // Clear stall flag for next cycle]
}
pesp->if_id = next_if_id;
pesp->id_ex = next_id_ex;
pesp->ex_mem = next_ex_mem;
pesp->mem_wb = next_mem_wb;
pesp->clock_cycle += 1; // Increment clock cycle

}


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

    while (!pesp.HALT && pesp.clock_cycle < 100) {
    cpu_cycle(&pesp);
}

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