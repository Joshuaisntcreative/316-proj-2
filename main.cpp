// syntax analyzer code written in cpp
// ------------------------------------------------------------
// SYNTAX ANALYZER MAIN FILE
// ------------------------------------------------------------

// ------------------------------------------------------------
// main.cpp (with lexer embedded)
// ------------------------------------------------------------

#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <cstdlib>
#include <unordered_map>
#include "usefulheaders.hpp"
#include "sNode.hpp"
#include "customstack.cpp"

// ------------------------------------------------------------
// Lexer globals
// ------------------------------------------------------------
int charClass;
char lexeme[100];
char nextChar;
int lexLen;
int token;
int nextToken;
int integerLiteral;
float floatLiteral;
FILE *in_fp;
std::string lexeme_s;


// ------------------------------------------------------------
// Lexer helper functions
// ------------------------------------------------------------
void addChar() {
    if (lexLen <= 98) {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    } else
        printf("Error - lexeme is too long \n");
}

int lookup(char ch) {
    switch (ch) {
        case '(': addChar(); return nextToken = LEFT_PAREN;
        case ')': addChar(); return nextToken = RIGHT_PAREN;
        case '+': addChar(); return nextToken = ADD_OP;
        case '-': addChar(); return nextToken = SUB_OP;
        case '*': addChar(); return nextToken = MULT_OP;
        case '/': addChar(); return nextToken = DIV_OP;
        case '^': addChar(); return nextToken = POW_OP;
        case '=': addChar(); return nextToken = ASSIGN_OP;
        case ',': addChar(); return nextToken = COMMA;
        default:  addChar(); return nextToken = EOF_TOKEN;
    }
}

void getChar() {
    if ((nextChar = getc(in_fp)) != EOF) {
        if (isalpha(nextChar)) charClass = LETTER;
        else if (isdigit(nextChar)) charClass = DIGIT;
        else charClass = UNKNOWN;
    } else charClass = EOF_TOKEN;
}

void getNonBlank() {
    while (isspace(nextChar)) getChar();
}

// ------------------------------------------------------------
// Lexical analyzer function
// ------------------------------------------------------------
int lex() {
    lexLen = 0;
    getNonBlank();
    switch (charClass) {
        case LETTER: {
            addChar(); getChar();
            while (charClass == LETTER || charClass == DIGIT) { addChar(); getChar(); }
            lexeme_s = lexeme;
            if (lexeme_s == "int") nextToken = INT_KEYWORD;
            else if (lexeme_s == "float") nextToken = FLOAT_KEYWORD;
            else nextToken = IDENT;
            break;
        }
        case DIGIT: {
            addChar(); getChar();
            while (charClass == DIGIT) { addChar(); getChar(); }
            if (nextChar == '.') {
                addChar(); getChar();
                while (charClass == DIGIT) { addChar(); getChar(); }
                floatLiteral = atof(lexeme);
                nextToken = FLOAT_CONST;
            } else {
                integerLiteral = atoi(lexeme);
                nextToken = INT_CONST;
            }
            break;
        }
        case UNKNOWN: {
            nextToken = lookup(nextChar);
            getChar();
            break;
        }
        case EOF_TOKEN:
            nextToken = EOF_TOKEN;
            lexeme[0] = 'E'; lexeme[1] = 'O'; lexeme[2] = 'F'; lexeme[3] = 0;
            break;
    }
    return nextToken;
}

// ------------------------------------------------------------
// Forward declarations for parser functions
// ------------------------------------------------------------
sNode *assign();
sNode *assign_list();
sNode *expr();
sNode *term();
sNode *factor();
sNode *declare_list();
sNode *program();
sNode *root;

// Symbol table and datatype info
struct SymbolInfo;
std::unordered_map<std::string, SymbolInfo> symbolTable;
Datatype currDatatype;

