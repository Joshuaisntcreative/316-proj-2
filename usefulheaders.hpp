#ifndef LEXICAL_ANALYSER_H
#define LEXICAL_ANALYSER_H
#include <fstream>
#include <unordered_map>
#define LETTER 0
#define DIGIT 1
#define UNKNOWN 99
//used for the lexical analyser to handle multiple lines in the front.in file
#define NEWLINE 120
//Token codes

// going to define the custom codes for the project after right paren to make it easier to understand
#define INT_CONST 10
#define IDENT 11
#define FLOAT_CONST 12
#define COMMA 13
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


//will be used by the symboltable later on
enum Datatype {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_UNKNOWN   // useful for errors
};

union Value{
    int i;
    float f;
};
//this will hold every identifier's important symboltable info
struct SymbolInfo{
    Datatype type;
    Datatype expectedType;
    Datatype computedType;
    Value value;
    bool initialized;
};
extern std::unordered_map<std::string, SymbolInfo> symbolTable;
extern std::string lexeme_s;
extern int integerLiteral;
extern float floatLiteral;
#endif