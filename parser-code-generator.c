// Patrick Sherbondy
// COP 3402, Spring 2020
// Worked alone.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "compiler.h"

// Functional Prototypes
// This is necessary since some of these functions call one another consistently, making the order
// of implicit declaration unreliable.
void printErrorMessage(FILE *ofp, int errorCode);
void generateInstruction(instruction *instReg, int op, int r, int l, int m);
void getToken(FILE *tokenStream);
int position(symbol *table);
void insert(symbol *table, FILE *ofp, int kind);
void variableDeclaration(FILE *tokenStream, FILE *ofp, symbol *table);
void constantDeclaration(FILE *tokenStream, FILE *ofp, symbol *table);
void factor(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg);
void term(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg);
void expression(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg);
void condition(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg);
void statement(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg);
void block(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg, int ndxTable);
void program(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg);
int parserCodeGenerator(void);

// Holds the current token the P-CG is evaluating.
int currentToken;
// Contains the various indexes for the output instruction register, the symbol table, and data.
int ndxInstReg, ndxTable, ndxData;
// This is the register pointer, which moves according to which instructions are created for the
// virtual machine.
int ptrReg = 0;
// This is the current lexicographical level.
int currentLevel = 0;
// Holds the value of the number currently being evaluated.
int number;
// Holds the name of the identifier currently being evaluated.
char identifier[12], numberStr[6], buffer[20];

// Contains various, possible error messages. All that needs to be passed is the output file
// pointer, and the error code as an integer.
// PLEASE NOTE: I realize some of these error codes will not be utilized by this program. For
// simplicity sake, I decided to include the entirety of Appendix C here.
void printErrorMessage(FILE *ofp, int errorCode)
{
	printf("*** ERROR CODE %03d ***\n", errorCode);

	if (errorCode == 1)
	{
		printf("Must use = instead of :=\n");
	}
	else if (errorCode == 2)
	{
		printf("= must be followed by a number.\n");
	}
	else if (errorCode == 3)
	{
		printf("Identifier must be followed by =.\n");
	}
	else if (errorCode == 4)
	{
		printf("const, var, and procudedure must be followed by an identifier.\n");
	}
	else if (errorCode == 5)
	{
		printf("Semicolon or comma missing.\n");
	}
	else if (errorCode == 6)
	{
		printf("Incorrect symbol after procedure delcaration.\n");
	}
	else if (errorCode == 7)
	{
		printf("Statement expected.\n");
	}
	else if (errorCode == 8)
	{
		printf("Incorrect symbol after statement part in block.\n");
	}
	else if (errorCode == 9)
	{
		printf("Period expected.\n");
	}
	else if (errorCode == 10)
	{
		printf("Missing semicolon between statements.\n");
	}
	else if (errorCode == 11)
	{
		printf("Undeclared identifier.\n");
	}
	else if (errorCode == 12)
	{
		printf("Assignment to constant or procedure is not allowed.\n");
	}
	else if (errorCode == 13)
	{
		printf("Assignment operator expected.\n");
	}
	else if (errorCode == 14)
	{
		printf("Call must be followed by an identifier.\n");
	}
	else if (errorCode == 15)
	{
		printf("Call of a constant or variable is meaningless.\n");
	}
	else if (errorCode == 16)
	{
		printf("then expected.\n");
	}
	else if (errorCode == 17)
	{
		printf("Semicolon or } expected.\n");
	}
	else if (errorCode == 18)
	{
		printf("do expected.\n");
	}
	else if (errorCode == 19)
	{
		printf("Incorrect symbol following statement.\n");
	}
	else if (errorCode == 20)
	{
		printf("Relational operator expected.\n");
	}
	else if (errorCode == 21)
	{
		printf("Expression must not contain a procedure identifier.\n");
	}
	else if (errorCode == 22)
	{
		printf("Right parenthesis missing.\n");
	}
	else if (errorCode == 23)
	{
		printf("The preceding factor cannot begin with this symbol.\n");
	}
	else if (errorCode == 24)
	{
		printf("An expression cannot begin with this symbol.\n");
	}
	else if (errorCode == 25)
	{
		printf("This number is too large.\n");
	}
  else if (errorCode == 26)
  {
    printf("Input file not found.\n");
  }
  else if (errorCode == 27)
  {
    printf("Output file could not be created.\n");
  }
	else if (errorCode == 28)
	{
		printf("Max symbol table size exceeded.\n");
	}
	else if (errorCode == 29)
	{
		printf("Max lexicographical levels exceeded.\n");
	}

	fclose(ofp);

  exit(errorCode);
}

