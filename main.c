// Patrick Sherbondy
// COP 3402, Spring 2020
// Worked alone.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

// PLEASE NOTE: I realize that callsym, procsym, and elsesym are contained in this string array.
// I do not utilize them in anyway. They are necessary because of the standard that was set for
// these token types in assignment 2. If I got rid of these three symbols here, then the index
// system used to identify these tokens would be incorrect.
const char *tokenTypes[] = {"n/a", "nulsym", "identsym", "numbersym", "plussym", "minussym",
"multsym",  "slashsym", "oddsym", "eqsym", "neqsym", "lessym", "leqsym",
"gtrsym", "geqsym", "lparentsym", "rparentsym", "commasym", "semicolonsym",
"periodsym", "becomessym", "beginsym", "endsym", "ifsym", "thensym",
"whilesym", "dosym", "callsym", "constsym", "varsym", "procsym",
"writesym", "readsym", "elsesym"};

// If an error is encountered, exit the entire program.
void checkErrors(int producedError)
{
	if (producedError == 1)
	{
		exit(1);
	}
}

// The driving force of the tiny PL/0 compiler.
int main(int argc, char *argv[])
{
	FILE *ifp;

	char *filename = argv[1];
	char buffer[1024];
	char readChar;
	int printLexList = 0, printAssemblyCode = 0, printVMTrace = 0;
	int currentToken, i;
	int producedError;

	// Ensure the user has provided the bare minimum requirement for number of arguments.
	if (argc < 2)
	{
		printf("ERROR: Not enough arguments.\n");
		printf("Correct format: ./compile <input file name and extension> <compiler directives>\n");
		printf("Valid compiler directives:\n");
		printf("	-l to print a list of tokens\n");
		printf("	-a to print the generated assembly code\n");
		printf("	-v to print the virtual machine execution trace\n");

		return 1;
	}

	// Ensure the user has provided an input file. The minimum size for an input file would be 5
	// characters, i.e. "a.txt", so if the user incorrectly did "./a.out -l", this would catch that
	// there is a missing argument.
	if (strlen(argv[1]) < 5)
	{
		printf("ERROR: No input file found.\n");
		printf("Correct format: ./compile <input file name and extension> <compiler directives>\n");
		printf("Valid compiler directives:\n");
		printf("	-l to print a list of tokens\n");
		printf("	-a to print the generated assembly code\n");
		printf("	-v to print the virtual machine execution trace\n");

		return 1;
	}

	// Loop through to find any and all compiler directives.
	while (argc > 1)
	{
		if (strcmp(argv[argc - 1], "-l") == 0)
		{
			printLexList = 1;
		}

		if (strcmp(argv[argc - 1], "-a") == 0)
		{
			printAssemblyCode = 1;
		}

		if (strcmp(argv[argc - 1], "-v") == 0)
		{
			printVMTrace = 1;
		}

		argc--;
	}

	// Run the lexical analyzer with the provided input file.
	producedError = lexicalAnalyzer(filename);

	// Ensure no errors were produced by the lexical analyzer.
	checkErrors(producedError);

	// If the user wants to see the lexeme token list, this will print its internal and symbolic
	// representations.
	if (printLexList == 1)
	{
		ifp = fopen("token-stream.txt", "r");

		// Ensure the file was opened correctly.
		if (ifp == NULL)
		{
			printf("ERROR: Could not open token-stream.txt\n");

			return 1;
		}

		printf("Internal Representation of Token File:\n\n");

		readChar = fgetc(ifp);

		// Print the internal representation of the token file.
		while (readChar != EOF)
		{
			printf("%c", readChar);
			readChar = fgetc(ifp);
		}

		printf("\n\n");

		rewind(ifp);

		printf("Symbolic Representation of Token File:\n\n");

		// Print the symbolic representation of the token file.
		while (fscanf(ifp, "%s", buffer) != EOF)
		{
			// Handles identifiers and numbers, ensuring that the names and values of each are
			// printed correctly in the terminal.
			if (strlen(buffer) == 1 && buffer[0] == '2')
			{
				fscanf(ifp, "%s", buffer);
				printf("identsym %s ", buffer);
			}
			else if (strlen(buffer) == 1 && buffer[0] == '3')
			{
				fscanf(ifp, "%s", buffer);
				printf("numbersym %s ", buffer);
			}
			else
			{
				printf("%s ", tokenTypes[atoi(buffer)]);
			}
		}

		printf("\n\n");

		fclose(ifp);
	}

	// Run the parser / code generator.
	producedError = parserCodeGenerator();

	// Produce the print message ensuring no errors were encountered.
	if (producedError == 0 && printAssemblyCode == 1)
	{
		printf("No errors, program is syntactically correct.\n");
	}

	// Print out the generated assembly code.
	if (printAssemblyCode == 1)
	{
		printf("\n");

		ifp = fopen("generated-assembly-code.txt", "r");

		// Ensure the file was opened successfully.
		if (ifp == NULL)
		{
			printf("ERROR: Could not open token-stream.txt\n");

			return 1;
		}

		printf("Generated Assembly Code:\n\n");

		readChar = fgetc(ifp);

		// Prints out the contents of the output file created by the parser / code-generator.
		while (readChar != EOF)
		{
			printf("%c", readChar);
			readChar = fgetc(ifp);
		}

		printf("\n");

		fclose(ifp);
	}

	// Run the virtual machine.
	producedError = virtualMachine();

	// Ensure the virtual machine produced no errors.
	checkErrors(producedError);

	// Print out the contents of the file produced by the virtual machine.
	if (printVMTrace == 1)
	{
		ifp = fopen("virtual-machine-output.txt", "r");

		// Ensure the file opened successfully.
		if (ifp == NULL)
		{
			printf("ERROR: Could not open token-stream.txt\n");

			return 1;
		}

		readChar = fgetc(ifp);

		// Print out the contents of the file.
		while (readChar != EOF)
		{
			printf("%c", readChar);
			readChar = fgetc(ifp);
		}

		fclose(ifp);
	}

	return 0;
}
