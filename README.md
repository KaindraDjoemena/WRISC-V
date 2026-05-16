# WRISC-V

## Description

a functioning RV32G (unprivileged) implementation -- [RISC-V Instruction Set Manual](https://docs.riscv.org/reference/isa/unpriv/unpriv-index.html)

## Status

| Extension | Impl | Test |
| --- | --- | --- |
| I | v | v |
| M | v | v |
| A | X | X |
| F | X | X |
| D | X | X |
| Zicsr | X | X |
| Zifencei | v | X |
| C | X | X |

## Testing

Test suite: https://github.com/riscv-software-src/riscv-tests

```bash
cd riscv-tests/

riscv64-unknown-elf-gcc -march=<arch> -mabi=ilp32 -static -mcmodel=medany \
-fvisibility=hidden -nostdlib -nostartfiles \
-I./env/p -I./isa/macros/scalar \
-T./env/p/link.ld ./isa/<isa>/<file>.S \
-o <file>.elf

riscv64-unknown-elf-objcopy -O binary <file>.elf <file>.bin
```

```bash
cd riscv-tests/

riscv64-unknown-elf-objdump -d elf/<file>.elf
```