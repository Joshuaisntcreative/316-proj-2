#ifndef SNODE_H_
#define SNODE_H_

#include <string>
#include "usefulheaders.hpp"

class sNode
{
public:
    // What type of value this node holds
    enum Label {
        OP,
        INT_CONSTANT,
        IDENTIFIER,
        FLOAT_CONSTANT,
    };

    // The data stored in the node
    struct Content {
        char op;
        int integer_constant;
        float float_constant;
        std::string identifier;  // <-- now a string

        Content() : op(0), integer_constant(0), float_constant(0), identifier("") {}
    };

    // Constructor
    sNode(Label l, Content c, sNode* left_child, sNode* right_child);
    sNode();

    // Factory functions
    static sNode* mkSnode(Label l, sNode* left_child, sNode* right_child);
    static sNode* mkSnode(Label l, Content c, sNode* left_child, sNode* right_child);

public:
    Label tag;       // which kind of node this is
    Content data;    // holds the value
    sNode* left;     // left child
    sNode* right;    // right child

    Datatype computedType;
    Datatype expectedType;
};

#endif // SNODE_H_