// Generates an instruction to be sent to the virtual machine by adding it to the instReg
// array of instructions. op is the opcode, r is the register, l is the level, and m is used for
// miscellaneous actions, such as literal numbers or addresses for arithmetic operations.
void generateInstruction(instruction *instReg, int op, int r, int l, int m)
{
  instReg[ndxInstReg].op = op;
  instReg[ndxInstReg].r = r;
  instReg[ndxInstReg].l = l;
  instReg[ndxInstReg].m = m;

  ndxInstReg++;
}

// Reads in the next token from the token stream file produced by the lexical analyzer.
// It sets the currentToken global variable to whatever is read in next.
void getToken(FILE *tokenStream)
{
	fscanf(tokenStream, "%s", buffer);

	currentToken = atoi(buffer);

	// If a 2 is encountered, then the next token needs to be read in to see what to get the
	// name of the identifier.
  if (currentToken == 2)
  {
		fscanf(tokenStream, "%s", identifier);
  }
	// If a 3 is encountered, we need to do a similar thing to the step above, except we then need
	// to convert the string to an integer, since 3 represents a number.
  else if (currentToken == 3)
  {
		fscanf(tokenStream, "%s", numberStr);

		number = atoi(numberStr);
  }

  return;
}

// Find the position of a certain identifier within the symbol table.
// It returns an integer.
int position(symbol *table)
{
	int i = ndxTable;

	// Copy the name of the identifier into the very first position. This prevents the loop below from
	// entering into forbidden memory space behind the array, while also signalling that the identifier
	// is not in the symbol table.
	strcpy(table[0].name, identifier);

	// Loop through the symbol table until the desired identifier is found.
	while (strcmp(table[i].name, identifier) != 0)
	{
		i--;
	}

	// Return the position of the identifier in the symbol table.
	return i;
}

// Inserts a identifier into the symbol table, setting the relevant information of that identifier.
void insert(symbol *table, FILE *ofp, int kind)
{
	// Increase the table index.
	ndxTable++;

	// If the max table size is exceeded, throw an error.
	if (ndxTable > MAX_SYMBOL_TABLE_SIZE)
	{
		printErrorMessage(ofp, 28);
	}

	// Set the first two fields of the identifier to its relevant information.
	strcpy(table[ndxTable].name, identifier);
	table[ndxTable].kind = kind;

	// If we're dealing with a constant, then all that needs to be set is the value.
	if (table[ndxTable].kind == CONSTANT)
	{
		table[ndxTable].val = number;
	}
	// Otherwise, the address fields need to be set.
	else if (table[ndxTable].kind == VARIABLE)
	{
		table[ndxTable].level = 0;
		table[ndxTable].addr = ndxData;
		ndxData++;
	}

}

// Declares variables by inserting them into the symbol table.
void variableDeclaration(FILE *tokenStream, FILE *ofp, symbol *table)
{
	// Ensure we've found an identifier.
	if (currentToken == identsym)
	{
		// 0 is for the current lexicographical level.
		insert(table, ofp, VARIABLE);
		getToken(tokenStream);
	}
	// Otherwise, no identifier was found.
	// const / var / procedure must be followed by an identifier.
	else
	{
		printErrorMessage(ofp, 4);
	}

	return;
}

