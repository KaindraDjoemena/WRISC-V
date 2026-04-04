#pragma once

#include "busDevice.h"

#include <cstdint>
#include <vector>


class RAM : public BusDevice
{
public:
    RAM(uint32_t base, uint32_t size);

    uint32_t read(uint32_t addr, uint8_t size) override;
    void write(uint32_t addr, uint8_t size, uint32_t val) override;
    bool contains(uint32_t addr) override;

private:
    uint32_t m_base, m_size;
    std::vector<uint8_t> m_mem;
};