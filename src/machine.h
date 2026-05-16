#pragma once

#include "bus.h"
#include "cpu.h"
#include "ram.h"
#include "extension.h"
#include <cstdint>
#include <stdexcept>
#include <fstream>
#include <vector>


template <typename T_ISA>
class Machine
{
public:

    Bus bus;
    RAM ram;
    CPU<T_ISA> cpu;

    Machine(uint32_t ramAddr = 0x80000000, uint32_t ramSize = 0x1000)
    : bus()
    , ram(ramAddr, ramSize)
    , cpu(bus)
    {
        bus.connect(&ram);
        cpu.r.fill(0);
        cpu.pc = ramAddr;
    }

    void execute_instruction(uint32_t encoding)
    {
        write_word_to_ram(cpu.pc, encoding);
        
        cpu.step();
    }

    void write_word_to_ram(uint32_t addr, uint32_t value)
    {
        bus.write(addr, 4, value);
    }

    void write_half_to_ram(uint32_t addr, uint16_t value)
    {
        bus.write(addr, 2, value);
    }

    void write_byte_to_ram(uint32_t addr, uint8_t value)
    {
        bus.write(addr, 1, value);
    }

    uint32_t read_word_from_ram(uint32_t addr)
    {
        return bus.read(addr, 4);
    }

    uint16_t read_half_from_ram(uint32_t addr)
    {
        return static_cast<uint16_t>(bus.read(addr, 2));
    }

    uint8_t read_byte_from_ram(uint32_t addr)
    {
        return static_cast<uint8_t>(bus.read(addr, 1));
    }

    void load_binary(const std::string& filepath, uint32_t load_addr)
    {
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file) throw std::runtime_error("Could not open binary file.");

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!file.read((char*)buffer.data(), size)) throw std::runtime_error("Read error.");

        for (size_t i = 0; i < buffer.size(); ++i)
        {
            bus.write(load_addr + i, 1, buffer[i]);
        }
        
        cpu.pc = load_addr;
    }
};