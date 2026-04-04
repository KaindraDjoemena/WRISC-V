#pragma once

#include "extension.h"
#include "../isa.h"
#include "../cpu.h"
#include "../encoding.h"
#include "utils.h"

#include <cstdint>
#include <iostream>


inline
bool RV32I::decode(uint32_t encoding, DecodedInstr& d)
{
    d.instr = Instr::UNKNOWN;

    if (mask(encoding, 0, 2) != 0b11)
    {
        return false;
    }

    switch(baseOpcodeMap[mask(encoding, 5, 2)][mask(encoding, 2, 3)])
    {
        case Op::OP_IMM:
        {
            d.rd  = extractRd(encoding);
            d.rs1 = extractRs1(encoding);
            d.imm = extractImm(InstrFmt::I, encoding);

            if (d.rd == 0 && d.rs1 == 0 && d.imm == 0) { d.instr = Instr::NOP; };

            uint8_t funct3 = mask(encoding, 12, 3);
            switch(funct3)
            {
                case 0b000: d.instr = Instr::ADDI;  return true;
                case 0b010: d.instr = Instr::SLTI;  return true;
                case 0b011: d.instr = Instr::SLTIU; return true;
                case 0b111: d.instr = Instr::ANDI;  return true;
                case 0b110: d.instr = Instr::ORI;   return true;
                case 0b100: d.instr = Instr::XORI;  return true;
                case 0b001:
                {
                    d.imm   = mask(encoding, 20, 5);
                    d.instr = Instr::SLLI; 
                    return true;
                }
                case 0b101:
                {
                    d.imm = mask(encoding, 20, 5);
                    uint8_t funct7 = mask(encoding, 25, 7);
                    switch(funct7)
                    {
                        case 0b0000000: d.instr = Instr::SRLI; return true;
                        case 0b0100000: d.instr = Instr::SRAI; return true;
                    };
                }
                default: return false;
            };
        }
        case Op::LUI:
        {
            d.instr = Instr::LUI;
            d.rd    = extractRd(encoding);
            d.imm   = extractImm(InstrFmt::U, encoding);
            return true;
        }
        case Op::AUIPC:
        {
            d.instr = Instr::AUIPC;
            d.rd    = extractRd(encoding);
            d.imm   = extractImm(InstrFmt::U, encoding);
            return true;
        }
        case Op::OP:
        {
            d.rd  = mask(encoding, 7, 5);
            d.rs1 = extractRs1(encoding);
            d.rs2 = extractRs2(encoding);
            d.imm = 0;

            uint8_t funct3 = mask(encoding, 12, 3);
            switch(funct3)
            {
                case 0b000:
                {
                    uint8_t funct7 = mask(encoding, 25, 7);
                    switch(funct7)
                    {
                        case 0b0000000: d.instr = Instr::ADD; return true;
                        case 0b0100000: d.instr = Instr::SUB; return true;
                        default:        return false;
                    }
                }
                case 0b001: d.instr = Instr::SLL;  return true;
                case 0b010: d.instr = Instr::SLT;  return true;
                case 0b011: d.instr = Instr::SLTU; return true;
                case 0b100: d.instr = Instr::XOR;  return true;
                case 0b101:
                {
                    uint8_t funct7 = mask(encoding, 25, 7);
                    switch(funct7)
                    {
                        case 0b0000000: d.instr = Instr::SRL; return true;
                        case 0b0100000: d.instr = Instr::SRA; return true;
                        default:        return false;
                    }
                }
                case 0b110: d.instr = Instr::OR;  return true;
                case 0b111: d.instr = Instr::AND; return true;
                default:    return false;
            };
        }
        case Op::JAL:
        {
            d.instr = Instr::JAL;
            d.rd    = extractRd(encoding);
            d.imm   = extractImm(InstrFmt::J, encoding);
            return true;
        }
        case Op::JALR:
        {
            d.instr = Instr::JALR;
            d.rd    = extractRd(encoding);
            d.rs1   = extractRs1(encoding);
            d.imm   = extractImm(InstrFmt::I, encoding);
            return true;
        }
        case Op::BRANCH:
        {
            d.rs1 = extractRs1(encoding);
            d.rs2 = extractRs2(encoding);
            d.imm = extractImm(InstrFmt::B, encoding);

            uint8_t funct3 = mask(encoding, 12, 3);
            switch(funct3)
            {
                case 0b000: d.instr = Instr::BEQ;  return true;
                case 0b001: d.instr = Instr::BNE;  return true;
                case 0b100: d.instr = Instr::BLT;  return true;
                case 0b101: d.instr = Instr::BGE;  return true;
                case 0b110: d.instr = Instr::BLTU; return true;
                case 0b111: d.instr = Instr::BGEU; return true;
                default:    return false;
            };
        }
        case Op::LOAD:
        {
            d.rd  = extractRd(encoding);
            d.rs1 = extractRs1(encoding);
            d.imm = extractImm(InstrFmt::I, encoding);

            uint8_t funct3 = mask(encoding, 12, 3);
            switch(funct3)
            {
                case 0b000: d.instr = Instr::LB;  return true;
                case 0b001: d.instr = Instr::LH;  return true;
                case 0b010: d.instr = Instr::LW;  return true;
                case 0b100: d.instr = Instr::LBU; return true;
                case 0b101: d.instr = Instr::LHU; return true;
                default:    return false;
            };
        }
        case Op::STORE:
        {
            d.rs1 = extractRs1(encoding);
            d.rs2 = extractRs2(encoding);
            d.imm = extractImm(InstrFmt::S, encoding);

            uint8_t funct3 = mask(encoding, 12, 3);
            switch(funct3)
            {
                case 0b000: d.instr = Instr::SB; return true;
                case 0b001: d.instr = Instr::SH; return true;
                case 0b010: d.instr = Instr::SW; return true;
                default:    return false;
            };
        }
        case Op::MISC_MEM:
        {
            d.rd  = extractRd(encoding);
            d.rs1 = extractRs1(encoding);
            d.imm = extractImm(InstrFmt::I, encoding);

            uint8_t fm = mask(encoding, 28, 4);

            if (encoding == 0x0100000F) { d.instr = Instr::PAUSE;     return true; }
            if (fm == 0b1000)           { d.instr = Instr::FENCE_TSO; return true; }
            
            d.instr = Instr::FENCE;
            return true;
        }
        case Op::SYSTEM:
        {
            uint32_t imm = extractImm(InstrFmt::I, encoding);
            switch(imm)
            {
                case 0b0: d.instr = Instr::ECALL;  return true;
                case 0b1: d.instr = Instr::EBREAK; return true;
                default:  return false;
            }
        }
        default: return false;
    };
}

