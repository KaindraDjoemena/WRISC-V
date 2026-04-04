#pragma once

#include "../encoding.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>


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

inline
static void trace(uint32_t pc, const DecodedInstr& d)
{
    std::cout << "pc: " << pc << " | " << (int)d.instr << " | rd: " << d.rd << " | rs1: " << d.rs1 << " | rs2: " << d.rs2 << " | imm: " << d.imm << '\n';
}