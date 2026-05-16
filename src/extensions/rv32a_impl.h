#pragma once

#include "extension.h"
#include "../isa.h"
#include "../cpu.h"
#include "../encoding.h"
#include "utils.h"

#include <cstdint>
#include <iostream>


inline
bool RV32A::decode(uint32_t encoding, DecodedInstr& d)
{
    if (mask(encoding, 0, 7) != 0b0101111)
    {
        return false;
    }

    d.rd  = extractRd(encoding);
    d.rs1 = extractRs1(encoding);
    d.rs2 = extractRs2(encoding);
    
    uint8_t funct5 = mask(encoding, 27, 5);

    switch (funct5)
    {
        case 0b00010: d.instr = Instr::LR_W;      return true;
        case 0b00011: d.instr = Instr::SC_W;      return true;
        case 0b00001: d.instr = Instr::AMOSWAP_W; return true;
        case 0b00000: d.instr = Instr::AMOADD_W;  return true;
        case 0b00100: d.instr = Instr::AMOXOR_W;  return true;
        case 0b01100: d.instr = Instr::AMOAND_W;  return true;
        case 0b01000: d.instr = Instr::AMOOR_W;   return true;
        case 0b10000: d.instr = Instr::AMOMIN_W;  return true;
        case 0b10100: d.instr = Instr::AMOMAX_W;  return true;
        case 0b11000: d.instr = Instr::AMOMINU_W; return true;
        case 0b11100: d.instr = Instr::AMOMAXU_W; return true;
        default: return false;
    }
}

template<typename ISA_t>
bool RV32A::execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC)
{
    uint32_t addr = cpu.r[d.rs1];

    if (d.instr == Instr::LR_W)
    {
        cpu.resValid = true;
        cpu.resAddr  = addr;
        cpu.wReg(d.rd, cpu.bus.read(addr, 4));
        return true;
    }
    
    if (d.instr == Instr::SC_W)
    {
        if (cpu.resValid && cpu.resAddr == addr)
        {
            cpu.bus.write(addr, 4, cpu.r[d.rs2]);
            cpu.wReg(d.rd, 0); // 0 = Success
        }
        else
        {
            cpu.wReg(d.rd, 1);
        }
        cpu.resValid = false;  // SC always clears reservation
        return true;
    }

    // AMO
    uint32_t old_val = cpu.bus.read(addr, 4);
    uint32_t src_val = cpu.r[d.rs2];
    uint32_t res = 0;

    switch (d.instr)
    {
        case Instr::AMOSWAP_W: res = src_val; break;
        case Instr::AMOADD_W:  res = old_val + src_val; break;
        case Instr::AMOXOR_W:  res = old_val ^ src_val; break;
        case Instr::AMOAND_W:  res = old_val & src_val; break;
        case Instr::AMOOR_W:   res = old_val | src_val; break;
        case Instr::AMOMIN_W:  res = std::min((int32_t)old_val, (int32_t)src_val); break;
        case Instr::AMOMAX_W:  res = std::max((int32_t)old_val, (int32_t)src_val); break;
        case Instr::AMOMINU_W: res = std::min(old_val, src_val); break;
        case Instr::AMOMAXU_W: res = std::max(old_val, src_val); break;
        default: return false;
    }

    cpu.bus.write(addr, 4, res);
    cpu.wReg(d.rd, old_val);

    cpu.resValid = false;  // AMO clears reservation

    return true;
}