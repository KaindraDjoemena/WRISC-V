#pragma once

#include "extension.h"
#include "../isa.h"
#include "../cpu.h"
#include "../encoding.h"
#include "utils.h"

#include <cstdint>
#include <iostream>
#include <limits>


inline
bool RV32Zicsr::decode(uint32_t encoding, DecodedInstr& d)
{
    d.instr = Instr::UNKNOWN;

    if (mask(encoding, 0, 2) != 0b11)
    {
        return false;
    }

    switch(baseOpcodeMap[mask(encoding, 5, 2)][mask(encoding, 2, 3)])
    {
        case Op::SYSTEM:
        {
            d.rd  = extractRd(encoding);
            d.rs1 = extractRs1(encoding);
            d.imm = extractImm(InstrFmt::I, encoding);

            uint8_t funct3 = mask(encoding, 12, 3);
            switch(funct3)
            {
                case 0b001: d.instr = Instr::CSRRW;  return true;
                case 0b010: d.instr = Instr::CSRRS;  return true;
                case 0b011: d.instr = Instr::CSRRC;  return true;
                case 0b101: d.instr = Instr::CSRRWI; return true;
                case 0b110: d.instr = Instr::CSRRSI; return true;
                case 0b111: d.instr = Instr::CSRRCI; return true;
                default: return false;
            };
        }
        default: return false;
    };
}

template<typename ISA_t>
bool RV32Zicsr::execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC)
{
    uint32_t csrAddr = static_cast<uint32_t>(d.imm) & 0xFFF;
    uint32_t oldVal  = 0;

    bool shouldRead = true;
    if ((d.instr == Instr::CSRRW || d.instr == Instr::CSRRWI) && d.rd == 0)
    {
        shouldRead = false;
    }

    if (shouldRead)
    {
        oldVal = cpu.csr[csrAddr];
    }

    uint32_t srcVal = 0;

    if (d.instr == Instr::CSRRWI || d.instr == Instr::CSRRSI || d.instr == Instr::CSRRCI)
    {
        srcVal = d.rs1; 
    }
    else
    {
        srcVal = cpu.r[d.rs1];
    }

    bool should_write = true;
    if ((d.instr == Instr::CSRRS  || d.instr == Instr::CSRRC || 
         d.instr == Instr::CSRRSI || d.instr == Instr::CSRRCI)
         && srcVal == 0)
    {
        should_write = false;
    }

    if (should_write)
    {
        uint32_t new_val = 0;
        switch(d.instr)
        {
            case Instr::CSRRW:
            case Instr::CSRRWI: new_val = srcVal;           break;

            case Instr::CSRRS:
            case Instr::CSRRSI: new_val = oldVal | srcVal;  break;

            case Instr::CSRRC:
            case Instr::CSRRCI: new_val = oldVal & ~srcVal; break;

            default: return false;
        }
        cpu.csr[csrAddr] = new_val;
    }

    if (shouldRead)
    {
        cpu.wReg(d.rd, oldVal);
    }

    return true;
}