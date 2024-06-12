AS=	riscv32-elf-as
LD=	riscv32-elf-ld

kernel.elf:	l.o kernel.o kernel.ld
	$(LD) -Tkernel.ld -o $@ l.o kernel.o
bc:	bc.c
	cc -ansi -Wall -o $@ bc.c
%.s:	%.b bc
	./bc < $< > $@
clean:
	rm -f *.o bc kernel.elf
.PHONY: clean