// Forward declarations of AST/IL helpers
void printSymbolTable(const std::unordered_map<std::string, SymbolInfo> &table);
void process_single_declaration(Datatype type);
void printTree(sNode *node, int depth = 0);
static void printIndent(int depth);
std::string typeToString(Datatype t);
Datatype getVariableType(const std::string& id);
void computeTypes(sNode *node);
void generatePostfix(sNode* node);
void evaluateAST(sNode* node, Stack& stack);

// Intermediate language operations
void ftoi(Stack& stack);
void itof(Stack& stack);
void iadd(Stack& stack);
void fadd(Stack& stack);
void imult(Stack& stack);
void fmult(Stack& stack);
void idiv(Stack& stack);
void fdiv(Stack& stack);
void isub(Stack& stack);
void fsub(Stack& stack);


// ------------------------------------------------------------
// MAIN FUNCTION
// ------------------------------------------------------------
int main()
{
    // Open input file
    if ((in_fp = fopen("front.in", "r")) == NULL)
        printf("ERROR - cannot open front.in \n");
    else
    {
        getChar();  // initialize first character
        lex();      // get first token
        root = program(); // parse entire program
    }

    //computeTypes(root);   // determine types for AST
    // printSymbolTable(symbolTable);
    // printTree(root);
    // generatePostfix(root);

    Stack stack;
    evaluateAST(root, stack); // evaluate AST
    printSymbolTable(symbolTable); // show results
}

// ------------------------------------------------------------
// <program> -> <declare_list>{<declare_list>|<assign_list>}
// ------------------------------------------------------------
sNode *program()
{
    sNode *first = nullptr;
    sNode *last = nullptr;

    // Process declaration blocks first
    while (nextToken == INT_KEYWORD || nextToken == FLOAT_KEYWORD)
    {
        declare_list(); // fills symbol table
    }

    // Process assignments
    while (nextToken == IDENT)
    {
        sNode *nextNode = assign_list(); // parse assignment chain
        if (!first)
            first = nextNode; // root of AST
        if (last)
            last->right = nextNode; // link sequence of assignments
        last = nextNode;
    }

    return first; // returns root of assignment AST
}

// ------------------------------------------------------------
// <assign> -> <ident> = <expr>
// ------------------------------------------------------------
sNode *assign()
{
    if (nextToken != IDENT)
        return nullptr;

    // Create LHS identifier node
    sNode::Content c;
    c.identifier = lexeme_s; // assign string directly
    sNode *lhs = sNode::mkSnode(sNode::IDENTIFIER, c, nullptr, nullptr);

    lex(); // consume IDENT

    if (nextToken == ASSIGN_OP)
    {
        lex(); // consume '='
        sNode *rhs = expr(); // parse RHS expression
        sNode::Content opContent;
        opContent.op = '=';

        return sNode::mkSnode(sNode::OP, opContent, lhs, rhs);
    }

    std::cout << "Error: expected '=' in assign()\n";
    return nullptr;
}

// ------------------------------------------------------------
// <assign_list> -> {<ident>=}<expr>
// Handles chained assignments: a = b = c = expr
// ------------------------------------------------------------
sNode* assign_list()
{
    if (nextToken != IDENT)
        return nullptr;

    sNode* lhsNodes[100]; // store chain of LHS identifiers
    int lhsCount = 0;

    // Parse chain of identifiers separated by '='
    while (nextToken == IDENT)
    {
        sNode::Content c;
        c.identifier = lexeme_s; // full identifier
        lhsNodes[lhsCount++] = sNode::mkSnode(sNode::IDENTIFIER, c, nullptr, nullptr);

        lex(); // consume IDENT

        if (nextToken == ASSIGN_OP)
            lex(); // consume '='
        else
            break; // end of chain
    }

    // Parse RHS as full expression
    sNode* rhs = expr();

    // Build nested assignment tree from right to left
    for (int i = lhsCount - 1; i >= 0; --i)
    {
        sNode::Content opContent;
        opContent.op = '=';
        rhs = sNode::mkSnode(sNode::OP, opContent, lhsNodes[i], rhs);
    }

    return rhs; // return root of nested assignment
}