// Declares constants by inserting them into the symbol table.
void constantDeclaration(FILE *tokenStream, FILE *ofp, symbol *table)
{
	// Ensure we've found an identifier.
	if (currentToken == identsym)
	{
		getToken(tokenStream);

		// If it's become, throw an error.
		// "becomesym" is ':=', which cannot be used for constant declarations. Only '=' is allowed.
		if (currentToken == becomessym)
		{
			printErrorMessage(ofp, 1);

			return;
		}
		// Check that the token after the identifier is an equal symbol.
		else if (currentToken == eqsym)
		{
			getToken(tokenStream);

			// Check that the next token is a number.
			if (currentToken == numbersym)
			{
				// Insert the constant into the symbol table.
				insert(table, ofp, CONSTANT);
			}
			// Otherwise, we've found a constant that is being assigned no value.
			// ":=" must be followed by a number when it comes to constants.
			else
			{
				printErrorMessage(ofp, 2);

				return;
			}
		}
		// Otherwise, the token that came after the constant identifier was not a correct "=", but
		// something else. Throw an error.
		else
		{
			printErrorMessage(ofp, 3);

			return;
		}
	}
	// If we didn't even find an identifier, then state that the word "const" must be followed by
	// a list of identifiers.
	else
	{
		printErrorMessage(ofp, 4);

		return;
	}

	getToken(tokenStream);

	return;
}

// Determines whether a segment of code follows the correct grammar for "factor"
// This function generates instructions.
void factor(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg)
{
  int i;

	// Ensure the factor starts with either an identifier, number, or left parenthesis to follow
	// the grammar rules. If not, throw an error.
	if (currentToken != identsym && currentToken != numbersym && currentToken != lparentsym)
	{
		printErrorMessage(ofp, 23);

		return;
	}

	// Loop until a non-terminating symbol is encountered.
	while (currentToken == identsym || currentToken == numbersym || currentToken == lparentsym)
  {
		// If the current token is an identifier, then we need to determine if it exists and what
		// type it is before generating instructions.
    if (currentToken == identsym)
    {
			// Get the position of the active identifier within the symbol table.
      i = position(table);

			// If i is 0, that means the program looped through the entire symbol table without finding
			// the identifier. State that the identifier was undeclared.
      if (i == 0)
      {
        printErrorMessage(ofp, 11);

				return;
      }
			// Otherwise, we're set to start generating relevant instructions.
      else
      {
				// If the identifier is a constant, use LIT to load a literal value into the register.
        if (table[i].kind == CONSTANT)
        {
          generateInstruction(instReg, lit, ptrReg++, 0, table[i].val);
        }
				// Otherwise, if it's a variable, use LOD to load the value of a variable into the register
				// from the stack.
        else if (table[i].kind == VARIABLE)
        {
          generateInstruction(instReg, lod, ptrReg++, currentLevel - table[i].level, table[i].addr);
        }
				// Otherwise, it is a procedure, and expressions cannot contain procedure identifiers.
				else
				{
					printErrorMessage(ofp, 21);
				}

        getToken(tokenStream);
      }
    }
		// If the current token is a number, ensure it's within bounds and generate an instruction.
    else if (currentToken == numbersym)
    {
			// Ensure the current number is within the size limit for numbers, if not, state that it is
			// too large.
      if (strlen(numberStr) > MAX_NUMBER_LEN)
      {
        printErrorMessage(ofp, 25);

				return;
      }

			// Generate a LIT instruction to load the literal number into the register.
      generateInstruction(instReg, lit, ptrReg++, 0, number);
      getToken(tokenStream);
    }
		// If the current token is a left parentheses, we've encountered an expression that needs to
		// be processed first according to the order of operations.
    else if (currentToken == lparentsym)
    {
			// Dive into the expression.
      getToken(tokenStream);
      expression(tokenStream, ofp, table, instReg);

			// If the current token after returning from the expression is a right parentheses, then
			// the expression was correctly terminated, and we get the next token.
      if (currentToken == rparentsym)
      {
        getToken(tokenStream);
      }
			// Otherwise, we're missing a right parentheses.
      else
      {
        printErrorMessage(ofp, 22);

				return;
      }
    }
  }

	return;
}

