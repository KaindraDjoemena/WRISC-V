#pragma once

#include "extensions/extension.h"
#include "encoding.h"

#include <cstdint>
#include <optional>


template<typename... Exts>
struct ISA
{
    using word_t  = uint32_t;
    using instr_t = Instr;
    
    static std::optional<DecodedInstr> decode(word_t enc)
    {
        DecodedInstr d;
        bool matched = (... || Exts::decode(enc, d));
        return matched ? std::optional{d} : std::nullopt;
    }

    static void execute(CPU<ISA>& cpu, DecodedInstr d, word_t& nextPC)
    {
        bool matched = (... || Exts::execute(cpu, d, nextPC));
    }
};