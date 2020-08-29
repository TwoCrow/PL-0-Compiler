// Patrick Sherbondy
// COP 3402, Spring 2020
// Worked alone.

#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"

// This function, provided by Prof. Montagne, finds a varaible in a different activation record
// l levels down.
int base(int l, int *BP, int *stack)
{
	int bl = *BP;

	while (l > 0)
	{
		bl = stack[bl + l];
		l--;
	}

	return bl;
}

// This function prints the contents of the register file to an output file.
void printRegisterFileContents(FILE *ofp, int *regFile)
{
	int i = 0;

	while (i < 8)
	{
		fprintf(ofp, "%d ", regFile[i]);

		i++;
	}

	fprintf(ofp, "\n");

	return;
}

// Prints out the contents of the stack to the amount specified by maxPrintLen. For instance,
// if maxPrintLen = 40, the whole stack is printed. If maxPrintLen = 6, only the first 6 entries
// are printed, for example.
void printStackContents(FILE *ofp, int *stack, int maxPrintLen, int showAR, int level, int *BP)
{
	int SP = maxPrintLen;
	int i = 1;
	fprintf(ofp, "Stack: ");

	if (maxPrintLen <= 0)
	{
		return;
	}

	// Loop through the stack until either the max stack height is reached, or the contents of the
	// active stack are printed.
	while (i < MAX_STACK_HEIGHT && maxPrintLen > 0)
	{
		fprintf(ofp, "%d ", stack[i]);

		// If the activation record is being shown, it is necessary to check whether the base pointer
		// is equal to the current iteration + 1, there is more than 1 lexicographical level, and that
		// the stack pointer is greater than the base pointer. All of this ensures that the bars are
		// printed in such a way that each activation record is distinct.
		if (showAR == 1 && *BP == i + 1 && level >= 1 && SP > *BP)
		{
			fprintf(ofp, "| ");
		}

		maxPrintLen--;
		i++;
	}
}

// This function takes in the IR array and breaks it down into its various arguments. It then
// manipulates the stack and register file based on the instructions found within the given value
// in the IR array.
// This function returns 1 if the program should stop, and 0 otherwise.
int executeInstruction(instruction *IR, int *stack, int *regFile, int *PC, int *BP, int *SP,
											 int *level, int *len)
{
	// Creating new variables for the sake of creating shorter lines and allowing speedier use of
	// these variables.
	int op = IR[*PC - 1].op;
	int r  = IR[*PC - 1].r;
	int l  = IR[*PC - 1].l;
	int m  = IR[*PC - 1].m;

	// LIT R, 0, M
	if (op == 1)
	{
		regFile[r] = m;
	}
	// RTN 0, 0, 0
	else if (op == 2)
	{
		*level = *level - 1;
		*SP = *BP - 1;
		*BP = stack[*SP + 3];
		*PC = stack[*SP + 4];
	}
	// LOD R, L, M
	else if (op == 3)
	{
		regFile[r] = stack[base(l, BP, stack) + m];
	}
	// STO R, L, M
	else if (op == 4)
	{
		stack[base(l, BP, stack) + m] = regFile[r];
	}
	// CAL 0, L, M
	else if (op == 5)
	{
		stack[*SP + 1] = 0;
		stack[*SP + 2] = base(l, BP, stack);
		stack[*SP + 3] = *BP;
		stack[*SP + 4] = *PC;

		*level = *level + 1;
		*BP = *SP + 1;
		*PC = m;
	}
	// INC 0, 0, M
	else if (op == 6)
	{
		*SP = *SP + m;
		*len = *len + m;
	}
	// JMP 0, 0, M
	else if (op == 7)
	{
		*PC = m;
	}
	// JPC R, 0, M
	else if (op == 8)
	{
		if (regFile[r] == 0)
		{
			*PC = m;
		}
	}
	// SIO R, 0, 1
	else if (op == 9)
	{
		printf("%d\n", regFile[r]);
	}
	// SIO R, 0, 2
	else if (op == 10)
	{
		scanf("%d", &regFile[r]);
	}
	// SIO R, 0, 3
	else if (op == 11)
	{
		return 1;
	}
	// NEG
	else if (op == 12)
	{
		regFile[r] = regFile[r] * -1;
	}
	// ADD
	else if (op == 13)
	{
		regFile[r] = regFile[l] + regFile[m];
	}
	// SUB
	else if (op == 14)
	{
		regFile[r] = regFile[l] - regFile[m];
	}
	// MUL
	else if (op == 15)
	{
		regFile[r] = regFile[l] * regFile[m];
	}
	// DIV
	else if (op == 16)
	{
		regFile[r] = regFile[l] / regFile[m];
	}
	// ODD
	else if (op == 17)
	{
		regFile[r] = regFile[r] % 2;
	}
	// MOD
	else if (op == 18)
	{
		regFile[r] = regFile[l] % regFile[m];
	}
	// EQL
	else if (op == 19)
	{
		regFile[r] = (regFile[l] == regFile[m]);
	}
	// NEQ
	else if (op == 20)
	{
		regFile[r] = (regFile[l] != regFile[m]);
	}
	// LSS
	else if (op == 21)
	{
		regFile[r] = (regFile[l] < regFile[m]);
	}
	// LEQ
	else if (op == 22)
	{
		regFile[r] = (regFile[l] <= regFile[m]);
	}
	// GTR
	else if (op == 23)
	{
		regFile[r] = (regFile[l] > regFile[m]);
	}
	// GEQ
	else if (op == 24)
	{
		regFile[r] = (regFile[l] >= regFile[m]);
	}
	// Invalid OP code
	else
	{
		printf("ERROR: Invalid OP code.\n");

		return 1;
	}

	return 0;
}