// Determines whether a segment of code follows the correct grammar for "term"
// This function generates instructions.
void term(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg)
{
  int termChar;

	// Term always begins with a factor
	factor(tokenStream, ofp, table, instReg);

	// Loop until either a multiplication sign or division sign is not encountered.
	// This simulates the fact that after a '*' or '/', there must be another factor, which can
	// be followed by more mult / div signs and accompanying factors.
  while (currentToken == multsym || currentToken == slashsym)
  {
		// Save the current token.
    termChar = currentToken;

		// Get the next token and process the factor again.
    getToken(tokenStream);
    factor(tokenStream, ofp, table, instReg);

		// If a multiplication sign is encountered, generate a MUL instruction.
		// It takes the values in the first two positions below the register pointer and places the
		// result in the second position.
    if (termChar == multsym)
    {
      generateInstruction(instReg, mul, ptrReg - 2, ptrReg - 2, ptrReg - 1);
			ptrReg--;
    }
		// If a division sign is encountered, generate a DIV instruction.
    else
    {
      generateInstruction(instReg, divi, ptrReg - 2, ptrReg - 2, ptrReg - 1);
			ptrReg--;
    }
  }

	return;
}

// Determines whether a segment of code follows the correct grammar for "expression"
// This function generates instructions. It does not directly generate errors because it relies
// so heavily on the definition of factor. Think of it as a binding agent that allows all forms of
// basic arithmetic expressions to be recognized by this language.
void expression(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg)
{
  int expChar;

	// Ensure that the first symbol found in expression follows grammatical rules.
	if (currentToken != plussym && currentToken != minussym && currentToken != identsym
		&& currentToken != numbersym && currentToken != lparentsym)
		{
			printErrorMessage(ofp, 24);

			return;
		}

	// If the current token is a plus or minus sign, then the code is asking the program
	// to turn a literal positive or negative.
	if (currentToken == plussym || currentToken == minussym)
  {
		// Save current token.
    expChar = currentToken;

		// Get the next token and call term in case we've encountered an arithmetic statement that
		// happens to begin with a negative number, i.e. -5 + 2 / 6;
    getToken(tokenStream);
    term(tokenStream, ofp, table, instReg);

		// If the saved token was a minus sign, then generate a NEG instruction.
    if (expChar == minussym)
    {
      generateInstruction(instReg, neg, ptrReg - 1, 0, 0);
    }
  }
	// Otherwise, we've only found a literal value and should call term to process it accordingly.
  else
  {
    term(tokenStream, ofp, table, instReg);
  }

	// Loop until a symbol other than an addition or subtraction sign is encountered.
  while (currentToken == plussym || currentToken == minussym)
  {
		// Save the current token.
    expChar = currentToken;

		// Get the next token and call term.
    getToken(tokenStream);
    term(tokenStream, ofp, table, instReg);

		// If the saved token is a plus sign, then generate an ADD instruction.
    if (expChar == plussym)
    {
      generateInstruction(instReg, add, ptrReg - 2, ptrReg - 2, ptrReg - 1);
			ptrReg--;
    }
		// Otherwise, generate a SUB instruction to subtract.
    else
    {
      generateInstruction(instReg, sub, ptrReg - 2, ptrReg - 2, ptrReg - 1);
			ptrReg--;
    }
  }

  return;
}

