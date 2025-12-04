#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include "usefulheaders.hpp"
#include <string.h>
#include <cstdlib>
/* Global declarations */
/* Variables */
int charClass;
char lexeme[100];
char nextChar;
int lexLen;
int token;
int nextToken;
int integerLiteral;
FILE *in_fp, *fopen();
std::string lexeme_s;
/*****************************************************/
/* lookup - a function to lookup operators and parentheses
and return the token */

int lookup(char ch)
{
    switch (ch)
    {
    case '(':
        addChar();
        nextToken = LEFT_PAREN;
        break;
    case ')':
        addChar();
        nextToken = RIGHT_PAREN;
        break;
    case '+':
        addChar();
        nextToken = ADD_OP;
        break;
    case '-':
        addChar();
        nextToken = SUB_OP;
        break;
    case '*':
        addChar();
        nextToken = MULT_OP;
        break;
    case '/':
        addChar();
        nextToken = DIV_OP;
        break;
    case '^':
        addChar();
        nextToken = POW_OP;
        break;
    case '=':
        addChar();
        nextToken = ASSIGN_OP;
        break;
    case ',':
        addChar();
        nextToken = COMMA;
        break;
    default:
        addChar();
        nextToken = EOF;
        break;
    }
    return nextToken;
}

// you want a separate look up table for the +=, -=,...
// we can tell the lexical analyser to look at this special table if the codes for the

/*****************************************************/
/* addChar - a function to add nextChar to lexeme */
void addChar()
{
    if (lexLen <= 98)
    {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    }
    else
        printf("Error - lexeme is too long \n");
}

// called when there is an operator detected by the lexical analyser and will determine whether the token is a regular operator or an incremental one i.e (+, +=)
// will call get next char to peek ahead and determine if the next char is a =, if so we will handle the call differently.

/*****************************************************/
/* getChar - a function to get the next character of
input and determine its character class */
void getChar()
{
    // getc returns an int.
    if ((nextChar = getc(in_fp)) != EOF)
    {
        if (isalpha(nextChar))
            charClass = LETTER;
        else if (isdigit(nextChar))
            charClass = DIGIT;
        else
            charClass = UNKNOWN;
    }
    else
        charClass = EOF;
}

/*****************************************************/

/* getNonBlank - a function to call getChar until it
returns a non-whitespace character */
void getNonBlank()
{
    while (isspace(nextChar))
        getChar();
}
/******************************************************/
/* lex - a simple lexical analyzer for arithmetic
expressions */
int lex()
{
    lexLen = 0;
    getNonBlank();
    switch (charClass)
    {
        /* Parse identifiers */
    case LETTER:
        addChar();
        getChar();
        while (charClass == LETTER || charClass == DIGIT)
        {
            addChar();
            getChar();
        }
        lexeme_s = lexeme;

        if (lexeme_s == "int")
        {
            nextToken = INT_KEYWORD;
        }
        else if (lexeme_s == "float")
        {
            nextToken = FLOAT_KEYWORD;
        }
        else
        {
            nextToken = IDENT;
        }

        break;
    /* Parse integer literals */
    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT)
        {
            addChar();
            getChar();
        }
        integerLiteral = atoi(lexeme);
        break;
    /* Parentheses and operators */
    case UNKNOWN:
        nextToken = lookup(nextChar);
        getChar();
        break;
    /* EOF */
    case EOF:
        nextToken = EOF;
        lexeme[0] = 'E';
        lexeme[1] = 'O';
        lexeme[2] = 'F';
        lexeme[3] = 0;
        break;
    } /* End of switch */
    // std::cout << "Next token is: " << nextToken << " Next lexeme is " << lexeme << std::endl;
    return nextToken;
} /* End of function lex */
