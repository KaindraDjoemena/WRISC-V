#pragma once

#include "bus.h"
#include "cpu.h"
#include "ram.h"
#include <cstdint>
#include <stdexcept>

template<typename ISA>
class Machine
{
public:
    Machine()
    : bus()
    , ram(0x80000000, 32 * 1024 * 1024)
    , cpu(bus)
    {
        bus.connect(&ram);
    }

    const RAM&      getRAM() { return ram; }
    const CPU<ISA>& getCPU() { return cpu; }

    void runSteps(uint32_t nSteps)
    {
        for (uint32_t i = 0; i < nSteps; ++i)
        {
            try
            {
                cpu.step();
            }
            catch (const std::runtime_error& e)
            {
                std::cerr << "Execution halted due to error: " << e.what() << std::endl;
                break;
            }
        }
    }

private:
    Bus bus;
    RAM ram;
    CPU<ISA> cpu;
};