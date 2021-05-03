#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "SimpleRV32I.h"
#include "SimpleRV32I_utils.h"

/*                                      RV32I instruction format
 *('S' - Sign Bit, 'X' - normal bit, '-->' - right fill, '<--' - left fill) : for getting 32bit immediates
 *===========================================================================================================
 *
 * |`````````````````{31:12,0-->}```````````|```11:7````|`````6:0``````|
 * |S x x x x x x x x x x x x x x x x x x x | x x x x x | x x x x x x x|   U-Type Instruction
 * |___________________IMM__________________|_____RD____|____OPCODE____|
 *
 *===========================================================================================================
 *
 * |``````{<--S,20,10:1,11,19:12,0-->}``````|```11:7````|`````6:0``````|
 * |S x x x x x x x x x x x x x x x x x x x | x x x x x | x x x x x x x|   J-Type Instruction
 * |___________________IMM__________________|_____RD____|____OPCODE____|
 *
 *===========================================================================================================
 *
 * |`````31:25````|```24:20```|```19:15```|`14:12`|```11:7````|`````6:0``````|
 * |x x x x x x x | x x x x x | x x x x x | x x x | x x x x x | x x x x x x x|   R-Type Instruction
 * |____FUNCT7____|____RS2____|_____RS1___|_FUNCT3|_____RD____|____OPCODE____|
 *
 *===========================================================================================================
 *
 * |`````31:25````|```24:20```|```19:15```|`14:12`|```11:7````|`````6:0``````|
 * |x x x x x x x | x x x x x | x x x x x | x x x | x x x x x | x x x x x x x|   I-Type Instruction
 * |____FUNCT7____|___SHAMT___|_____RS1___|_FUNCT3|_____RD____|____OPCODE____|
 *
 *===========================================================================================================
 *
 * |````````{S,11:0}````````|```19:15```|`14:12`|```11:7````|`````6:0``````|
 * |S x x x x x x x x x x x | x x x x x | x x x | x x x x x | x x x x x x x|   I-Type Instruction
 * |__________IMM___________|_____RS1___|_FUNCT3|_____RD____|____OPCODE____|
 *
 *===========================================================================================================
 *
 * |`````31:25````|```24:20```|```19:15```|`14:12`|```11:7````|`````6:0``````|
 * |x x x x x x x | x x x x x | x x x x x | x x x | x x x x x | x x x x x x x|   S-Type Instruction
 * |__IMM[11:5]___|____RS2____|_____RS1___|_FUNCT3|_IMM[4:0]__|____OPCODE____|
 *
 *===========================================================================================================
 *
 * |`````31:25````|```24:20```|```19:15```|`14:12`|```11:7````|`````6:0``````|
 * |x x x x x x x | x x x x x | x x x x x | x x x | x x x x x | x x x x x x x|   B-Type Instruction
 * |__IMM[12,10:5]|____RS2____|_____RS1___|_FUNCT3|IMM[4:1,11]|____OPCODE____|
 *
 *===========================================================================================================
 *
 */

RV32I_INST::RV32I_INST() {
    opcode = 0;
    rd = 0;
    rs1 = 0;
    rs2 = 0;
    funct3 = 0;
    funct7 = 0;
    shamt = 0;
    imm = 0;
}


/*
 * takes a 32bit rv32i instruction as input
 * and decodes various fields within the instruction
 * */
