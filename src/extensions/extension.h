/*
    The RISC-V extensions are defined here.
    Implementation of each extension is inside their corresponding *_impl.h files.
*/

#pragma once

#include "../encoding.h"

#include <cstdint>


// Definition
template<typename ISA>
class CPU;

struct RV32I
{
    using word_t = uint32_t;

    static bool decode(uint32_t encoded, DecodedInstr& d);

    template<typename ISA_t>
    static bool execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC);
};

struct RV32M
{
    using word_t = uint32_t;

    static bool decode(uint32_t encoded, DecodedInstr& d);

    template<typename ISA_t>
    static bool execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC);
};

struct RV32A
{
    using word_t = uint32_t;

    static bool decode(uint32_t encoded, DecodedInstr& d);

    template<typename ISA_t>
    static bool execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC);
};

struct RV32F
{
    using word_t = uint32_t;

    static bool decode(uint32_t encoded, DecodedInstr& d);

    template<typename ISA_t>
    static bool execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC);
};

struct RV32D
{
    using word_t = uint32_t;

    static bool decode(uint32_t encoded, DecodedInstr& d);

    template<typename ISA_t>
    static bool execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC);
};

struct RV32Zifencei
{
    using word_t = uint32_t;

    static bool decode(uint32_t encoded, DecodedInstr& d);

    template<typename ISA_t>
    static bool execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC);
};

struct RV32Zicsr
{
    using word_t = uint32_t;

    static bool decode(uint32_t encoded, DecodedInstr& d);

    template<typename ISA_t>
    static bool execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC);
};


// Implementation
#include "rv32i_impl.h"
#include "rv32m_impl.h"
#include "rv32a_impl.h"
#include "rv32zifencei_impl.h"
#include "rv32zicsr_impl.h"