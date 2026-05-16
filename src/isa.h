#pragma once

#include "extensions/extension.h"
#include "encoding.h"

#include <cstdint>
#include <optional>


template<typename... Exts>
struct ISA
{
    using word_t  = uint32_t;

    static bool decode(word_t enc, DecodedInstr& d)
    {
        return (... || Exts::decode(enc, d));
    }

    template<typename CPUType>
    static bool execute(CPUType& cpu, DecodedInstr d, word_t& nextPC)
    {
        return (... || Exts::execute(cpu, d, nextPC));
    }
};