void RV32I_INST::decodeInst(uint32_t inst) {
    this->inst = inst;
    opcode  = inst & 0x0000007f;
    rd      = (inst >> 7)  & 0x00000001f;
    rs1     = (inst >> 15) & 0x00000001f;
    rs2     = (inst >> 20) & 0x00000001f;
    funct3  = (inst >> 12) & 0x000000007;
    funct7  = (inst >> 25) & 0x00000007f;
    shamt   = (inst >> 20) & 0x00000001f;

    //extract immediate value
    switch(getType()) {
        case U_TYPE:    
                        imm = inst & 0xfffff000;
                        break;
        case B_TYPE:    
                        imm |= ((inst >> 7) & 0x0000001e);
                        imm |= ((inst >> 20) & 0x00007e0);
                        imm |= ((inst << 4) & 0x0000800);
                        imm |= ((((int32_t)inst) >> 20) & 0xfffff000);
                        break;
        case I_TYPE:
                        if((opcode == 0b0010011) && ((funct3 == 0x01) || (funct3 == 0x05))) {
                            imm |= ((inst >> 20) & 0x0000001f);
                            imm |= ((((int32_t)inst) >> 20) & 0xffffffe0);
                        }else{
                            imm |= ((inst >> 20) & 0x00000fff);
                            imm |= ((((int32_t)inst) >> 20) & 0xfffff000);
                        }
                        break;
        case S_TYPE: 
                        imm |= ((inst >> 7) & 0x0000001f);
                        imm |= ((((int32_t)inst) >> 20) & 0xffffffe0);
                        break;
        case J_TYPE: 
                        imm |= (inst & 0x000ff000);
                        imm |= ((inst >> 9) &  0x00000800);
                        imm |= ((inst >> 20) &  0x000007fe);
                        break;
        case INVALID_TYPE: 
                        std::cerr << "Invalid rv32I instruction/opcode: " << std::hex << inst << "/" << opcode << std::endl;
                        exit(1);
                        break;
    }
}


/*
 *returns the type of the instruction: I/J/R/S/B/U
 * */
rv32i_op_type RV32I_INST::getType() {
    rv32i_op_type type;
    switch(opcode) {
        case 0b0110111:
        case 0b0010111:     
                            type = U_TYPE;
                            break;
        case 0b1100011:     
                            type = B_TYPE;
                            break;
        case 0b0000011:
        case 0b0010011:     
        case 0b1110011:
        case 0b1100111:     
                            type = I_TYPE;
                            break;
        case 0b0100011:     
                            type = S_TYPE;
                            break;
        case 0b0110011:     
                            type = R_TYPE;
                            break;
        case 0b1101111:     
                            type = J_TYPE;
                            break;
        default:            
                            type = INVALID_TYPE;
                            break;
    }
    return type;
}



/*
 * Returns the exact mnemonic operation to be performed
 */
