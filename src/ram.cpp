#include "ram.h"
#include <cstdint>
#include <stdexcept>


RAM::RAM(uint32_t base, uint32_t size)
: m_base(base)
, m_size(size)
, m_mem(size, 0)
{
}

uint32_t RAM::read(uint32_t addr, uint8_t size)
{
    uint32_t offset = addr - m_base;
    
    if (offset + size > m_size)
    {
        throw std::runtime_error("Memory access out of bounds for RAM device: addr=0x" + std::to_string(addr) + ", size=" + std::to_string(size));
    }
    
    uint32_t mem;
    switch(size)
    {
        case 1: mem = m_mem[offset]; break;
        case 2: mem = m_mem[offset] | (m_mem[offset + 1] << 8); break;
        case 4: mem = m_mem[offset] | (m_mem[offset + 1] << 8) | (m_mem[offset + 2] << 16) | (m_mem[offset + 3] << 24); break;
        default: throw std::runtime_error("Unsupported memory access size: " + std::to_string(size));
    }

    return mem;
}

void RAM::write(uint32_t addr, uint8_t size, uint32_t val)
{
    uint32_t offset = addr - m_base;

    if (offset + size > m_size)
    {
        throw std::runtime_error("Memory access out of bounds for RAM device: addr=0x" + std::to_string(addr) + ", size=" + std::to_string(size));
    }

    switch(size)
    {
        case 1:
        {
            m_mem[offset] = val & 0xFF;
            break;
        }
        case 2:
        {
            m_mem[offset]     =  val       & 0xFF;
            m_mem[offset + 1] = (val >> 8) & 0xFF;
            break;
        }
        case 4:
        {
            m_mem[offset]     =  val        & 0xFF;
            m_mem[offset + 1] = (val >> 8)  & 0xFF;
            m_mem[offset + 2] = (val >> 16) & 0xFF;
            m_mem[offset + 3] = (val >> 24) & 0xFF;
            break;
        }
        default: throw std::runtime_error("Unsupported memory access size: " + std::to_string(size));
    }
}

bool RAM::contains(uint32_t addr)
{
    return addr >= m_base && addr < m_base + m_size;
}