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
bool RV32M::decode(uint32_t encoding, DecodedInstr& d)
{
    d.instr = Instr::UNKNOWN;

    if (mask(encoding, 0, 2) != 0b11)
    {
        return false;
    }

    switch(baseOpcodeMap[mask(encoding, 5, 2)][mask(encoding, 2, 3)])
    {
        case Op::OP:
        {
            if (mask(encoding, 25, 7) != 0b0000001)
            {
                return false;
            }

            d.rd  = mask(encoding, 7, 5);
            d.rs1 = extractRs1(encoding);
            d.rs2 = extractRs2(encoding);
            d.imm = 0;

            uint8_t funct3 = mask(encoding, 12, 3);
            switch(funct3)
            {
                case 0b000: d.instr = Instr::MUL;    return true;
                case 0b001: d.instr = Instr::MULH;   return true;
                case 0b010: d.instr = Instr::MULHSU; return true;
                case 0b011: d.instr = Instr::MULHU;  return true;
                case 0b100: d.instr = Instr::DIV;    return true;
                case 0b101: d.instr = Instr::DIVU;   return true;
                case 0b110: d.instr = Instr::REM;    return true;
                case 0b111: d.instr = Instr::REMU;   return true;
                default:    return false;
            };
        }
        default: return false;
    };
}

template<typename ISA_t>
bool RV32M::execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC)
{
    switch(d.instr)
    {
        case Instr::UNKNOWN: return true;
        case Instr::NOP:     return true;
        case Instr::MUL:
        {
            uint32_t op1 = (uint32_t)cpu.r[d.rs1];
            uint32_t op2 = (uint32_t)cpu.r[d.rs2];
            uint64_t prd = (uint64_t)op1 * (uint64_t)op2;

            cpu.wReg(d.rd, (uint32_t)prd);

            return true;
        }
        case Instr::MULH:
        {
            int32_t op1 = (int32_t)cpu.r[d.rs1];
            int32_t op2 = (int32_t)cpu.r[d.rs2];
            int64_t prd = (int64_t)op1 * (int64_t)op2;;

            cpu.wReg(d.rd, (uint32_t)(prd >> 32));

            return true;
        }
        case Instr::MULHSU:
        {
            int32_t  op1s = (int32_t) cpu.r[d.rs1];
            uint32_t op2u = (uint32_t)cpu.r[d.rs2];
            int64_t  prd  = (int64_t)op1s * (uint64_t)op2u;

            cpu.wReg(d.rd, (uint32_t)(prd >> 32));

            return true;
        }
        case Instr::MULHU:
        {
            uint32_t op1 = (uint32_t)cpu.r[d.rs1];
            uint32_t op2 = (uint32_t)cpu.r[d.rs2];
            uint64_t prd = (uint64_t)op1 * (uint64_t)op2;

            cpu.wReg(d.rd, (uint32_t)(prd >> 32));

            return true;
        }
        case Instr::DIV:
        {
            int32_t dvd = (int32_t)cpu.r[d.rs1];
            int32_t dvr = (int32_t)cpu.r[d.rs2];
            int64_t res;

            if (dvr == 0)
            {
                res = -1;
            }
            else if (dvd == std::numeric_limits<int32_t>::min() && dvr == -1)
            {
                res = std::numeric_limits<int32_t>::min();
            }
            else
            {
                res = dvd / dvr;
            }

            cpu.wReg(d.rd, (uint32_t)res);

            return true;
        }
        case Instr::DIVU:
        {
            uint32_t dvd = cpu.r[d.rs1];
            uint32_t dvr = cpu.r[d.rs2];
            uint32_t res;

            if (dvr == 0)
            {
                res = std::numeric_limits<uint32_t>::max();
            }
            else
            {
                res = dvd / dvr;
            }

            cpu.wReg(d.rd, res);

            return true;
        }
        case Instr::REM:
        {
            int32_t dvd = (int32_t)cpu.r[d.rs1];
            int32_t dvr = (int32_t)cpu.r[d.rs2];
            int32_t res;

            if (dvr == 0)
            {
                res = dvd;
            }
            else if (dvd == std::numeric_limits<int32_t>::min() && dvr == -1)
            {
                res = 0;
            }
            else
            {
                res = dvd % dvr;
            }

            cpu.wReg(d.rd, (uint32_t)res);

            return true;
        }
        case Instr::REMU:
        {
            uint32_t dvd = cpu.r[d.rs1];
            uint32_t dvr = cpu.r[d.rs2];
            uint32_t res;

            if (dvr == 0)
            {
                res = dvd;
            }
            else
            {
                res = dvd % dvr;
            }

            cpu.wReg(d.rd, res);

            return true;
        }
        default: return false;
    };
    return false;
}