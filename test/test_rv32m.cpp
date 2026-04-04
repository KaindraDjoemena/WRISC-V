#include <catch2/catch_all.hpp>
#include <cstdint>
#include "cpu.h"
#include "bus.h"
#include "ram.h"
#include "isa.h"
#include "encoding.h"
#include "extensions/extension.h"


struct TestCPUFixture {
    Bus bus;
    RAM ram;
    CPU<RV32M> cpu;

    TestCPUFixture()
    : bus()
    , ram(0x80000000, 0x1000) // 4KB
    , cpu(bus)
    {
        bus.connect(&ram);
        cpu.r.fill(0);
        cpu.pc = 0x80000000;
    }

    void execute_instruction(uint32_t encoding)
    {
        uint32_t initial_pc = cpu.pc;

        DecodedInstr d;
        RV32I::decode(encoding, d);

        if (d.instr == Instr::UNKNOWN)
        {
            FAIL("Illegal instruction encountered at PC 0x" << std::hex << initial_pc << " with encoding: 0x" << encoding);
        }

        uint32_t nextPC = initial_pc + 4;
        RV32I::execute(cpu, d, nextPC);

        cpu.pc = nextPC;
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
};


/*
    MUL x3, x1, x2
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32M: MUL pos * pos", "[RV32M][Execute][MUL]")
{
    cpu.r[1] = 10;
    cpu.r[2] = 5;
    execute_instruction(0x022081b3);
    REQUIRE(cpu.r[3] == 50);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    MUL x3, x1, x2
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: MUL pos * neg", "[RV32M][Execute][MUL]")
{
    cpu.r[1] = 3;
    cpu.r[2] = (uint32_t)-10;
    execute_instruction(0x022081b3);
    REQUIRE(cpu.r[3] == (uint32_t)-30);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    MUL x3, x1, x2
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: MUL neg * neg", "[RV32M][Execute][MUL]")
{
    cpu.r[1] = (uint32_t)-4;
    cpu.r[2] = (uint32_t)-8;
    execute_instruction(0x022081b3);
    REQUIRE(cpu.r[3] == 32);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    MULH x3, x1, x2
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: MULH pos * pos (high bits)", "[RV32M][Execute][MULH]")
{
    cpu.r[1] = 3;
    cpu.r[2] = 6;
    execute_instruction(0x022081b3);
    REQUIRE(cpu.r[3] == 32);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    MULH x3, x1, x2
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: MULH pos * neg (high bits)", "[RV32M][Execute][MULH]")
{
    cpu.r[1] = 3;
    cpu.r[2] = (uint32_t)-7;
    execute_instruction(0x022081b3);
    REQUIRE(cpu.r[3] == (uint32_t)-21);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    MULH x3, x1, x2
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: MULH neg * pos (high bits)", "[RV32M][Execute][MULH]")
{
    cpu.r[1] = (uint32_t)-3;
    cpu.r[2] = 7;
    execute_instruction(0x022081b3);
    REQUIRE(cpu.r[3] == (uint32_t)-21);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    MULH x3, x1, x2
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: MULH neg * neg (high bits)", "[RV32M][Execute][MULH]")
{
    cpu.r[1] = (uint32_t)-6;
    cpu.r[2] = (uint32_t)-6;
    execute_instruction(0x022081b3);
    REQUIRE(cpu.r[3] == 36);
    REQUIRE(cpu.pc == 0x80000004);
}