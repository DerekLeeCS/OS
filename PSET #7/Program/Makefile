all: main a.out

main:
	gcc -o main main.c

a.out: asm.o
	ld -m elf_x86_64 asm.o

asm.o:
	as --64 -o asm.o asm.S
