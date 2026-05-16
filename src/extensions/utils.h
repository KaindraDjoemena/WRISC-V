#pragma once

#include "../encoding.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>

template<typename ISA>
class CPU;

inline
static uint32_t mask(uint32_t val, uint8_t n, uint8_t len)
{
    return (val >> n) & ((1u << len) - 1);
}

inline
static int32_t signExtend(int32_t val, int32_t n)
{
    uint32_t sign_bit = 1u << (n - 1);
    return (int32_t)((val ^ sign_bit) - sign_bit);
}

inline
static uint32_t extractRs1(uint32_t encoding)
{
    return mask(encoding, 15, 5);
}

inline
static uint32_t extractRs2(uint32_t encoding)
{
    return mask(encoding, 20, 5);
}

inline
static uint32_t extractRd(uint32_t encoding)
{
    return mask(encoding, 7, 5);
}

inline
static int32_t extractImm(InstrFmt type, uint32_t encoding)
{
    /*
     * https://docs.riscv.org/reference/isa/unpriv/rv32.html#immtypes
    */

    // NOTE: -Wall -Wswitch
    switch(type)
    {
        case InstrFmt::I:
        {
            return signExtend(mask(encoding, 20, 12), 12);
        }
        case InstrFmt::S:
        {
            uint32_t imm11_5 = mask(encoding, 25, 7);
            uint32_t imm4_0  = mask(encoding, 7, 5);
            return signExtend((imm11_5 << 5) | (imm4_0), 12);
        }
        case InstrFmt::B:
        {
            uint32_t imm12   = mask(encoding, 31, 1);
            uint32_t imm10_5 = mask(encoding, 25, 6);
            uint32_t imm4_1  = mask(encoding, 8, 4);
            uint32_t imm11   = mask(encoding, 7, 1);
            return signExtend((imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1) | 0, 13);
        }
        case InstrFmt::U:
        {
            return (int32_t)(encoding & 0xFFFFF000);
        }
        case InstrFmt::J:
        {
            uint32_t imm20    = mask(encoding, 31, 1);
            uint32_t imm10_1  = mask(encoding, 21, 10);
            uint32_t imm11    = mask(encoding, 20, 1);
            uint32_t imm19_12 = mask(encoding, 12, 8);
            return signExtend((imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1), 21);
        }
        default: throw std::runtime_error("Invalid Immediate Type");
    };
}

inline const char* instrToStr(Instr i)
{
    switch(i)
    {
        // --- RV32I (Base Integer) ---
        case Instr::LUI:      return "LUI";
        case Instr::AUIPC:    return "AUIPC";
        case Instr::JAL:      return "JAL";
        case Instr::JALR:     return "JALR";
        case Instr::BEQ:      return "BEQ";
        case Instr::BNE:      return "BNE";
        case Instr::BLT:      return "BLT";
        case Instr::BGE:      return "BGE";
        case Instr::BLTU:     return "BLTU";
        case Instr::BGEU:     return "BGEU";
        case Instr::LB:       return "LB";
        case Instr::LH:       return "LH";
        case Instr::LW:       return "LW";
        case Instr::LBU:      return "LBU";
        case Instr::LHU:      return "LHU";
        case Instr::SB:       return "SB";
        case Instr::SH:       return "SH";
        case Instr::SW:       return "SW";
        case Instr::ADDI:     return "ADDI";
        case Instr::SLTI:     return "SLTI";
        case Instr::SLTIU:    return "SLTIU";
        case Instr::XORI:     return "XORI";
        case Instr::ORI:      return "ORI";
        case Instr::ANDI:     return "ANDI";
        case Instr::SLLI:     return "SLLI";
        case Instr::SRLI:     return "SRLI";
        case Instr::SRAI:     return "SRAI";
        case Instr::ADD:      return "ADD";
        case Instr::SUB:      return "SUB";
        case Instr::SLL:      return "SLL";
        case Instr::SLT:      return "SLT";
        case Instr::SLTU:     return "SLTU";
        case Instr::XOR:      return "XOR";
        case Instr::SRL:      return "SRL";
        case Instr::SRA:      return "SRA";
        case Instr::OR:       return "OR";
        case Instr::AND:      return "AND";
        case Instr::FENCE:    return "FENCE";
        case Instr::FENCE_I:  return "FENCE.I";
        case Instr::ECALL:    return "ECALL";
        case Instr::EBREAK:   return "EBREAK";
        case Instr::PAUSE:    return "PAUSE";
        case Instr::FENCE_TSO: return "FENCE.TSO";

        // --- RV32M (Multiply/Divide) ---
        case Instr::MUL:      return "MUL";
        case Instr::MULH:     return "MULH";
        case Instr::MULHSU:   return "MULHSU";
        case Instr::MULHU:    return "MULHU";
        case Instr::DIV:      return "DIV";
        case Instr::DIVU:     return "DIVU";
        case Instr::REM:      return "REM";
        case Instr::REMU:     return "REMU";

        // --- Zicsr (Control and Status Registers) ---
        case Instr::CSRRW:    return "CSRRW";
        case Instr::CSRRS:    return "CSRRS";
        case Instr::CSRRC:    return "CSRRC";
        case Instr::CSRRWI:   return "CSRRWI";
        case Instr::CSRRSI:   return "CSRRSI";
        case Instr::CSRRCI:   return "CSRRCI";

        // --- RV32A (Atomics - for your next implementation) ---
        // case Instr::LR_W:      return "LR.W";
        // case Instr::SC_W:      return "SC.W";
        // case Instr::AMOSWAP_W: return "AMOSWAP.W";
        // case Instr::AMOADD_W:  return "AMOADD.W";
        // case Instr::AMOXOR_W:  return "AMOXOR.W";
        // case Instr::AMOAND_W:  return "AMOAND.W";
        // case Instr::AMOOR_W:   return "AMOOR.W";
        // case Instr::AMOMIN_W:  return "AMOMIN.W";
        // case Instr::AMOMAX_W:  return "AMOMAX.W";
        // case Instr::AMOMINU_W: return "AMOMINU.W";
        // case Instr::AMOMAXU_W: return "AMOMAXU.W";

        // --- Privileged / System Stubs ---
        case Instr::MRET:     return "MRET";
        case Instr::SRET:     return "SRET";
        case Instr::WFI:      return "WFI";

        case Instr::UNKNOWN:
        default:              return "UNKNOWN";
    }
}

inline
const char* regToAbi(int i)
{
    const char* abi[] = {
        "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
        "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
        "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
    };
    return (i >= 0 && i < 32) ? abi[i] : "??";
}

template<typename ISA_t>
void trace(uint32_t pc, const DecodedInstr& d, const CPU<ISA_t>& cpu)
{
    printf("PC: [0x%08x]  %-10s ", pc, instrToStr(d.instr));

    if (d.rd != 0)
    {
        printf("%s(x%d)=0x%08x ", regToAbi(d.rd), d.rd, cpu.r[d.rd]);
    }

    if (d.rs1 != 0)
    {
        printf("s1:%s(x%d)=0x%x ", regToAbi(d.rs1), d.rs1, cpu.r[d.rs1]);
    }

    if (d.rs2 != 0)
    {
        printf("s2:%s(x%d)=0x%x ", regToAbi(d.rs2), d.rs2, cpu.r[d.rs2]);
    }
    
    if (d.imm != 0)
    {
        printf("imm:0x%x ", d.imm);
    }

    printf(" | gp(x3):%d\n", cpu.r[3]);
}