// Determines whether a segment of code follows the correct grammar for "condition"
// This function generates instructions.
void condition(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg)
{
  int relationChar;

	// If the current token is the reserved word "odd", then generate the ODD instruction to see
	// if the value in the selected position in the register is odd, and place the result in the
	// selected position.
  if (currentToken == oddsym)
  {
    getToken(tokenStream);
    expression(tokenStream, ofp, table, instReg);
    generateInstruction(instReg, odd, ptrReg - 1, 0, 0);
  }
	// Otherwise, the relation character is not odd, and other of the many other relation characters.
  else
  {
    expression(tokenStream, ofp, table, instReg);

		// If the current token is not any of the valid relation characters, state that a relation
		// operator was expected.
    if (currentToken != eqsym && currentToken != neqsym && currentToken != lessym
			&& currentToken != leqsym && currentToken != gtrsym && currentToken != geqsym)
    {
      printErrorMessage(ofp, 20);

			return;
    }
		// Otherwise, process the relation operator and generate instructions accordingly.
    else
    {
			// Save the current token before delving into the expression.
			relationChar = currentToken;
			getToken(tokenStream);
			expression(tokenStream, ofp, table, instReg);

			// This conditional block determines which relation operator is being operated on.
      if (relationChar == eqsym)
      {
        generateInstruction(instReg, eql, ptrReg - 2, ptrReg - 2, ptrReg - 1);
      }
      else if (relationChar == neqsym)
      {
        generateInstruction(instReg, neq, ptrReg - 2, ptrReg - 2, ptrReg - 1);
      }
      else if (relationChar == lessym)
      {
        generateInstruction(instReg, lss, ptrReg - 2, ptrReg - 2, ptrReg - 1);
      }
      else if (relationChar == leqsym)
      {
        generateInstruction(instReg, leq, ptrReg - 2, ptrReg - 2, ptrReg - 1);
      }
      else if (relationChar == gtrsym)
      {
        generateInstruction(instReg, gtr, ptrReg - 2, ptrReg - 2, ptrReg - 1);
      }
      else if (relationChar == geqsym)
      {
        generateInstruction(instReg, geq, ptrReg - 2, ptrReg - 2, ptrReg - 1);
      }

			ptrReg -= 2;
    }
  }

	return;
}

