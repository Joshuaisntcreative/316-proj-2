#ifndef LEXICAL_ANALYSER_H
#define LEXICAL_ANALYSER_H
#include <fstream>
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
//used for the lexical analyser to handle multiple lines in the front.in file
#define NEWLINE 120
//Token codes

// going to define the custom codes for the project after right paren to make it easier to understand
#define INT_LIT 10
#define IDENT 11
#define ASSIGN_OP 20
#define ADD_OP 21
#define SUB_OP 22
#define MULT_OP 23
#define DIV_OP 24
#define POW_OP 26
#define LEFT_PAREN 27
#define RIGHT_PAREN 28


//custom tokens codes for the project
#define INT_KEYWORD 50
#define FLOAT_KEYWORD 51


#define EOF_TOKEN -1

extern int nextToken;
extern char lexeme[];
extern FILE *in_fp;

int lex();
void getChar();
void addChar();
void getNonBlank();
int checkKeyword();

#endif