template<typename ISA_t>
bool RV32I::execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC)
{
    switch(d.instr)
    {
        case Instr::UNKNOWN: return true;
        case Instr::NOP:     return true; // NOTE: Shouldnt NOP be implicitly handled by ADDI x0, x0, 0 ? it just advances the pc and increments any applicable perf counters.
        case Instr::LUI:   cpu.wReg(d.rd, d.imm); return true;
        case Instr::AUIPC: cpu.wReg(d.rd, cpu.pc + (uint32_t)d.imm); return true;
        case Instr::JAL:
        {
            cpu.wReg(d.rd, nextPC);
            nextPC = cpu.pc + d.imm;
            return true;    // NOTE: x1 and x5 convention
        }
        case Instr::JALR: 
        {
            cpu.wReg(d.rd, nextPC);
            nextPC = (cpu.r[d.rs1] + d.imm) & ~1u;
            return true;
        }// NOTE: IMplement instruction-address-misaligned exception
        case Instr::BEQ:   if ((int32_t) cpu.r[d.rs1] == (int32_t) cpu.r[d.rs2]) nextPC = cpu.pc + d.imm; return true;
        case Instr::BNE:   if ((int32_t) cpu.r[d.rs1] != (int32_t) cpu.r[d.rs2]) nextPC = cpu.pc + d.imm; return true;
        case Instr::BLT:   if ((int32_t) cpu.r[d.rs1] <  (int32_t) cpu.r[d.rs2]) nextPC = cpu.pc + d.imm; return true;
        case Instr::BGE:   if ((int32_t) cpu.r[d.rs1] >= (int32_t) cpu.r[d.rs2]) nextPC = cpu.pc + d.imm; return true;
        case Instr::BLTU:  if ((uint32_t)cpu.r[d.rs1] <  (uint32_t)cpu.r[d.rs2]) nextPC = cpu.pc + d.imm; return true;
        case Instr::BGEU:  if ((uint32_t)cpu.r[d.rs1] >= (uint32_t)cpu.r[d.rs2]) nextPC = cpu.pc + d.imm; return true;
        
        case Instr::LB:    cpu.wReg(d.rd, (int8_t)  cpu.bus.read(cpu.r[d.rs1] + d.imm, 1)); return true;
        case Instr::LH:    cpu.wReg(d.rd, (int16_t) cpu.bus.read(cpu.r[d.rs1] + d.imm, 2)); return true;
        case Instr::LW:    cpu.wReg(d.rd,           cpu.bus.read(cpu.r[d.rs1] + d.imm, 4)); return true;
        case Instr::LBU:   cpu.wReg(d.rd, (uint8_t) cpu.bus.read(cpu.r[d.rs1] + d.imm, 1)); return true;
        case Instr::LHU:   cpu.wReg(d.rd, (uint16_t)cpu.bus.read(cpu.r[d.rs1] + d.imm, 2)); return true;

        case Instr::SB:    cpu.bus.write(cpu.r[d.rs1] + d.imm, 1, cpu.r[d.rs2]); return true;
        case Instr::SH:    cpu.bus.write(cpu.r[d.rs1] + d.imm, 2, cpu.r[d.rs2]); return true;
        case Instr::SW:    cpu.bus.write(cpu.r[d.rs1] + d.imm, 4, cpu.r[d.rs2]); return true;
        
        case Instr::ADDI:  cpu.wReg(d.rd, cpu.r[d.rs1] + d.imm); return true;
        case Instr::SLTI:  cpu.wReg(d.rd, ((int32_t) cpu.r[d.rs1] < (int32_t) d.imm) ? 1 : 0); return true;
        case Instr::SLTIU: cpu.wReg(d.rd, ((uint32_t)cpu.r[d.rs1] < (uint32_t)d.imm) ? 1 : 0); return true;
        case Instr::XORI:  cpu.wReg(d.rd, cpu.r[d.rs1] ^ d.imm); return true;
        case Instr::ORI:   cpu.wReg(d.rd, cpu.r[d.rs1] | d.imm); return true;
        case Instr::ANDI:  cpu.wReg(d.rd, cpu.r[d.rs1] & d.imm); return true;
        case Instr::SLLI:  cpu.wReg(d.rd, cpu.r[d.rs1] << d.imm); return true;
        case Instr::SRLI:  cpu.wReg(d.rd, (uint32_t)cpu.r[d.rs1] >> (d.imm & 0x1F)); return true;
        case Instr::SRAI:  cpu.wReg(d.rd, (int32_t) cpu.r[d.rs1] >> (d.imm & 0x1F)); return true;

        case Instr::ADD:   cpu.wReg(d.rd, cpu.r[d.rs1] + cpu.r[d.rs2]); return true;   // NOTE: Read more about overflows and XLEN
        case Instr::SUB:   cpu.wReg(d.rd, cpu.r[d.rs1] - cpu.r[d.rs2]); return true;
        case Instr::SLL:   cpu.wReg(d.rd, cpu.r[d.rs1] << (cpu.r[d.rs2] & 0x1F)); return true;
        case Instr::SLT:   cpu.wReg(d.rd, ((int32_t) cpu.r[d.rs1] < (int32_t) cpu.r[d.rs2]) ? 1 : 0); return true; 
        case Instr::SLTU:  cpu.wReg(d.rd, ((uint32_t)cpu.r[d.rs1] < (uint32_t)cpu.r[d.rs2]) ? 1 : 0); return true;
        case Instr::XOR:   cpu.wReg(d.rd, cpu.r[d.rs1] ^ cpu.r[d.rs2]); return true;
        case Instr::SRL:   cpu.wReg(d.rd, (uint32_t)cpu.r[d.rs1] >> (cpu.r[d.rs2] & 0x1F)); return true;
        case Instr::SRA:   cpu.wReg(d.rd, (int32_t) cpu.r[d.rs1] >> (cpu.r[d.rs2] & 0x1F)); return true;    // NOTE: Read more on logical bitwise shifts
        case Instr::OR:    cpu.wReg(d.rd, cpu.r[d.rs1] | cpu.r[d.rs2]); return true;
        case Instr::AND:   cpu.wReg(d.rd, cpu.r[d.rs1] & cpu.r[d.rs2]); return true;
        case Instr::FENCE: 
        case Instr::FENCE_TSO:
        case Instr::PAUSE:        return true;
        case Instr::ECALL:
        case Instr::EBREAK:       return true;
        default:                  return false;
    };
}