// ------------------------------------------------------------
// <expr> -> <term> {(+ | -) <term>}
// ------------------------------------------------------------
sNode *expr()
{
    sNode *left = term(); // parse first term

    while (nextToken == ADD_OP || nextToken == SUB_OP)
    {
        char op = (nextToken == ADD_OP ? '+' : '-');
        lex();
        sNode *right = term(); // parse next term
        sNode::Content opContent;
        opContent.op = op;

        left = sNode::mkSnode(sNode::OP, opContent, left, right); // left-associative
    }
    return left;
}

// ------------------------------------------------------------
// <term> -> <factor> {(* | /) <factor>}
// ------------------------------------------------------------
sNode *term()
{
    sNode *lhs = factor(); // parse first factor

    while (nextToken == MULT_OP || nextToken == DIV_OP)
    {
        char op = (nextToken == MULT_OP ? '*' : '/');
        lex();
        sNode *rhs = factor(); // parse next factor
        sNode::Content opContent;
        opContent.op = op;
        lhs = sNode::mkSnode(sNode::OP, opContent, lhs, rhs);
    }

    return lhs;
}

// ------------------------------------------------------------
// <factor> -> id | int_constant | ( <expr> ) | float_const
// ------------------------------------------------------------
sNode* factor()
{
    if (nextToken == IDENT)
    {
        sNode::Content c;
        c.identifier = lexeme_s;
        sNode* n = sNode::mkSnode(sNode::IDENTIFIER, c, nullptr, nullptr);
        lex(); // consume IDENT
        return n;
    }

    if (nextToken == INT_CONST)
    {
        sNode::Content c;
        c.integer_constant = integerLiteral;
        sNode* n = sNode::mkSnode(sNode::INT_CONSTANT, c, nullptr, nullptr);
        lex();
        return n;
    }

    if (nextToken == FLOAT_CONST)
    {
        sNode::Content c;
        c.float_constant = floatLiteral;
        sNode* n = sNode::mkSnode(sNode::FLOAT_CONSTANT, c, nullptr, nullptr);
        lex();
        return n;
    }

    if (nextToken == LEFT_PAREN)
    {
        lex(); // consume '('
        sNode* n = expr(); // parse inner expression
        if (nextToken == RIGHT_PAREN)
            lex(); // consume ')'
        else
            std::cout << "Error: expected ')' in factor" << std::endl;
        return n;
    }

    std::cout << "Error: unexpected token in factor" << std::endl;
    return nullptr;
}

// ------------------------------------------------------------
// <declare_list> -> (int|float) <ident> [=<expr>]{,<ident>[=<expr>]}
// ------------------------------------------------------------
sNode *declare_list()
{
    if (nextToken == INT_KEYWORD)
        currDatatype = TYPE_INT;
    else if (nextToken == FLOAT_KEYWORD)
        currDatatype = TYPE_FLOAT;
    else
    {
        std::cout << "Error: expected type specifier\n";
        return nullptr;
    }

    lex(); // consume type keyword

    if (nextToken != IDENT)
    {
        std::cout << "Error: expected identifier after type\n";
        return nullptr;
    }

    process_single_declaration(currDatatype);

    while (nextToken == COMMA)
    {
        lex();
        if (nextToken != IDENT)
        {
            std::cout << "Error: expected identifier after comma\n";
            return nullptr;
        }
        process_single_declaration(currDatatype);
    }

    return nullptr; // no AST nodes for declarations
}

// ------------------------------------------------------------
// Utility functions for printing
// ------------------------------------------------------------
void printSymbolTable(const std::unordered_map<std::string, SymbolInfo> &table)
{
    std::cout << "---------------------- \nSymbol table Contents\nName | Type | Value\n----------------------\n";
    for (const auto &entry : table)
    {
        const std::string &lname = entry.first;
        const SymbolInfo &info = entry.second;
        std::cout << lname << "  |  ";
        switch (info.type)
        {
        case TYPE_INT:
            std::cout << "INT   ";
            if (info.initialized)
                std::cout << info.value.i;
            else
                std::cout << "(uninitialized)";
            break;
        case TYPE_FLOAT:
            std::cout << "FLOAT ";
            if (info.initialized)
                std::cout << info.value.f;
            else
                std::cout << "(uninitialized)";
            break;
        default:
            std::cout << "UNKNOWN";
        }
        std::cout << std::endl;
    }
}

