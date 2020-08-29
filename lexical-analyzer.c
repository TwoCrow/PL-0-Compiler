// Patrick Sherbondy
// COP 3402, Spring 2020
// Worked alone.

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

// Contains words reserved by PL/0
const char *reservedWords [] = {"const", "var", "procedure", "call", "begin", "end", "if", "then",
"else", "while", "do", "read", "write", "odd"};

// Contains symbols reserved by PL/0
const char symbols [] = {'+', '-', '*', '/', '(', ')', '=', ',', '.', '<', '>',  ';' , ':'};

// The core of the lexical analyzer. It takes in a filename, and then processes the input file into
// a token stream that can be easily read by further components of the machine. For now, all output
// is routed to an output file, including the character stream.
int lexicalAnalyzer(char *filename)
{
  // Open an input file specified by the user at the command line.
	FILE *ifp = fopen(filename, "r");
  // Open an output file to store the token stream.
	FILE *ofp = fopen("token-stream.txt", "w");

  // Get the linked list set up.
	token *currentToken = malloc(sizeof(token));
	token *head = currentToken;
	token *temp = head;

	// These are flag variables used later.
	int readingComment = 0, validSymbol = 0;
  // Keeps track of the size of the list.
	int listSize = 0;
	int i;

  // Used in parsing information from the input file.
	char tempChar, printInput, buffer;

	// Ensure the file opened successfully.
	if (ifp == NULL)
	{
		printf("*** ERROR IN LEXICAL ANALYZER ***\n");
		printf("File could not be opened. Check that you spelled the filename correctly, and\n");
		printf("that the file is in the same directory as lexical-analyzer.c");

		return 1;
	}

	// Ensure the output file was successfully created / overwritten.
	if (ofp == NULL)
	{
		printf("*** ERROR IN LEXICAL ANALYZER ***\n");
		printf("Could not create or overwrite output files.\n");

		return 1;
	}

  // Used to store the greater strings.
	char token[1024];

  // Take the first character from the file to get things started.
  buffer = fgetc(ifp);

  // This while loop serves as the beating heart of the program. It will eventually run through
  // every character in the input file.
	while (buffer != EOF)
	{
    // Reset i to 0 as a precaution. It is used in every loop within this program.
    i = 0;

		// If white space is encountered, skip past it and move to the next character.
		if (isspace(buffer))
		{
      buffer = fgetc(ifp);

			continue;
		}

    // The next few conditionals handle comments by recognizing the relevant characters, recognizing
    // false alarms (i.e. there was just a standalone '/' symbol), and recognizing when the comment
    // chain is ending.
    // This attempts to end the comment loop.
		if (readingComment == 1 && buffer == '*')
		{
      tempChar = fgetc(ifp);

      // Determine whether the / being evaluated is one that terminates the comment. If not, rewind.
      if (tempChar == '/')
      {
        readingComment = 0;
        buffer = fgetc(ifp);
        continue;
      }
      else
      {
        ungetc(tempChar, ifp);
      }
		}

		// If readingComment is 1, a comment was found and the loop is currently trying to find the
		// end of the comment string.
		if (readingComment == 1)
		{
      buffer = fgetc(ifp);

			continue;
		}

		// If a starting comment symbol is encountered, ignore it and the preceeding comment until a
		// closing comment symbol is found.
		if (buffer == '/')
		{
      tempChar = fgetc(ifp);

      // This determines if the * being evaluated helps begin a comment. If not, rewind.
      if (tempChar == '*')
      {
        readingComment = 1;
        continue;
      }
      else
      {
        ungetc(tempChar, ifp);
      }
		}

		// If the program encounters a digit, it needs to be stored in the linked list, and checked to
		// ensure that it falls within the guidelines.
		if (isdigit(buffer))
		{
      // Loop through the number being evaluated to ensure that it is a number.
      for (i = 0; isdigit(buffer) || isalpha(buffer); i++)
      {
        token[i] = buffer;

        if (!isdigit(buffer))
        {
          // If the next character is not a number, but a letter, then it's likely someone named
          // a variable such that it starts with a number, which is not allowed.
          if (isalpha(buffer))
          {
            token[i + 1] = '\0';
						printf("*** ERROR IN LEXICAL ANALYZER ***\n");
            printf("ERROR 1: Variable starting with \"%s\" begins with digit. All variables must start with a letter.\n", token);
  					printf("Terminating...\n");

            return 1;
          }

          ungetc(buffer, ifp);

          break;
        }

        // Ensure the number is within restrictions.
        if (i >= MAX_NUMBER_LEN)
        {
          token[i + 1] = '\0';

					printf("*** ERROR IN LEXICAL ANALYZER ***\n");
          printf("ERROR 2: The number \"%s\"'s length is too long. Max number length is: %d\n", token, MAX_NUMBER_LEN);
					printf("(Note: this is also Error Code 025 in parser / code-generator, but it is caught here.)\n");
  				printf("Terminating...\n");

          return 1;
        }

        buffer = fgetc(ifp);
      }
      // The loop will invariably collect an extra character, so undo it.
      ungetc(buffer, ifp);

      // Prevent garbage values from entering the string.
      token[i] = '\0';

      // Set relevant information in the list node.
      currentToken->id = numbersym;
			strcpy(currentToken->name, token);

			// Move to the next token.
			currentToken->next = malloc(sizeof(token));
			currentToken = currentToken->next;
			listSize++;
		}
		// Otherwise, if an alphabetic character is encountered, then the program has likely encountered
		// a variable, and needs to ensure the variable is within restrictions before adding it to the
		// list. It could also be a reserved word - like 'begin' or 'if' - and needs to handle these
		// words accordingly.
		else if (isalpha(buffer))
		{
      // Read through the input file until a character that is not a letter or a digit is found.
      for (i = 0; isalpha(buffer) || isdigit(buffer); i++)
      {
        token[i] = buffer;

        buffer = fgetc(ifp);

        // If it is not a letter or a digit, break out of the loop and rewind.
        if (!isalpha(buffer) && !isdigit(buffer))
        {
          ungetc(buffer, ifp);

          break;
        }
      }

      // Prevent garbage values from entering the token.
      token[i + 1] = '\0';

      // Ensure the token falls within length requirements.
      if (strlen(token) > MAX_IDENTIFIER_LEN)
      {
				printf("*** ERROR IN LEXICAL ANALYZER ***\n");
        printf("ERROR 3: Identifier \"%s\" is too long. Identifiers can be no longer than ", token);
				printf("%d characters.\n", MAX_IDENTIFIER_LEN);
				printf("Terminating...\n");

        return 1;
      }

      // Consume the next character in the input stream to ensure that the current identifier
      // contains valid characters.
      buffer = fgetc(ifp);
      validSymbol = 0;

      // Ensure that the character that terminated the above loop isn't a valid character meant
      // to be interpreted in another way. For example, if the above loop takes in "x*", it could be
      // seeing a section of the larger "z := x*y", which is valid. But if it took in "x$", then
      // the identifier contains an invalid character and the program must terminate.
      if (!isalpha(buffer) && !isdigit(buffer) && !isspace(buffer))
      {
        for (i = 0; i < TOTAL_SYMBOLS; i++)
        {
          if (buffer == symbols[i])
          {
            validSymbol = 1;
          }
        }

        // If validSymbol == 0, then the symbol currently contained within the identifier is not
        // valid, and the program must termiante.
        if (validSymbol == 0)
        {
					printf("*** ERROR IN LEXICAL ANALYZER ***\n");
          printf("ERROR 4: \"%c\" is an invalid symbol.\n", buffer);
          printf("Terminating...\n");

          return 1;
        }
      }

      // Undo the look-ahead.
      ungetc(buffer, ifp);

      // Loop through the reserved words to check if the current string being processed is a
      // reserved word.
      for (i = 0; i < TOTAL_RESERVED_WORDS; i++)
      {
        if (strcmp(token, reservedWords[i]) == 0)
        {
          break;
        }
      }

      // Compare the value of i to the values below, which sets the current token to one of the
      // reserved word values.

      // const
			if (i == 0)
			{
				currentToken->id = constsym;
				strcpy(currentToken->name, "const");
			}
			// var
			else if (i == 1)
			{
				currentToken->id = varsym;
				strcpy(currentToken->name, "var");
			}
			// begin
			else if (i == 2)
			{
				currentToken->id = beginsym;
				strcpy(currentToken->name, "begin");
			}
			// end
			else if (i == 3)
			{
				currentToken->id = endsym;
				strcpy(currentToken->name, "end");
			}
			// if
			else if (i == 4)
			{
				currentToken->id = ifsym;
				strcpy(currentToken->name, "if");
			}
			// then
			else if (i == 5)
			{
				currentToken->id = thensym;
				strcpy(currentToken->name, "then");
			}
			// while
			else if (i == 6)
			{
				currentToken->id = whilesym;
				strcpy(currentToken->name, "while");
			}
			// do
			else if (i == 7)
			{
				currentToken->id = dosym;
				strcpy(currentToken->name, "do");
			}
			// read
			else if (i == 8)
			{
				currentToken->id = readsym;
				strcpy(currentToken->name, "read");
			}
			// write
			else if (i == 9)
			{
				currentToken->id = writesym;
				strcpy(currentToken->name, "write");
			}
			// odd
			else if (i == 10)
			{
				currentToken->id = oddsym;
				strcpy(currentToken->name, "odd");
			}
			// If the current string is not a reserved word, then it must be an identifier.
			else
			{
				currentToken->id = identsym;
				strcpy(currentToken->name, token);
			}

      // Move to the next token.
      currentToken->next = malloc(sizeof(token));
      currentToken = currentToken->next;
      listSize++;
		}
		// If the current string is not caught by the previous two conditionals, then the program has
		// found one of the special symbols, and must add it to the token list accordingly.
		else
		{
			// Compare the active character to each symbol in the symbols array, until a match is found.
			for (i = 0; i < TOTAL_SYMBOLS; i++)
			{
				if (buffer == symbols[i])
				{
					break;
				}
			}

      // Whatever index the loop broke on is the character that the analyzer is currently evaluating.
      // Store that information in the token list node.
			// +
			if (i == 0)
			{
				currentToken->id = plussym;
				strcpy(currentToken->name, "+");
			}
			// -
			else if (i == 1)
			{
				currentToken->id = minussym;
				strcpy(currentToken->name, "-");
			}
			// *
			else if (i == 2)
			{
				currentToken->id = multsym;
				strcpy(currentToken->name, "*");
			}
			// /
			else if (i == 3)
			{
				currentToken->id = slashsym;
				strcpy(currentToken->name, "/");
			}
			// (
			else if (i == 4)
			{
				currentToken->id = lparentsym;
				strcpy(currentToken->name, "(");
			}
			// )
			else if (i == 5)
			{
				currentToken->id = rparentsym;
				strcpy(currentToken->name, ")");
			}
			// =
			else if (i == 6)
			{
				currentToken->id = eqsym;
				strcpy(currentToken->name, "=");
			}
			// ,
			else if (i == 7)
			{
				currentToken->id = commasym;
				strcpy(currentToken->name, ",");
			}
			// .
			else if (i == 8)
			{
				currentToken->id = periodsym;
				strcpy(currentToken->name, ".");
			}
			// <>, <=, and <
			else if (i == 9)
			{
        // Take in the next character to see what token is being dealt with.
        buffer = fgetc(ifp);

        // If it's >, it's not equal.
				// <> (NEQ)
				if (buffer == '>')
				{
					currentToken->id = neqsym;
					strcpy(currentToken->name, ">");
				}
        // If it's =, it's less than or equal.
				// <=
				else if (buffer == '=')
				{
					currentToken->id = leqsym;
					strcpy(currentToken->name, "<=");
				}
        // Otherwise, it has to be less than. If the character isn't recognized by the analzyer, it
        // will be caught by the next loop iteration.
				// <
				else
				{
          ungetc(buffer, ifp);

					currentToken->id = lessym;
					strcpy(currentToken->name, "<");
				}
			}
			// > and >=
			else if (i == 10)
			{
        // Take in the next character to see what token is being dealt with.
        buffer = fgetc(ifp);

        // If it's =, then we have a greater than or equal.
				// >=
				if (buffer == '=')
				{
					currentToken->id = geqsym;
					strcpy(currentToken->name, ">=");
				}
        // Otherwise, undo the character that was just consumed, because it's strictly greater than.
				// >
				else
				{
          ungetc(buffer, ifp);

					currentToken->id = gtrsym;
					strcpy(currentToken->name, ">");
				}
			}
			// ;
			else if (i == 11)
			{
				currentToken->id = semicolonsym;
				strcpy(currentToken->name, ";");
			}
			// :=
			else if (i == 12)
			{
        // Used to craft an explicit error message, just in case.
        token[0] = buffer;
        buffer = fgetc(ifp);
        token[1] = buffer;
        token[2] = '\0';

        // Ensure an assignment is being looked at. If not, report an error.
				if (buffer == '=')
				{
					currentToken->id = becomessym;
					strcpy(currentToken->name, ":=");
				}
        // If the symbol directly after any given ":" is not "=", then the analzyer treats the two
        // characters as a single token, and prints out an error message, hence the saving of the
        // characters in the token above.
				else
				{
					printf("*** ERROR IN LEXICAL ANALYZER ***\n");
					printf("ERROR 4: \"%s\" is an invalid symbol.\n", token);
					printf("Terminating...\n");

					return 1;
				}
			}
      // If none of the previous conditionals caught the value, then the value is invalid, and
      // an error must be reported.
			else
			{
				printf("*** ERROR IN LEXICAL ANALYZER ***\n");
				printf("ERROR 4: \"%c\" is an invalid symbol.\n", buffer);
				printf("Terminating...\n");

				return 1;
			}

			// Move to the next token.
			currentToken->next = malloc(sizeof(token));
			currentToken = currentToken->next;
			listSize++;
		}

    // Consume the next character in the file.
    buffer = fgetc(ifp);
	}

  // Reset the input file pointer so that the source program can be printed in the output file.
	rewind(ifp);

	// Print out the contents of everything into an output file.
	//fprintf(ofp, "Source Program:\n");

	// Print the input file's source code at the top of the output file.
	// while (fscanf(ifp, "%c", &printInput) != EOF)
	// {
	// 	fprintf(ofp, "%c", printInput);
	// }

  // Print out the headers for the lexeme table.
	// fprintf(ofp, "\n");
	// fprintf(ofp, "Lexeme Table:\n");
	// fprintf(ofp, "%-30s %12s\n", "lexeme", "token type");
	//
	// // Prints out the lexeme table one token at a time.
	// for (i = 0; i < listSize; i++)
	// {
	// 	fprintf(ofp, "%-30s %4d\n", temp->name, temp->id);
	//
	// 	temp = temp->next;
	// }
	//
	// fprintf(ofp, "\n");

	// fprintf(ofp, "Lexeme List:\n");

	temp = head;

  // Prints out the lexeme list.
	for (i = 0; i < listSize; i++)
	{
		fprintf(ofp, "%d ", temp->id);

		if (temp->id == 2 || temp->id == 3)
		{
			fprintf(ofp, "%s ", temp->name);
		}

		temp = temp->next;
	}

  // Close the file pointers.
	fclose(ifp);
	fclose(ofp);

	return 0;
}
