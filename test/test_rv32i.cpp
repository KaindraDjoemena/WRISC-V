#include <catch2/catch_all.hpp>
#include "cpu.h"
#include "bus.h"
#include "ram.h"
#include "isa.h"
#include "encoding.h"
#include "extensions/extension.h"


struct TestCPUFixture {
    Bus bus;
    RAM ram;
    CPU<RV32I> cpu;

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
    ADDI x3, x1, 5
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: ADDI execution (x3 = x1 + 5)", "[RV32I][Execute][ADDI]")
{
    cpu.r[1] = 10;
    execute_instruction(0x00508193);
    REQUIRE(cpu.r[3] == 15);
    REQUIRE(cpu.r[0] == 0);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    ADDI x1, x2, -10
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: ADDI execution (x1 = x2 - 10, signed imm)", "[RV32I][Execute][ADDI]")
{
    cpu.r[2] = 20;
    execute_instruction(0xff610093);
    REQUIRE(cpu.r[1] == 10);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    ADDI x1, x1, 1
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: ADDI execution wraps on overflow", "[RV32I][Execute][ADDI]")
{
    cpu.r[1] = 0xFFFFFFFF;
    execute_instruction(0x00108093);
    REQUIRE(cpu.r[1] == 0x00000000);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    ADDI x0, x0, 0
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: NOP execution (ADDI x0, x0, 0)", "[RV32I][Execute][NOP]")
{
    cpu.r[1] = 5;
    execute_instruction(0x00000013);
    REQUIRE(cpu.r[0] == 0);
    REQUIRE(cpu.r[1] == 5);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    ADDI x0, x1, 1  — rd = x0
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: Write to x0 is ignored", "[RV32I][Execute][x0]")
{
    cpu.r[1] = 42;
    execute_instruction(0x00108013);
    REQUIRE(cpu.r[0] == 0);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    LUI x1, 0x12345
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LUI execution", "[RV32I][Execute][LUI]")
{
    execute_instruction(0x123450b7);
    REQUIRE(cpu.r[1] == 0x12345000);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    LUI x1, 1
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LUI execution clears lower 12 bits", "[RV32I][Execute][LUI]")
{
    cpu.r[1] = 0xFFFFFFFF;
    execute_instruction(0x000010b7);
    REQUIRE(cpu.r[1] == 0x00001000);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    AUIPC x1, 0x10000
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: AUIPC execution", "[RV32I][Execute][AUIPC]")
{
    cpu.pc = 0x80000000;
    execute_instruction(0x10000097);
    REQUIRE(cpu.r[1] == (0x80000000 + 0x10000000));
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    AUIPC x1, 1
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: AUIPC execution at non-base PC", "[RV32I][Execute][AUIPC]")
{
    cpu.pc = 0x80000100;
    execute_instruction(0x00001097);
    REQUIRE(cpu.r[1] == 0x80001100);
    REQUIRE(cpu.pc == 0x80000104);
}

/*
    JAL x1, 20
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: JAL execution (forward)", "[RV32I][Execute][JAL]")
{
    cpu.pc = 0x80000000;
    execute_instruction(0x014000ef);
    REQUIRE(cpu.r[1] == 0x80000004);
    REQUIRE(cpu.pc == 0x80000014);
}

/*
    JAL x1, -8
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: JAL execution (backward)", "[RV32I][Execute][JAL]")
{
    cpu.pc = 0x80000010;
    execute_instruction(0xff9ff0ef);
    REQUIRE(cpu.r[1] == 0x80000014);
    REQUIRE(cpu.pc == 0x80000008);
}

/*
    JALR x1, 8(x2)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: JALR execution", "[RV32I][Execute][JALR]")
{
    cpu.pc = 0x80000000;
    cpu.r[2] = 0x80000100;
    execute_instruction(0x008100E7);
    REQUIRE(cpu.r[1] == 0x80000004);
    REQUIRE(cpu.pc == 0x80000108);
}

/*
    BEQ x1, x2, 12
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BEQ execution (taken)", "[RV32I][Execute][BEQ]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = 10;
    execute_instruction(0x00208663);
    REQUIRE(cpu.pc == 0x8000000C);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: BEQ execution (not taken)", "[RV32I][Execute][BEQ]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = 20;
    execute_instruction(0x00208663);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    BNE x1, x2, 8
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BNE execution (taken)", "[RV32I][Execute][BNE]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = 20;
    execute_instruction(0x00209463);
    REQUIRE(cpu.pc == 0x80000008);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BNE execution (not taken)", "[RV32I][Execute][BNE]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = 10;
    execute_instruction(0x00209463);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    BLT x1, x2, 8
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BLT execution (taken, signed)", "[RV32I][Execute][BLT]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = (uint32_t)-10;
    cpu.r[2] = 5;
    execute_instruction(0x0020C663);
    REQUIRE(cpu.pc == 0x8000000c);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BLT execution (not taken, signed)", "[RV32I][Execute][BLT]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = (uint32_t)-5;
    execute_instruction(0x0020C663);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    BGE x1, x2, 8
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BGE execution (taken, signed)", "[RV32I][Execute][BGE]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = 5;
    execute_instruction(0x0020d463);
    REQUIRE(cpu.pc == 0x80000008);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BGE execution (equal, taken)", "[RV32I][Execute][BGE]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 5;
    cpu.r[2] = 5;
    execute_instruction(0x0020d463);
    REQUIRE(cpu.pc == 0x80000008);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BGE execution (not taken, signed)", "[RV32I][Execute][BGE]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = (uint32_t)-10;
    cpu.r[2] = 5;
    execute_instruction(0x0020d463);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    BLTU x1, x2, 8
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BLTU execution (taken, unsigned)", "[RV32I][Execute][BLTU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 5;
    cpu.r[2] = 10;
    execute_instruction(0x0020e463);
    REQUIRE(cpu.pc == 0x80000008);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BLTU execution (not taken, unsigned)", "[RV32I][Execute][BLTU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = 5;
    execute_instruction(0x0020e463);
    REQUIRE(cpu.pc == 0x80000004);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BLTU treats high bit as unsigned (not taken)", "[RV32I][Execute][BLTU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0xFFFFFFF6;
    cpu.r[2] = 5;
    execute_instruction(0x0020e463);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    BGEU x1, x2, 12
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BGEU execution (taken, unsigned)", "[RV32I][Execute][BGEU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 10;
    cpu.r[2] = 5;
    execute_instruction(0x0020F663);
    REQUIRE(cpu.pc == 0x8000000c);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BGEU execution (equal, taken)", "[RV32I][Execute][BGEU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 5;
    cpu.r[2] = 5;
    execute_instruction(0x0020F663);
    REQUIRE(cpu.pc == 0x8000000c);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BGEU execution (not taken, unsigned)", "[RV32I][Execute][BGEU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 5;
    cpu.r[2] = 10;
    execute_instruction(0x0020F663);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    BEQ x1, x2, -8
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: BEQ backward branch (taken)", "[RV32I][Execute][BEQ]")
{
    cpu.pc = 0x80000010;
    cpu.r[1] = 7;
    cpu.r[2] = 7;
    execute_instruction(0xFE208CE3);
    REQUIRE(cpu.pc == 0x80000008);
}

/*
    LW x2, 4(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LW execution", "[RV32I][Execute][LW]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    write_word_to_ram(cpu.r[1] + 4, 0xDEADBEEF);

    execute_instruction(0x0040A103);
    REQUIRE(cpu.r[2] == 0xDEADBEEF);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    LB x2, 0(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LB execution (sign-extended negative)", "[RV32I][Execute][LB]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    write_byte_to_ram(cpu.r[1], 0xFF);
    write_byte_to_ram(cpu.r[1] + 1, 0x01);

    execute_instruction(0x00008103);
    REQUIRE(cpu.r[2] == (uint32_t)-1);
    REQUIRE(cpu.pc == 0x80000004);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LB execution (sign-extended positive)", "[RV32I][Execute][LB]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    write_byte_to_ram(cpu.r[1], 0x7F);
    execute_instruction(0x00008103);
    REQUIRE(cpu.r[2] == 127);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    LBU x2, 0(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LBU execution (zero-extended)", "[RV32I][Execute][LBU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    write_byte_to_ram(cpu.r[1], 0xFF);
    write_byte_to_ram(cpu.r[1] + 1, 0x01);
    execute_instruction(0x0000c103);
    REQUIRE(cpu.r[2] == 0xFF);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    LH x2, 0(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LH execution (sign-extended negative)", "[RV32I][Execute][LH]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    write_half_to_ram(cpu.r[1], 0xFFFF);
    write_word_to_ram(cpu.r[1] + 2, 0x01010101);
    execute_instruction(0x00009103);
    REQUIRE(cpu.r[2] == (uint32_t)-1);
    REQUIRE(cpu.pc == 0x80000004);
}
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LH execution (sign-extended positive)", "[RV32I][Execute][LH]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    write_half_to_ram(cpu.r[1], 0x7FFF);
    execute_instruction(0x00009103);
    REQUIRE(cpu.r[2] == 32767);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    LHU x2, 0(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: LHU execution (zero-extended)", "[RV32I][Execute][LHU]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    write_half_to_ram(cpu.r[1], 0xFFFF);
    write_word_to_ram(cpu.r[1] + 2, 0x01010101);
    execute_instruction(0x0000d103);
    REQUIRE(cpu.r[2] == 0xFFFF);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    SW x2, 8(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: SW execution", "[RV32I][Execute][SW]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000020;
    cpu.r[2] = 0xCAFEBABE;
    execute_instruction(0x0020a423);
    REQUIRE(read_word_from_ram(cpu.r[1] + 8) == 0xCAFEBABE);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    SB x2, 0(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: SB execution (stores low byte only)", "[RV32I][Execute][SB]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    cpu.r[2] = 0xDEADBEEF;
    execute_instruction(0x00208023);
    REQUIRE(read_byte_from_ram(cpu.r[1]) == 0xEF);
    REQUIRE(read_byte_from_ram(cpu.r[1] + 1) == 0x00);
    REQUIRE(cpu.pc == 0x80000004);
}

/*
    SH x2, 0(x1)
*/
TEST_CASE_METHOD(TestCPUFixture, "RV32I: SH execution (stores low halfword only)", "[RV32I][Execute][SH]")
{
    cpu.pc = 0x80000000;
    cpu.r[1] = 0x80000010;
    cpu.r[2] = 0xDEADBEEF;
    execute_instruction(0x00209023);
    REQUIRE(read_half_from_ram(cpu.r[1]) == 0xBEEF);
    REQUIRE(read_byte_from_ram(cpu.r[1] + 2) == 0x00);
    REQUIRE(cpu.pc == 0x80000004);
}


// ============================================================
// I-Type ALU
// ============================================================

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLTI execution (x1 < 5, signed)", "[RV32I][Execute][SLTI]")
{
    cpu.r[1] = 3;
    execute_instruction(0x0050a193); // SLTI x3, x1, 5
    REQUIRE(cpu.r[3] == 1);
    REQUIRE(cpu.pc == 0x80000004);

    cpu.r[1] = 10;
    execute_instruction(0x0050a193);
    REQUIRE(cpu.r[3] == 0);
    REQUIRE(cpu.pc == 0x80000008);

    cpu.r[1] = (uint32_t)-10;
    execute_instruction(0x0050a193);
    REQUIRE(cpu.r[3] == 1); // -10 < 5 signed
    REQUIRE(cpu.pc == 0x8000000C);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLTIU execution (x1 < 5, unsigned)", "[RV32I][Execute][SLTIU]")
{
    cpu.r[1] = 3;
    execute_instruction(0x0050b193); // SLTIU x3, x1, 5
    REQUIRE(cpu.r[3] == 1);
    REQUIRE(cpu.pc == 0x80000004);

    cpu.r[1] = 10;
    execute_instruction(0x0050b193);
    REQUIRE(cpu.r[3] == 0);
    REQUIRE(cpu.pc == 0x80000008);

    cpu.r[1] = 0xFFFFFFF6; // large unsigned value
    execute_instruction(0x0050b193);
    REQUIRE(cpu.r[3] == 0); // 0xFFFFFFF6 is NOT < 5 unsigned
    REQUIRE(cpu.pc == 0x8000000C);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: XORI execution (signed imm -1, flip all bits)", "[RV32I][Execute][XORI]")
{
    cpu.r[1] = 0xF0F0F0F0;
    execute_instruction(0xfff0c113); // XORI x2, x1, -1
    REQUIRE(cpu.r[2] == 0x0F0F0F0F);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: ANDI execution (x3 = x1 & 0xF)", "[RV32I][Execute][ANDI]")
{
    cpu.r[1] = 0xFFF000FF;
    execute_instruction(0x00F0f193); // ANDI x3, x1, 0xF
    REQUIRE(cpu.r[3] == 0x0000000F);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: ORI execution (x3 = x1 | 0xF)", "[RV32I][Execute][ORI]")
{
    cpu.r[1] = 0x0000000A;
    execute_instruction(0x00F0e193); // ORI x3, x1, 0xF
    REQUIRE(cpu.r[3] == 0x0000000F);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLLI execution (x3 = x1 << 2)", "[RV32I][Execute][SLLI]")
{
    cpu.r[1] = 0x0000000A;
    execute_instruction(0x00209193); // SLLI x3, x1, 2
    REQUIRE(cpu.r[3] == 0x00000028);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SRLI execution (x3 = x1 >> 2, logical)", "[RV32I][Execute][SRLI]")
{
    cpu.r[1] = 0x8000000C;
    execute_instruction(0x0020d193); // SRLI x3, x1, 2
    REQUIRE(cpu.r[3] == 0x20000003);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SRAI execution (x3 = x1 >> 2, arithmetic)", "[RV32I][Execute][SRAI]")
{
    cpu.r[1] = 0x8000000C;
    execute_instruction(0x4020d193); // SRAI x3, x1, 2
    REQUIRE(cpu.r[3] == 0xE0000003);
    REQUIRE(cpu.pc == 0x80000004);
}


// ============================================================
// R-Type ALU
// ============================================================

TEST_CASE_METHOD(TestCPUFixture, "RV32I: ADD execution (x3 = x1 + x2)", "[RV32I][Execute][ADD]")
{
    cpu.r[1] = 10;
    cpu.r[2] = 20;
    execute_instruction(0x002081b3); // ADD x3, x1, x2
    REQUIRE(cpu.r[3] == 30);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: ADD execution wraps on overflow", "[RV32I][Execute][ADD]")
{
    cpu.r[1] = 0xFFFFFFFF;
    cpu.r[2] = 1;
    execute_instruction(0x002081b3);
    REQUIRE(cpu.r[3] == 0x00000000);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SUB execution (x3 = x1 - x2)", "[RV32I][Execute][SUB]")
{
    cpu.r[1] = 30;
    cpu.r[2] = 10;
    execute_instruction(0x402081b3); // SUB x3, x1, x2
    REQUIRE(cpu.r[3] == 20);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SUB execution wraps on underflow", "[RV32I][Execute][SUB]")
{
    cpu.r[1] = 0;
    cpu.r[2] = 1;
    execute_instruction(0x402081b3);
    REQUIRE(cpu.r[3] == 0xFFFFFFFF);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: XOR execution", "[RV32I][Execute][XOR]")
{
    cpu.r[1] = 0xF0F0F0F0;
    cpu.r[2] = 0x0F0F0F0F;
    execute_instruction(0x0020C1B3); // XOR x3, x1, x2
    REQUIRE(cpu.r[3] == 0xFFFFFFFF);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: OR execution", "[RV32I][Execute][OR]")
{
    cpu.r[1] = 0x0000F0F0;
    cpu.r[2] = 0x00000F0F;
    execute_instruction(0x0020E1B3); // OR x3, x1, x2
    REQUIRE(cpu.r[3] == 0x0000FFFF);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: AND execution", "[RV32I][Execute][AND]")
{
    cpu.r[1] = 0xFF00FF00;
    cpu.r[2] = 0xF0F0F0F0;
    execute_instruction(0x0020F1B3); // AND x3, x1, x2
    REQUIRE(cpu.r[3] == 0xF000F000);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLT execution (signed, x1 < x2)", "[RV32I][Execute][SLT]")
{
    cpu.r[1] = (uint32_t)-10;
    cpu.r[2] = 5;
    execute_instruction(0x0020a1b3); // SLT x3, x1, x2
    REQUIRE(cpu.r[3] == 1);
    REQUIRE(cpu.pc == 0x80000004);

    cpu.r[1] = 10;
    cpu.r[2] = (uint32_t)-5;
    execute_instruction(0x0020a1b3);
    REQUIRE(cpu.r[3] == 0);
    REQUIRE(cpu.pc == 0x80000008);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLT execution (equal, returns 0)", "[RV32I][Execute][SLT]")
{
    cpu.r[1] = 5;
    cpu.r[2] = 5;
    execute_instruction(0x0020a1b3);
    REQUIRE(cpu.r[3] == 0);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLTU execution (unsigned, x1 < x2)", "[RV32I][Execute][SLTU]")
{
    // 0xFFFFFFF6 > 5 unsigned — result should be 0
    cpu.r[1] = 0xFFFFFFF6;
    cpu.r[2] = 5;
    execute_instruction(0x0020B1B3); // SLTU x3, x1, x2
    REQUIRE(cpu.r[3] == 0);
    REQUIRE(cpu.pc == 0x80000004);

    // 5 < 0xFFFFFFF6 unsigned — result should be 1
    cpu.r[1] = 5;
    cpu.r[2] = 0xFFFFFFF6;
    execute_instruction(0x0020B1B3);
    REQUIRE(cpu.r[3] == 1);
    REQUIRE(cpu.pc == 0x80000008);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLL execution (x3 = x1 << x2)", "[RV32I][Execute][SLL]")
{
    cpu.r[1] = 0x0000000A;
    cpu.r[2] = 3;
    execute_instruction(0x002091B3); // SLL x3, x1, x2
    REQUIRE(cpu.r[3] == 0x00000050);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SLL uses only low 5 bits of shift amount", "[RV32I][Execute][SLL]")
{
    cpu.r[1] = 1;
    cpu.r[2] = 0x23; // only low 5 bits = 3
    execute_instruction(0x002091B3);
    REQUIRE(cpu.r[3] == 8); // 1 << 3
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SRL execution (x3 = x1 >> x2, logical)", "[RV32I][Execute][SRL]")
{
    cpu.r[1] = 0x8000000C;
    cpu.r[2] = 2;
    execute_instruction(0x0020D1B3); // SRL x3, x1, x2
    REQUIRE(cpu.r[3] == 0x20000003);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SRA execution (x3 = x1 >> x2, arithmetic)", "[RV32I][Execute][SRA]")
{
    cpu.r[1] = 0x8000000C;
    cpu.r[2] = 2;
    execute_instruction(0x4020D1B3); // SRA x3, x1, x2
    REQUIRE(cpu.r[3] == 0xE0000003);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: SRA preserves sign on positive value", "[RV32I][Execute][SRA]")
{
    cpu.r[1] = 0x40000008; // positive
    cpu.r[2] = 2;
    execute_instruction(0x4020D1B3);
    REQUIRE(cpu.r[3] == 0x10000002); // no sign extension
    REQUIRE(cpu.pc == 0x80000004);
}


// ============================================================
// System / Fence Instructions
// ============================================================

TEST_CASE_METHOD(TestCPUFixture, "RV32I: FENCE execution (NOP, advances PC)", "[RV32I][Execute][FENCE]")
{
    cpu.pc = 0x80000000;
    execute_instruction(0x0000000F);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: FENCE.TSO execution (NOP, advances PC)", "[RV32I][Execute][FENCE_TSO]")
{
    cpu.pc = 0x80000000;
    execute_instruction(0x0800000F);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: PAUSE execution (NOP, advances PC)", "[RV32I][Execute][PAUSE]")
{
    cpu.pc = 0x80000000;
    execute_instruction(0x0100000F);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: ECALL execution (advances PC)", "[RV32I][Execute][ECALL]")
{
    cpu.pc = 0x80000000;
    execute_instruction(0x00000073);
    REQUIRE(cpu.pc == 0x80000004);
}

TEST_CASE_METHOD(TestCPUFixture, "RV32I: EBREAK execution (advances PC)", "[RV32I][Execute][EBREAK]")
{
    cpu.pc = 0x80000000;
    execute_instruction(0x00100073);
    REQUIRE(cpu.pc == 0x80000004);
}