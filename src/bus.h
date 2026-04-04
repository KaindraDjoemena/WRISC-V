#pragma once

#include "busDevice.h"

#include <cstdint>
#include <vector>
#include <stdexcept>


class Bus
{
public:
    void connect(BusDevice* device)
    {
        m_devices.push_back(device);
    }

    uint32_t read(uint32_t addr, uint8_t size)
    {
        for (auto* d : m_devices)
        {
            if (d->contains(addr))
            {
                return d->read(addr, size);
            }
        }

        throw std::runtime_error("read from unmapped address");
    }

    void write(uint32_t addr, uint8_t size, uint32_t val)
    {
        for (auto* d : m_devices)
        {
            if (d->contains(addr))
            {
                d->write(addr, size, val);
                return;
            }
        }

        throw std::runtime_error("write to unmapped address");
    }

private:
    std::vector<BusDevice*> m_devices;
};