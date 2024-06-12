# simple-kernel

A simple RV32 kernel in B (and compiler).

## Build

`riscv32-elf-{as,ld}` as well as `make` are required.

```
make
```

Produces a `kernel.elf` which can be run with QEMU:

```
qemu-system-riscv32 -M virt -kernel kernel.elf -serial stdio
```