// Determines whether a segment of code follows the correct grammar for "statement"
// This function generates instructions. This function generates instructions, and serves almost as
// a hub for the body of the generated program.
void statement(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg)
{
  int i, ndxInstReg1, ndxInstReg2;

	// If the current token is an identifier, process the identifier to ensure it's within the grammar
	// and if it is, create an instruction.
	if (currentToken == identsym)
  {
		// Find the position of the identifier in the symbol table.
    i = position(table);

		// If the position of the identifier is 0, then the identifier was undeclared. Throw an error.
    if (i == 0)
    {
      printErrorMessage(ofp, 11);

			return;
    }
		// Otherwise, if the identifier at the given position is not a variable, it cannot have
		// reassignments made to it. State that is the case in an error message.
    else if (table[i].kind != VARIABLE)
    {
      printErrorMessage(ofp, 12);

			return;
    }

    getToken(tokenStream);

		// If the current token is a become symbol (':=' not '='), then get the next token.
    if (currentToken == becomessym)
    {
      getToken(tokenStream);
    }
		// Otherwise, state that an assignment operator was expected.
    else
    {
      printErrorMessage(ofp, 13);

			return;
    }

		// After an identifier and a ':=' comes an expression.
    expression(tokenStream, ofp, table, instReg);
		// Generate a STO instruction to store the value of the variable from the register into the
		// stack.
    generateInstruction(instReg, sto, --ptrReg, currentLevel - table[i].level, table[i].addr);
  }
	// Otherwise, if the current token is an if symbol, ensure it's within the grammar and generate
	// an instruction if it is.
  else if (currentToken == ifsym)
  {
    getToken(tokenStream);

		// "if" is followed by a condition
    condition(tokenStream, ofp, table, instReg);

		// After the condition, there must be a "then"
    if (currentToken == thensym)
    {
      getToken(tokenStream);
    }
		// If it's not "then", then state that "then" was expected.
    else
    {
      printErrorMessage(ofp, 16);

			return;
    }

		// Save the current instruction register index.
    ndxInstReg1 = ndxInstReg;

		// Generate a JPC instruction to jump to the body of the conditional if the value at the given
		// register is 0, skipping the conditional.
    generateInstruction(instReg, jpc, ptrReg <= 0 ? 0 : ptrReg - 1, currentLevel, ndxInstReg);
    statement(tokenStream, ofp, table, instReg);

		// Since else must follow if, but it is an optional item, it appears under the ifsym check.
		if (currentToken == elsesym)
		{
			getToken(tokenStream);

			// Add the instruction register index to the instruciton whose index was saved previously,
			// and then save the current index.
			instReg[ndxInstReg1].m = ndxInstReg + 1;
			ndxInstReg1 = ndxInstReg;

			// Genrate a jump instruction. Since it's else, we don't jump on a condition.
			generateInstruction(instReg, jmp, 0, 0, ptrReg);
			statement(tokenStream, ofp, table, instReg);
		}

		// Add the current instruction register index to the instruction whose index was saved
		// previously.
    instReg[ndxInstReg1].m = ndxInstReg;
  }
	// Otherwise, if the current token is a begin symbol, ensure it's within the grammar and generate
	// an instruction if it is.
  else if (currentToken == beginsym)
  {
		// "begin" is followed by a statement.
    getToken(tokenStream);
    statement(tokenStream, ofp, table, instReg);

		// Loop until a symbol that is not a semicolon, begin, if, or while is encountered.
    while (currentToken == semicolonsym || currentToken == beginsym || currentToken == ifsym
			|| currentToken == whilesym || currentToken == identsym)
    {
			// If a semicolon is encountered, get the next token to peek ahead.
      if (currentToken == semicolonsym)
      {
        getToken(tokenStream);
      }
			// Otherwise, state that the semicolon between statements is missing.
      else
      {
        printErrorMessage(ofp, 10);

				return;
      }

      statement(tokenStream, ofp, table, instReg);
    }

		// The very last token after "begin" is declared must be "end". Ensure that it is.
    if (currentToken == endsym)
    {
      getToken(tokenStream);
    }
		// If it's not, state that it was expected.
    else
		{
      printErrorMessage(ofp, 17);

			return;
    }
  }
	// Otherwise, if the current token is a while symbol, ensure it's within the grammar and generate
	// an instruction if it is.
  else if (currentToken == whilesym)
  {
		// Save the current instruction register index.
    ndxInstReg1 = ndxInstReg;

		// Get the next token and process the condition of the while loop.
    getToken(tokenStream);
    condition(tokenStream, ofp, table, instReg);

		// Save the current instruction register index.
    ndxInstReg2 = ndxInstReg;

		// Generate a JPC instruction to jump when the loop terminates.
    generateInstruction(instReg, jpc, ptrReg, 0, ndxInstReg1);

		// For the loop to begin, there must be a "do" symbol after the loop's condition.
    if (currentToken == dosym)
    {
      getToken(tokenStream);
    }
		// Otherwise, state that "do" was expected.
    else
    {
      printErrorMessage(ofp, 18);

			return;
    }

		// After a "do" is the body of the while loop, which is comprised of statements. For multiple
		// statements within a while loop, "begin" and "end" must be used within the loop.
    statement(tokenStream, ofp, table, instReg);
    generateInstruction(instReg, jmp, 0, 0, ndxInstReg1);

		// Add the current instruction register index to the instruction whose index was saved
		// previously.
    instReg[ndxInstReg2].m = ndxInstReg;
  }
	// Takes an input from the user and places it in the register, generating an instruction.
	else if (currentToken == readsym)
	{
		getToken(tokenStream);
		i = position(table);

		// If the position of the identifier is 0, then the identifier was undeclared. Throw an error.
    if (i == 0)
    {
      printErrorMessage(ofp, 11);

			return;
    }
		// Otherwise, if the identifier at the given position is not a variable, it cannot have
		// reassignments made to it. State that is the case in an error message.
    else if (table[i].kind != VARIABLE)
    {
      printErrorMessage(ofp, 12);

			return;
    }
		getToken(tokenStream);

		generateInstruction(instReg, sio2, ptrReg, 0, 2);
		generateInstruction(instReg, sto, ptrReg, currentLevel - table[i].level, table[i].addr);
	}
	// Prints the value stored in the requested variable to the screen, generating an instruction.
	else if (currentToken == writesym)
	{
		getToken(tokenStream);
		expression(tokenStream, ofp, table, instReg);

		generateInstruction(instReg, sio1, ptrReg - 1, 0, 1);
	}
	// Otherwise, if the current token is a call symbol, ensure it's within the grammar and generate
	// an instruction if it is.
	else if (currentToken == callsym)
	{
		getToken(tokenStream);

		// Call must be followed by an identifier. If it isn't, throw an error.
		if (currentToken != identsym)
		{
			printErrorMessage(ofp, 14);
		}
		// Otherwise, check to see if the called identifier is declared and generate an instruction
		// if it is.
		else
		{
			// Find the identifier.
			i = position(table);

			// If i is 0, then the identifier was not declared, and throw an error.
			if (i == 0)
			{
				printErrorMessage(ofp, 11);

				return;
			}
			// Otherwise, ensure the identifier is a procedure and generate an instruction.
			else if (table[i].kind == PROCEDURE)
			{
				// TODO MAY BE INCORRECT
				generateInstruction(instReg, cal, ptrReg, currentLevel - table[i].level, table[i].addr);
			}
			// If it's not a procedure, then throw an error message. The grammar does not allow calling
			// a cosntant or a variable.
			else
			{
				printErrorMessage(ofp, 15);
			}

			getToken(tokenStream);
		}
	}

  return;
}