void process_single_declaration(Datatype type)
{
    std::string name = lexeme_s; // full identifier name

    // Insert or update symbol table entry
    symbolTable[name].type = type;
    symbolTable[name].initialized = false;

    lex(); // consume IDENT

    // Optional initializer
    if (nextToken == ASSIGN_OP)
    {
        lex(); // consume '='

        // ---- INT variable initialization ----
        if (type == TYPE_INT)
        {
            if (nextToken == INT_CONST)
            {
                symbolTable[name].value.i = integerLiteral;
                symbolTable[name].initialized = true;
            }
            else
            {
                std::cout << "Type error: cannot assign non-int constant to int variable '"
                          << name << "'\n";
            }
        }
        // ---- FLOAT variable initialization ----
        else if (type == TYPE_FLOAT)
        {
            if (nextToken == FLOAT_CONST)
            {
                symbolTable[name].value.f = floatLiteral;
                symbolTable[name].initialized = true;
            }
            else if (nextToken == INT_CONST)
            {
                // Optional: allow int -> float promotion
                symbolTable[name].value.f = std::stof(lexeme_s);
                symbolTable[name].initialized = true;
            }
            else
            {
                std::cout << "Type error: invalid constant for float variable '"
                          << name << "'\n";
            }
        }

        lex(); // consume constant
    }
}

static void printIndent(int depth)
{
    for (int i = 0; i < depth; i++)
        std::cout << "  ";
}

void printTree(sNode *node, int depth)
{
    if (!node)
        return;

    // Indentation
    for (int i = 0; i < depth; ++i)
        std::cout << "  ";

    // Print node info
    switch (node->tag)
    {
    case sNode::IDENTIFIER:
        std::cout << "IDENTIFIER: " << node->data.identifier;
        break;

    case sNode::INT_CONSTANT:
        std::cout << "INT_CONSTANT: " << node->data.integer_constant;
        break;

    case sNode::FLOAT_CONSTANT:
        std::cout << "FLOAT_CONSTANT: " << node->data.float_constant;
        break;

    case sNode::OP:
        std::cout << "OP: " << node->data.op;
        break;

    default:
        std::cout << "UNKNOWN NODE";
    }

    // Print computed and expected types
    std::cout << " [computed: " << typeToString(node->computedType)
              << ", expected: " << typeToString(node->expectedType) << "]"
              << std::endl;

    // Recurse to children
    printTree(node->left, depth + 1);
    printTree(node->right, depth + 1);
}

void computeTypes(sNode *node)
{
    if (!node)
        return;

    // Post-order traversal
    computeTypes(node->left);
    computeTypes(node->right);

    switch (node->tag)
    {
    case sNode::INT_CONSTANT:
        node->computedType = TYPE_INT;
        node->expectedType = TYPE_INT;
        break;

    case sNode::FLOAT_CONSTANT:
        node->computedType = TYPE_FLOAT;
        node->expectedType = TYPE_FLOAT;
        break;

    case sNode::IDENTIFIER:
        node->computedType = getVariableType(node->data.identifier); // now a string
        node->expectedType = node->computedType;
        break;

    case sNode::OP:
        if (node->data.op == '+' || node->data.op == '-' ||
            node->data.op == '*' || node->data.op == '/')
        {
            // Binary arithmetic operators
            if (node->left->computedType == TYPE_FLOAT ||
                node->right->computedType == TYPE_FLOAT)
                node->computedType = TYPE_FLOAT;
            else
                node->computedType = TYPE_INT;

            node->expectedType = node->computedType;
        }
        else if (node->data.op == '=')
        {
            // Assignment
            node->expectedType = node->left->computedType;  // LHS type
            node->computedType = node->right->computedType; // RHS type

            if (node->left->computedType != node->right->computedType)
            {
                std::cout << "Type mismatch in assignment: "
                          << node->left->data.identifier << std::endl;
            }
        }
        break;

    default:
        node->computedType = TYPE_UNKNOWN;
        node->expectedType = TYPE_UNKNOWN;
        break;
    }
}

