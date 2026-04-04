#pragma once

#include <cstdint>
#include <array>


enum class InstrFmt : uint8_t
{
    R, I, S, B, U, J
};

enum class Op : uint8_t
{
    LOAD,  LOAD_FP,   CUSTOM0,  MISC_MEM, OP_IMM, AUIPC, OP_IMM_32,
    STORE,  STORE_FP, CUSTOM1,  AMO,      OP,     LUI,   OP_32,
    MADD,   MSUB,     NMSUB,    NMADD,    OP_FP,  OP_V,  CUSTOM2,
    BRANCH, JALR,     RESERVED, JAL,      SYSTEM, OP_VE, CUSTOM3
};

/*
 *  Format: opcodeMap[inst[6:5]][inst[4:2]]
 *
 *  0110111 encodes for LUI
 *  opcodeMap[01][101] = LUI
*/
constexpr std::array<std::array<Op, 7>, 4> baseOpcodeMap = {{
        {Op::LOAD,   Op::LOAD_FP,  Op::CUSTOM0,  Op::MISC_MEM, Op::OP_IMM, Op::AUIPC, Op::OP_IMM_32},
        {Op::STORE,  Op::STORE_FP, Op::CUSTOM1,  Op::AMO,      Op::OP,     Op::LUI,   Op::OP_32    },
        {Op::MADD,   Op::MSUB,     Op::NMSUB,    Op::NMADD,    Op::OP_FP,  Op::OP_V,  Op::CUSTOM2  },
        {Op::BRANCH, Op::JALR,     Op::RESERVED, Op::JAL,      Op::SYSTEM, Op::OP_VE, Op::CUSTOM3  }}};

enum class Instr : uint16_t
{
    // RV32I Base Instructions
    UNKNOWN = 0,
    NOP,
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
    FENCE,
    FENCE_TSO,
    PAUSE,
    ECALL,
    EBREAK,

    // RV32M
    MUL,
    MULH,
    MULHSU,
    MULHU,
    DIV,
    DIVU,
    REM,
    REMU
};


struct DecodedInstr
{
    Instr   instr;
    uint8_t rd;
    uint8_t rs1;
    uint8_t rs2;
    int32_t imm;    // sign-exteded
};