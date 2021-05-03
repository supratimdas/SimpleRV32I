#ifndef __SIMPLERV32I_H__
#define __SIMPLERV32I_H__
#include <stdint.h>

typedef enum {
    U_TYPE,
    I_TYPE,
    R_TYPE,
    J_TYPE,
    S_TYPE,
    B_TYPE,
    INVALID_TYPE
} rv32i_op_type;

typedef enum {
    LUI,
    AUIPC,
    JAL,
    JALR,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    LB,
    LH,
    LW,
    LBU,
    LHU,
    SB,
    SH,
    SW,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    ECALL,
    EBREAK,
    CSRRW,
    CSRRS,
    CSRRC,
    CSRRWI,
    CSRRSI,
    CSRRCI
} rv32i_operation;


class RV32I_INST {
    public:
        uint8_t     opcode;
        uint8_t     funct3;
        uint8_t     funct7;
        uint8_t     shamt;
        uint8_t     rd;
        uint8_t     rs1;
        uint8_t     rs2;
        int32_t     imm;
        uint32_t    inst;

        rv32i_op_type     getType();
        rv32i_operation   getOperation();
        void decodeInst(uint32_t inst);
        RV32I_INST(); 
};

class SimpleRV32I {
    private:
        uint32_t regs[32];
        uint32_t mem_size;
        uint8_t *inst_mem;
        uint8_t *data_mem;
        RV32I_INST inst;
        uint8_t status;
        uint32_t PC;

    public:
        SimpleRV32I(int=4000);
        ~SimpleRV32I();
        void loadProgram(std::string="code.txt");
        void loadData(std::string="data.txt");
        void dumpData(std::string="data_out.txt");
        void dumpRegs(std::string="regs_out.txt");
        int step();
};


#endif
