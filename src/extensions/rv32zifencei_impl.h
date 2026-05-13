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
bool RV32Zifencei::decode(uint32_t encoding, DecodedInstr& d)
{
    return 0;
}

template<typename ISA_t>
bool RV32Zifencei::execute(CPU<ISA_t>& cpu, DecodedInstr d, uint32_t& nextPC)
{
    return 0;
}