Datatype getVariableType(const std::string& id)
{
    if (symbolTable.find(id) != symbolTable.end())
        return symbolTable[id].type;
    return TYPE_UNKNOWN;
}
// Helper to convert Type enum to string
std::string typeToString(Datatype t)
{
    switch (t)
    {
    case TYPE_INT:
        return "INT";
    case TYPE_FLOAT:
        return "FLOAT";
    default:
        return "UNKNOWN";
    }
}

void generatePostfix(sNode* node) {
    if (!node) return;

    // Left subtree
    generatePostfix(node->left);

    // Right subtree
    generatePostfix(node->right);

    // Print current node
    switch (node->tag) {
        case sNode::INT_CONSTANT:
            std::cout << node->data.integer_constant << " ";
            break;

        case sNode::FLOAT_CONSTANT:
            std::cout << node->data.float_constant << " ";
            break;

        case sNode::IDENTIFIER:
            std::cout << node->data.identifier << " ";
            break;

        case sNode::OP:
            std::cout << node->data.op << " ";
            break;
    }
}
// ------------------------------------------------------------
// evaluateAST
// Post-order traversal of AST to simulate execution / generate IL
// ------------------------------------------------------------
void evaluateAST(sNode* node, Stack& stack) {
    if (!node) return;

    // Evaluate left and right children first (post-order)
    evaluateAST(node->left, stack);
    evaluateAST(node->right, stack);

    switch(node->tag) {

        // ---- Leaf nodes ----
        case sNode::INT_CONSTANT: {
            StackValue v;
            v.type = INT_TYPE;
            v.i = node->data.integer_constant;
            stack.push(v);  // push value to stack
            std::cout << "PUSH_INT " << v.i << std::endl;
            break;
        }

        case sNode::FLOAT_CONSTANT: {
            StackValue v;
            v.type = FLOAT_TYPE;
            v.f = node->data.float_constant;
            stack.push(v);  // push value to stack
            std::cout << "PUSH_FLOAT " << v.f << std::endl;
            break;
        }

        case sNode::IDENTIFIER: {
            std::string name = node->data.identifier;
            auto& info = symbolTable[name];
            StackValue v;
            if (info.type == TYPE_INT) {
                v.type = INT_TYPE;
                v.i = info.value.i;
                std::cout << "PUSH_VAR_INT " << name << std::endl;
            } else {
                v.type = FLOAT_TYPE;
                v.f = info.value.f;
                std::cout << "PUSH_VAR_FLOAT " << name << std::endl;
            }
            stack.push(v);  // push variable value
            break;
        }

        // ---- Operator nodes ----
        case sNode::OP: {
            char op = node->data.op;

            if (op == '=') { 
                // Assignment operator

                StackValue rhs = stack.pop(); // pop RHS value

                // Handle type conversion if needed
                if (rhs.type == INT_TYPE && node->left->computedType == TYPE_FLOAT) {
                    std::cout << "ITOF" << std::endl;
                    stack.push(rhs);
                    itof(stack); // convert int -> float
                    rhs = stack.pop();
                } else if (rhs.type == FLOAT_TYPE && node->left->computedType == TYPE_INT) {
                    std::cout << "FTOI" << std::endl;
                    stack.push(rhs);
                    ftoi(stack); // convert float -> int
                    rhs = stack.pop();
                }

                // Store result in variable
                std::string varName = node->left->data.identifier;
                if (rhs.type == INT_TYPE) {
                    symbolTable[varName].value.i = rhs.i;
                    symbolTable[varName].initialized = true;
                } else {
                    symbolTable[varName].value.f = rhs.f;
                    symbolTable[varName].initialized = true;
                }

                std::cout << "STORE " << varName << std::endl;
                stack.push(rhs);  // optional: push value back to stack
            } 
            else {
                // Arithmetic operators (+, -, *, /)
                StackValue right = stack.pop();
                StackValue left = stack.pop();

                // Promote types if needed (int -> float)
                if (left.type == INT_TYPE && right.type == FLOAT_TYPE) {
                    std::cout << "ITOF" << std::endl;
                    stack.push(left);
                    itof(stack);
                    left = stack.pop();
                } else if (left.type == FLOAT_TYPE && right.type == INT_TYPE) {
                    std::cout << "ITOF" << std::endl;
                    stack.push(right);
                    itof(stack);
                    right = stack.pop();
                }

                // Push operands back
                stack.push(left);
                stack.push(right);

                // Execute the correct IL arithmetic operation
                switch(op) {
                    case '+':
                        if (node->computedType == TYPE_FLOAT) {
                            fadd(stack); std::cout << "FADD" << std::endl;
                        } else {
                            iadd(stack); std::cout << "IADD" << std::endl;
                        }
                        break;
                    case '-':
                        if (node->computedType == TYPE_FLOAT) {
                            fsub(stack); std::cout << "FSUB" << std::endl;
                        } else {
                            isub(stack); std::cout << "ISUB" << std::endl;
                        }
                        break;
                    case '*':
                        if (node->computedType == TYPE_FLOAT) {
                            fmult(stack); std::cout << "FMULT" << std::endl;
                        } else {
                            imult(stack); std::cout << "IMULT" << std::endl;
                        }
                        break;
                    case '/':
                        if (node->computedType == TYPE_FLOAT) {
                            fdiv(stack); std::cout << "FDIV" << std::endl;
                        } else {
                            idiv(stack); std::cout << "IDIV" << std::endl;
                        }
                        break;
                }
            }
            break;
        }

        default:
            std::cerr << "Unknown node type in evaluation" << std::endl;
            break;
    }
}

