#include "sNode.hpp"

// Main constructor
sNode::sNode(Label l, Content c, sNode* left_child, sNode* right_child)
{
    tag = l;
    data = c;
    left = left_child;
    right = right_child;
}

sNode::sNode()
{
    
}
sNode* sNode::mkSnode(Label l, sNode* left_child, sNode* right_child)
{
    Content placeHolder;
    return mkSnode(l,placeHolder, left_child, right_child);
}
// Factory function
sNode* sNode::mkSnode(Label l, Content c, sNode* left_child, sNode* right_child)
{
    sNode* node = new sNode();
    node->tag = l;
    node->data = c;
    node ->left = left_child;
    node -> right = right_child;
    return node;
}