int virtualMachine(void)
{
	// Open the input file whose name is passed at the command line.
	FILE *ifp = fopen("generated-assembly-code.txt", "r");
	FILE *ofp = fopen("virtual-machine-output.txt", "w");

	// Ensure the file opened successfully.
	if (ifp == NULL)
	{
		printf("ERROR: File could not be opened Check that you spelled the filename correctly, and\n");
		printf("that the file is in the same directory as virtual-machine.c\n");

		return 1;
	}

	// Ensure the output file was successfully created / overwritten.
	if (ofp == NULL)
	{
		printf("ERROR: Could not create or overwrite output files.\n");

		return 1;
	}

	// Set up the initial values for the registers.
	int SP = 0;
	int BP = 1;
	int PC = 0;
	instruction IR[MAX_CODE_LENGTH];

	// This array allows for easy conversion between the opcode numbers and its string counterparts.
	// NOTE: "nul" is a stand-in for an opcode that is not recognized by the machine.
	char *opcodes[] = {"nul", "lit", "rtn", "lod", "sto", "cal", "inc", "jmp", "jpc", "sio",
										 "sio", "sio", "neg", "add", "sub", "mul", "div", "odd", "mod", "eql",
										 "neq", "lss", "leq", "gtr", "geq"};

	// Flag that determines whether the program should stop.
	int haltProgram = 0;
	// Used to help print out the correct amount of activation records.
	int level = 0;
	int len = 0;
	int temp = 0;

	// Initialize the stack and register file with zeroes.
	int stack[MAX_STACK_HEIGHT] = {0};
	int regFile[8] = {0};

	fprintf(ofp, "Your Program In Interpreted Assembly Language:\n\n");
	fprintf(ofp, "Line  OP   R  L  M\n");

	// Simulates the Fetch Cycle by retrieving all the isntructions from the text file and placing
	// them in an array of structs.
	while (fscanf(ifp, "%d %d %d %d", &IR[PC].op, &IR[PC].r, &IR[PC].l, &IR[PC].m) != EOF)
	{
		fprintf(ofp, "%2d    %s  %d  %d  %d\n", PC, opcodes[IR[PC].op], IR[PC].r, IR[PC].l, IR[PC].m);

		PC++;

		// Ensure the program doesn't go over the maximum code length.
		if (PC > MAX_CODE_LENGTH)
		{
			printf("ERROR: Too many lines of code. Maximum code length is %d.\n", MAX_CODE_LENGTH);

			return 1;
		}
	}

	fprintf(ofp, "\n");
	fprintf(ofp, "Execution of Your Program In the Virtual Machine:\n\n");

	// Reset the program counter for use in the execution cycle.
	PC = 0;

	fprintf(ofp, "                 pc   bp   sp  registers\n");
	fprintf(ofp, "Initial values   %2d   %2d   %2d  ", PC, BP, SP);

	// Print the inital values of the register file and the stack.
	printRegisterFileContents(ofp, regFile);
	printStackContents(ofp, stack, MAX_STACK_HEIGHT, 0, level, &BP);
	fprintf(ofp, "\n\n");

	fprintf(ofp, "                    pc   bp   sp   registers\n");

	// Simulates the Execute Cycle by taking instructions from the IR struct array, and calling
	// the executeInstruction() function to perform the commands accordingly.
	while (haltProgram != 1 || PC > MAX_CODE_LENGTH)
	{
		fprintf(ofp, "%3d %s %2d %2d %2d  ", PC, opcodes[IR[PC].op], IR[PC].r, IR[PC].l, IR[PC].m);

		// Increase the program counter and execute the instruction.
		PC++;
		haltProgram = executeInstruction(IR, stack, regFile, &PC, &BP, &SP, &level, &len);

		// Print the register file.
		fprintf(ofp, "  %2d   %2d   %2d   ", PC, BP, SP);
		printRegisterFileContents(ofp, regFile);

		// Print the stack.
		temp = SP;
		printStackContents(ofp, stack, temp, 1, level, &BP);
		fprintf(ofp, "\n\n");
	}

	// Ensure all open files are closed before returning.
	fclose(ifp);
	fclose(ofp);

	return 0;
}
