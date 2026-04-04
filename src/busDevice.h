#pragma once

#include <cstdint>


class BusDevice
{
public:
    virtual uint32_t read(uint32_t addr, uint8_t size) = 0;
    virtual void write(uint32_t addr, uint8_t size, uint32_t val) = 0;
    virtual bool contains(uint32_t addr) = 0;
    virtual ~BusDevice() = default;
};