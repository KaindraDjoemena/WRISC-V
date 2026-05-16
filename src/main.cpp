#include <iostream>

#include "isa.h"
#include "encoding.h"
#include "machine.h"
#include "extensions/extension.h"


struct TestCPUFixture
{
    using TestISA = ISA<RV32I, RV32M>;
    
    Machine<TestISA> vm;

    TestCPUFixture() : vm(0x80000000, 0x1000) {}

    void execute_instruction(uint32_t encoding)
    {
        vm.execute_instruction(encoding);
    }
};

int main()
{
    using MyISA = ISA<RV32I, RV32M, RV32Zicsr, RV32A>;
    Machine<MyISA> vm(0x80000000, 0x100000); // 1MB RAM

    vm.load_binary("riscv-tests/amoadd_w.bin", 0x80000000);
    const uint32_t tohost_addr = 0x80001000;

    while (true)
    {
        uint32_t status = vm.read_word_from_ram(tohost_addr);
        if (status != 0)
        {
            if (status == 1)
            {
                std::cout << ">>> [SUCCESS] RISC-V Compliance Test Passed <<<" << std::endl;
                return 0;
            }
            else
            {
                std::cout << ">>> [FAILURE] Test Case #" << (status >> 1) << " Failed <<<" << std::endl;
                return 1;
            }
        }

        try
        {
            vm.cpu.step();
        }
        catch (const std::exception& e)
        {
            if (vm.read_word_from_ram(tohost_addr) == 1)
            {
                std::cout << ">>> [SUCCESS] RISC-V Compliance Test Passed <<<" << std::endl;
                return 0;
            }
            std::cerr << "[FATAL] CPU Halted: " << e.what() << " at PC 0x" << std::hex << vm.cpu.pc << std::endl;
            return 1;
        }
    }
}