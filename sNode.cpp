#include "sNode.hpp"

// Main constructor
sNode::sNode(Label l, Content c, sNode* left_child, sNode* right_child)
{
    tag = l;
    data = c;
    left = left_child;
    right = right_child;
}

// Factory function
sNode* sNode::mkSnode(Label l, Content c, sNode* left_child, sNode* right_child)
{
    return new sNode(l, c, left_child, right_child);
}