rv32i_operation   RV32I_INST::getOperation() {
    rv32i_operation op;
    switch(getType()) {
        case U_TYPE:    
                        if(opcode == 0b0110111) op = LUI;
                        if(opcode == 0b0010111) op = AUIPC;
                        break;
        case B_TYPE:    
                        if(funct3 == 0b000) op = BEQ;
                        if(funct3 == 0b001) op = BNE;
                        if(funct3 == 0b100) op = BLT;
                        if(funct3 == 0b101) op = BGE;
                        if(funct3 == 0b110) op = BLTU;
                        if(funct3 == 0b111) op = BGEU;
                        break;
        case I_TYPE:
                        switch(opcode) {
                            case 0b0000011:
                                            if(funct3 == 0b000) op = LB;
                                            if(funct3 == 0b001) op = LH;
                                            if(funct3 == 0b010) op = LW;
                                            if(funct3 == 0b100) op = LBU;
                                            if(funct3 == 0b101) op = LHU;
                                            break;
                            case 0b0010011:
                                            if(funct3 == 0b000) op = ADDI;
                                            if(funct3 == 0b010) op = SLTI;
                                            if(funct3 == 0b011) op = SLTIU;
                                            if(funct3 == 0b100) op = XORI;
                                            if(funct3 == 0b110) op = ORI;
                                            if(funct3 == 0b111) op = ANDI;
                                            if(funct3 == 0b001) op = SLLI;
                                            if(funct3 == 0b101) {
                                                if(funct7 == 0b0000000) op = SRLI;
                                                if(funct7 == 0b0100000) op = SRAI;
                                            }
                                            break;
                            case 0b1100111: op = JALR;
                                            break;
                            case 0b1110011: //TODO: understand these category of instructions
                                            //CSRRW
                                            //CSRRS
                                            //CSRRC
                                            //CSRRWI
                                            //CSRRSI
                                            //CSRRCI
                                            if(funct3 == 0) {
                                                if(imm == 0) op = ECALL;
                                                if(imm == 1) op = EBREAK;
                                            }
                                            break;
                        }
                        break;
        case S_TYPE: 
                        if(funct3 == 0b000) op = SB;
                        if(funct3 == 0b001) op = SH;
                        if(funct3 == 0b010) op = SW;
                        break;
        case J_TYPE:    
                        op = JAL; 
                        break;
        case R_TYPE:
                        if(funct3 == 0b000) {
                            if(funct7 == 0b0000000) op = ADD;
                            if(funct7 == 0b0100000) op = SUB;
                        }
                        if(funct3 == 0b001) op = SLL;
                        if(funct3 == 0b010) op = SLT;
                        if(funct3 == 0b011) op = SLTU;
                        if(funct3 == 0b100) op = XOR;
                        if(funct3 == 0b101) {
                            if(funct7 == 0b0000000) op = SRL;
                            if(funct7 == 0b0100000) op = SRA;
                        }
                        if(funct3 == 0b110) op = OR;
                        if(funct3 == 0b111) op = AND;
                        break;
    
    }
    debug_printf(DEBUG_MEDIUM,"RV32I_INST::getOperation> inst(%08x) => op(%d) %s:%d\n",inst,op,__FILE__, __LINE__);
    
    return op;
}


SimpleRV32I::SimpleRV32I(int memSize) {
    inst_mem = NULL;
    data_mem = NULL;
    inst_mem = new uint8_t[memSize];
    mem_size = memSize;
    if(inst_mem == NULL) {
       std::cerr << "Unable to allocate instruction memory: " <<  memSize << " words" << std::endl;
       exit(1);
    }
    data_mem = new uint8_t[memSize];
    if(data_mem == NULL) {
       std::cerr << "Unable to allocate data memory: " <<  memSize << " words" << std::endl;
       exit(1);
    }

    //initialize memory/PC/status/registers to 0
    for(int i=0; i<memSize; i++) {
        inst_mem[i] = 0;
        data_mem[i] = 0;
    }

    PC = 0;
    status = 0;

    for(int i=0; i<32; i++) {
        regs[i] = 0;
    }
}

SimpleRV32I::~SimpleRV32I() {
    delete [] inst_mem;
    delete [] data_mem;
}


/*
 * loads a program binary from a ascii hexadecimal format file (.txt file)
 * each line in the input file will correspond to a 32bit value
 * */
void SimpleRV32I::loadProgram(std::string file) {
    std::ifstream inFile;
    inFile.open(file);
    std::string line;
    int i = 0;

    if(!inFile.is_open()) {
        std::cout << "Unable to open file: " << file << std::endl;
    }

    while(std::getline(inFile, line)) {
        std::stringstream s;
        uint32_t data;
        s << std::hex << line;
        s >> data;
        if(i == mem_size) {
            std::cerr << "Insufficient memory" << std::endl;
            exit(1);
        }
        *((uint32_t*)(inst_mem + i)) = data;
        i += 4;
    }
    inFile.close();
}

/*
 * loads a data from a ascii hexadecimal format file (.txt file)
 * each line in the input file will correspond to a 32bit value
 * */

void SimpleRV32I::loadData(std::string file) {
    std::ifstream inFile;
    inFile.open(file);
    std::string line;
    int i = 0;

    if(!inFile.is_open()) {
        std::cout << "Unable to open file: " << file << std::endl;
    }

    while(std::getline(inFile, line)) {
        std::stringstream s;
        uint32_t data;
        s << std::hex << line;
        s >> data;
        if(i == mem_size) {
            std::cerr << "Insufficient memory" << std::endl;
            exit(1);
        }
        *((uint32_t*)(data_mem + i)) = data;
        i += 4;
    }
    inFile.close();
}

