#pragma once

#include "encoding.h"
#include "bus.h"
#include "extensions/utils.h"

#include <array>
#include <cstdint>
#include <iostream>


template<typename ISA>
class CPU
{
public:
	Bus& bus;
	std::array<typename ISA::word_t, 32> r;
	typename ISA::word_t pc;

	typename ISA::word_t csr[4096] = {0};

	typename ISA::word_t resAddr = 0;
	bool resValid = false;

	CPU(Bus& bus) : bus(bus), pc(0)
	{
		r.fill(0);
	}

    void wReg(uint32_t reg_idx, typename ISA::word_t value)
    {
        if (reg_idx != 0)
		{
            r[reg_idx] = value;
        }
    }

	void step()
	{
		typename ISA::word_t encoding = bus.read(pc, 4);
		DecodedInstr d = {};

		if (!ISA::decode(encoding, d)) 
		{
			throw std::runtime_error("Illegal instruction at PC: 0x...");
		}

		typename ISA::word_t nextPC = pc + 4;

		if (!ISA::execute(*this, d, nextPC))
		{
			throw std::runtime_error("Execution failed at PC: 0x...");
		}

		pc = nextPC;
		r[0] = 0;
		trace(pc, d, *this);
	}
};