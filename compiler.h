// Patrick Sherbondy
// COP 3402, Spring 2020
// Worked alone.

// This header file was created in the interest of combining all shared aspects of the mini compiler.
// It contains every definition and struct used across the various files to simplify the
// programming process.

#define MAX_IDENTIFIER_LEN 11
#define MAX_NUMBER_LEN 5
// Change this value to reflect the total number of reserved words, should any be added in the
// future for some reason.
#define TOTAL_RESERVED_WORDS 14
#define TOTAL_SYMBOLS 13
#define TOTAL_TOKENS 33
#define MAX_STACK_HEIGHT 40
#define MAX_CODE_LENGTH 200
#define MAX_LEXI_LEVELS 3
#define MAX_SYMBOL_TABLE_SIZE 200

#define CONSTANT 1
#define VARIABLE 2
#define PROCEDURE 3

// Contains the various token types.
// PLEASE NOTE: I realize that callsym, procsym, and elsesym are contained in this string array.
// I do not utilize them in anyway. They are necessary because of the standard that was set for
// these token types in assignment 2. If I got rid of these three symbols here, then the index
// system used to identify these tokens would be incorrect.
typedef enum
{
    nulsym = 1, identsym, numbersym, plussym, minussym,
    multsym,  slashsym, oddsym, eqsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym,
    writesym, readsym, elsesym
} token_type;

// Used to simplify passing opcode arguments to the generateInstruction() function in
// parser-code-generator.c
typedef enum
{
  lit = 1, rtn, lod, sto, cal, inc, jmp, jpc, sio1, sio2, sio3, neg, add, sub, mul, divi, odd,
  mod, eql, neq, lss, leq, gtr, geq
} op_code;

// Used for the symbol table in parser-code-generator.c
typedef struct
{
  // const = 1, var = 2
	int kind;
  // Name up to 11 characters
	char name[12];
  // Number (ASCII value)
	int val;
  // Lexicographical level
	int level;
  // M address
	int addr;
} symbol;

// Used for token stream in lexical-analyzer.c and parser-code-generator.c
typedef struct token
{
	token_type id;
	char name[MAX_IDENTIFIER_LEN + 1];
	struct token *next;
} token;

// Used to represent instructions in machine code as the output of parser-code-generator.c
// and as the input foro virtual-machine.c
typedef struct
{
	int op; // opcode
	int r;  // register
	int l;  // lexicographical level
	int m;  // M
} instruction;

int lexicalAnalyzer(char *filename);
int parserCodeGenerator(void);
int virtualMachine(void);