/*
 * dumps the data memory content to a text file, in hex formatted ascii 
 * each line in the input file will correspond to a 32bit value
 * */

void SimpleRV32I::dumpData(std::string file) {
    std::ofstream outFile;
    outFile.open(file);

    if(!outFile.is_open()) {
        std::cout << "Unable to open file: " << file << std::endl;
    }

    for(int i=0; i< mem_size; i+=4) {
        uint32_t data = *((uint32_t*)(data_mem + i));
        outFile << "0x" << std::hex << std::setw(8) << std::setfill('0') << data << std::endl;
    }

    outFile.close();

}

/*
 * dumps the registers content to a text file, in hex formatted ascii 
 * each line in the input file will correspond to a 32bit value
 * */

void SimpleRV32I::dumpRegs(std::string file) {
    std::ofstream outFile;
    outFile.open(file);

    if(!outFile.is_open()) {
        std::cout << "Unable to open file: " << file << std::endl;
    }

    for(int i=0; i< 32; i++) {
        uint32_t data = regs[i];
        outFile << "0x" << std::hex << std::setw(8) << std::setfill('0') << data << std::endl;
    }

    outFile.close();
}

/*
 * steps through the loaded program
 */
int SimpleRV32I::step() {
    if(!status) {
        inst = RV32I_INST();
        uint32_t instruction = *((uint32_t*)(inst_mem + PC)); //fetch
        inst.decodeInst(instruction); //decode
        debug_printf(DEBUG_LOW,"SimpleRV32I::step> %08x %s:%d\n",instruction,__FILE__, __LINE__);
        switch(inst.getOperation()) { //execute
            case LUI:       regs[inst.rd] = inst.imm; PC = PC+4; break;
            case AUIPC:     regs[inst.rd] = PC + inst.imm; PC = PC+4; break;
            case JAL:       regs[inst.rd] = PC+4; PC = PC + inst.imm; break; 
            case JALR:      regs[inst.rd] = PC+4; PC = (((int32_t)regs[inst.rs1]) + inst.imm) & ~1; break;
            case BEQ:       PC = PC + ((regs[inst.rs1] == regs[inst.rs2]) ? inst.imm : 4); break; 
            case BNE:       PC = PC + ((regs[inst.rs1] != regs[inst.rs2]) ? inst.imm : 4); break;
            case BLT:       PC = PC + ((((int32_t)regs[inst.rs1]) < ((int32_t)regs[inst.rs2])) ? inst.imm : 4); break;
            case BGE:       PC = PC + ((((int32_t)regs[inst.rs1]) >= ((int32_t)regs[inst.rs2])) ? inst.imm : 4); break;
            case BLTU:      PC = PC + (((uint32_t)(regs[inst.rs1]) <  ((uint32_t)(regs[inst.rs2]))) ? inst.imm : 4); break;
            case BGEU:      PC = PC + (((uint32_t)(regs[inst.rs1]) >= ((uint32_t)(regs[inst.rs2]))) ? inst.imm : 4); break;
            case LB:        regs[inst.rd]=(int32_t)(*((int8_t*)(data_mem + regs[inst.rs1] + inst.imm))); PC = PC+4; break;
            case LH:        regs[inst.rd]=(int32_t)(*((int16_t*)(data_mem + regs[inst.rs1] + inst.imm))); PC = PC+4; break;
            case LW:        regs[inst.rd]=(int32_t)(*((int32_t*)(data_mem + regs[inst.rs1] + inst.imm))); PC = PC+4; break;
            case LBU:       regs[inst.rd]=(*((uint8_t*)(data_mem + regs[inst.rs1] + inst.imm))); PC = PC+4; break;
            case LHU:       regs[inst.rd]=(*((uint16_t*)(data_mem + regs[inst.rs1] + inst.imm))); PC = PC+4; break;
            case SB:        *((uint8_t*)(data_mem + regs[inst.rs1] + inst.imm)) = (uint8_t)(regs[inst.rs2] & 0x000000ff);  PC = PC+4; break;
            case SH:        *((uint16_t*)(data_mem + regs[inst.rs1] + inst.imm)) = (uint16_t)(regs[inst.rs2] & 0x0000ffff);  PC = PC+4; break;
            case SW:        *((uint32_t*)(data_mem + regs[inst.rs1] + inst.imm)) = (uint32_t)(regs[inst.rs2] & 0xffffffff);  PC = PC+4; break;
            case ADDI:      regs[inst.rd] = regs[inst.rs1] + inst.imm; PC = PC+4; break; 
            case SLTI:      regs[inst.rd] = ((int32_t)regs[inst.rs1] < ((int32_t)inst.imm)) ? 1 : 0; PC = PC+4; break;
            case SLTIU:     regs[inst.rd] = ((uint32_t)regs[inst.rs1] < ((uint32_t)inst.imm)) ? 1 : 0; PC = PC+4; break;
            case XORI:      regs[inst.rd] = regs[inst.rs1] ^ inst.imm; PC = PC+4; break;
            case ORI:       regs[inst.rd] = regs[inst.rs1] | inst.imm; PC = PC+4; break;
            case ANDI:      regs[inst.rd] = regs[inst.rs1] & inst.imm; PC = PC+4; break;
            case SLLI:      regs[inst.rd] = regs[inst.rs1] << inst.shamt; PC = PC+4; break;
            case SRLI:      regs[inst.rd] = regs[inst.rs1] >> inst.shamt; PC = PC+4; break;
            case SRAI:      regs[inst.rd] = ((int32_t)regs[inst.rs1]) >> inst.shamt; PC = PC+4; break;
            case ADD:       regs[inst.rd] = regs[inst.rs1] + regs[inst.rs2]; PC = PC+4; break; 
            case SUB:       regs[inst.rd] = regs[inst.rs1] - regs[inst.rs2]; PC = PC+4; break;
            case SLL:       regs[inst.rd] = regs[inst.rs1] << (regs[inst.rs2] & 0x0000001f); PC = PC+4; break;
            case SLT:       regs[inst.rd] = (((int32_t)regs[inst.rs1]) < ((int32_t)regs[inst.rs2])) ? 1 : 0; PC = PC+4; break;
            case SLTU:      regs[inst.rd] = (((uint32_t)regs[inst.rs1]) < ((uint32_t)regs[inst.rs2])) ? 1 : 0; PC = PC+4; break;
            case XOR:       regs[inst.rd] = regs[inst.rs1] ^ regs[inst.rs2]; PC = PC+4; break;
            case SRL:       regs[inst.rd] = ((uint32_t)regs[inst.rs1]) >> (regs[inst.rs2] & 0x0000001f); PC = PC+4; break;
            case SRA:       regs[inst.rd] = ((int32_t)regs[inst.rs1]) >> (regs[inst.rs2] & 0x0000001f); PC = PC+4; break;
            case OR:        regs[inst.rd] = regs[inst.rs1] | regs[inst.rs2]; PC = PC+4; break; 
            case AND:       regs[inst.rd] = regs[inst.rs1] & regs[inst.rs2]; PC = PC+4; break; 
            case ECALL:    status=1; break; 
            case EBREAK:   status=1; break;
            case CSRRW:     
            case CSRRS:     
            case CSRRC:     
            case CSRRWI:        
            case CSRRSI:        
            case CSRRCI:        
            default:       std::cerr << "Unimplemented" << std::endl; exit(1); break;
        }   
    }
    regs[0] = 0; //x0 register is always hardwired to 0.
    return status;
}