// Determines whether a segment of code follows the correct grammar for "statement"
// This function generates instructions.
void block(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg, int ndxTable)
{
  // Initial symbol table index
  int ndxTableInit;

	// If no size for the inital register is specified, then it must be at least 4. This is to account
	// for the stack pointer, the base pointer, the program counter, and the return value.
	ndxData = 4;
	// Save the initial value of the table index.
  ndxTableInit = ndxTable;
	// Plac ethe current instruction register index into the address member of the current symbol.
  table[ndxTable].addr = ndxInstReg;
	// Generate a JMP instruction.
  generateInstruction(instReg, jmp, 0, 0, 0);

	// Loop until either a constant or variable is not encountered in order to declare all constants
	// by placing them in the symbol table.
  while (currentToken == constsym || currentToken == varsym || currentToken == procsym)
  {
		// If the current token is a constant, ensure that the constant declarations are within
		// the grammar.
    if (currentToken == constsym)
    {
      getToken(tokenStream);
			// Declare a constant.
			constantDeclaration(tokenStream, ofp, table);

			if (currentToken != commasym && currentToken != semicolonsym)
			{
				printErrorMessage(ofp, 5);
			}
			else
			{
				getToken(tokenStream);
			}

			// Until a symbol other than an identifier is encountered, declare constants and place them
			// in the symbol table.
      while (currentToken == identsym || currentToken == commasym)
      {
				// Declare a constant.
				constantDeclaration(tokenStream, ofp, table);

				// Continue declaring constants until a semicolon or unknown symbol is found.
				while (currentToken == commasym)
        {
          getToken(tokenStream);
          constantDeclaration(tokenStream, ofp, table);
        }

				// If the current token is a semicolon, then constant declaration has finished.
        if (currentToken == semicolonsym)
        {
          getToken(tokenStream);
					break;
        }
				// Otherwise, a semicolon or comma is missing.
        else
        {
          printErrorMessage(ofp, 5);

					return;
        }
      }
    }

		// If the current token is a variable, then ensure it's within the grammar and add it to the
		// symbol table if it is.
    if (currentToken == varsym)
    {
      getToken(tokenStream);
			variableDeclaration(tokenStream, ofp, table);

			if (currentToken != commasym && currentToken != semicolonsym)
			{
				printErrorMessage(ofp, 5);
			}
			else
			{
				getToken(tokenStream);
			}

			// Until a symbol other than an identifier is encountered, declare variables as they're found.
      while (currentToken == identsym || currentToken == commasym)
      {
        variableDeclaration(tokenStream, ofp, table);

				// Until a symbol other than a comma is found, continue to declare variables.
				while (currentToken == commasym)
        {
          getToken(tokenStream);
          variableDeclaration(tokenStream, ofp, table);
        }

				// Variable declarations must end with a semicolon.
        if (currentToken == semicolonsym)
        {
          getToken(tokenStream);
					break;
        }
				// Otherwise, state that a semicolon was expected.
        else
        {
          printErrorMessage(ofp, 5);

					return;
        }
      }
    }

		// This loop identifies procedure symbols, and also handles the procedure declaration itself.
		while (currentToken == procsym)
		{
			// Procedure must be followed by an identifier. If it is, insert the procedure identifier
			// into the symbol table.
			if (currentToken == identsym)
			{
				insert(table, ofp, PROCEDURE);
				getToken(tokenStream);
			}
			// Otherwise, something else follows it, so report an error.
			else
			{
				printErrorMessage(ofp, 4);
			}

			// Ensure the identifier is followed by a semicolon.
			if (currentToken == semicolonsym)
			{
				getToken(tokenStream);
			}
			// If not, report that it's missing.
			else
			{
				printErrorMessage(ofp, 5);
			}

			// Go to the next lexicographical level.
			currentLevel++;

			// Ensure the maximum amount of lexicographical levels hasn't been exceeded.
			if (currentLevel > MAX_LEXI_LEVELS)
			{
				printErrorMessage(ofp, 29);
			}

			// If the max levels hasn't been exceeded, go up a level for sure.
			block(tokenStream, ofp, table, instReg, ndxTable);

			// Ensure the following procedure ends with a semicolon as well.
			if (currentToken == semicolonsym)
			{
				getToken(tokenStream);
			}
			// If not, report that the semicolon is missing.
			else
			{
				printErrorMessage(ofp, 5);
			}
		}
  }

	// Add the current instruction register index to the M member of the instruction at the
	// initial table index.
  instReg[table[ndxTableInit].addr].m = ndxInstReg;
	// Add the current instruction register to the address member of the symbol at the initial
	// table index.
  table[ndxTableInit].addr = ndxInstReg;

	// Generate an INC instruction with enough space for all variables (if there are any).
  generateInstruction(instReg, inc, 0, currentLevel, ndxData);
  statement(tokenStream, ofp, table, instReg);
	generateInstruction(instReg, rtn, 0, 0, 0);

  return;
}