// ------------------------------------------------------------
// ----- Type conversion -----

// Convert INT on stack to FLOAT
void itof(Stack& stack) {
    StackValue v = stack.pop();
    StackValue result;
    result.type = FLOAT_TYPE;
    result.f = static_cast<float>(v.i);
    stack.push(result);
}

// Convert FLOAT on stack to INT
void ftoi(Stack& stack) {
    StackValue v = stack.pop();
    StackValue result;
    result.type = INT_TYPE;
    result.i = static_cast<int>(v.f);
    stack.push(result);
}

// ------------------------------------------------------------
// ----- Arithmetic operations -----

// Integer addition
void iadd(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    StackValue result;
    result.type = INT_TYPE;
    result.i = a.i + b.i;
    stack.push(result);
}

// Float addition
void fadd(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    StackValue result;
    result.type = FLOAT_TYPE;
    result.f = a.f + b.f;
    stack.push(result);
}

// Integer subtraction
void isub(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    StackValue result;
    result.type = INT_TYPE;
    result.i = a.i - b.i;
    stack.push(result);
}

// Float subtraction
void fsub(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    StackValue result;
    result.type = FLOAT_TYPE;
    result.f = a.f - b.f;
    stack.push(result);
}

// Integer multiplication
void imult(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    StackValue result;
    result.type = INT_TYPE;
    result.i = a.i * b.i;
    stack.push(result);
}

// Float multiplication
void fmult(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    StackValue result;
    result.type = FLOAT_TYPE;
    result.f = a.f * b.f;
    stack.push(result);
}

// Integer division
void idiv(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    if (b.i == 0) { std::cerr << "Division by zero (int)!\n"; return; }
    StackValue result;
    result.type = INT_TYPE;
    result.i = a.i / b.i;
    stack.push(result);
}

// Float division
void fdiv(Stack& stack) {
    StackValue b = stack.pop();
    StackValue a = stack.pop();
    if (b.f == 0.0f) { std::cerr << "Division by zero (float)!\n"; return; }
    StackValue result;
    result.type = FLOAT_TYPE;
    result.f = a.f / b.f;
    stack.push(result);
}
