all: instruction instructionng

instruction: instruction.c
	gcc instruction.c -o instruction -lncurses

instructionng: instruction.c
	gcc instruction.c -o instructionng -DNOGRAPHICS