// Determines whether a segment of code follows the correct grammar for "program"
// "program" encapsulates the entire program.
void program(FILE *tokenStream, FILE *ofp, symbol *table, instruction *instReg)
{
	// Get the first token from the token stream.
  getToken(tokenStream);

  // Start at table index 0
  block(tokenStream, ofp, table, instReg, 0);

	// If the final token in the token stream is not a period, then state that a period was expected.
  if (currentToken != periodsym)
  {
    printErrorMessage(ofp, 9);

		return;
  }

	// Generate the SIO instruction that terminates the program.
	generateInstruction(instReg, sio3, 0, 0, 3);

  return;
}

// Handles the file pointers and creation of the output file, assuming no errors are created.
int parserCodeGenerator(void)
{
	FILE *tokenStream = fopen("token-stream.txt", "r");
	FILE *ofp = fopen("generated-assembly-code.txt", "w");

	int i;

  // Ensure input file was found.
  if (tokenStream == NULL)
  {
    printErrorMessage(ofp, 26);
  }

  // Ensure output file was created.
  if (ofp == NULL)
  {
    printErrorMessage(ofp, 27);
  }

	// Set up the symbol table and instruction register.
	symbol table[MAX_SYMBOL_TABLE_SIZE] = {0};
	instruction instReg[MAX_CODE_LENGTH];

	// Start processing the token stream, and allowing the parer / code-generator to act according
	// to the grammatical rules of PL/0
	program(tokenStream, ofp, table, instReg);

	// Otherwise, the output file can be filled with the contents of the instruction register,
	// ready to be processed by the virtual machine.
	for (i = 0; i < ndxInstReg; i++)
	{
		fprintf(ofp, "%d %d %d %d\n", instReg[i].op, instReg[i].r, instReg[i].l, instReg[i].m);
	}

	// Close the file streams.
  fclose(tokenStream);
  fclose(ofp);

	return 0;
}
