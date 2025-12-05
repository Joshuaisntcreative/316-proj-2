#ifndef SNODE_H_
#define SNODE_H_
class sNode
{
public:
    // What type of value this node holds
    enum Label {
        OP,
        INT_CONSTANT,
        IDENTIFIER,
        FLOAT_CONSTANT
    };

    // The data stored in the node
    union Content {
        char op;
        int integer_constant;
        float float_constant;
        char identifier;

        Content() {}   // Needed because union members are non-trivial
    };

    // Constructor
    sNode(Label l, Content c, sNode* left_child = nullptr, sNode* right_child = nullptr);

    

    // Factory function
    static sNode* mkSnode(Label l, Content c, sNode* left_child, sNode* right_child);

public:
    Label tag;             // which kind of node this is
    Content data;          // holds the value
    sNode* left;           // left child
    sNode* right;          // right child
};

#endif // SNODE_H_