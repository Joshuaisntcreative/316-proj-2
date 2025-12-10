//custom stack class for the project

//we are going to implement the basic functions
/*
push(x)
pop()
top()
isEmpty()
*/
#include <iostream>
#include "usefulheaders.hpp"
class Stack {
private:
    StackValue data[100];
    int topIndex;

public:
    Stack() : topIndex(-1) {}

    // Push a StackValue
    void push(StackValue v) {
        if (topIndex < 99) {
            data[++topIndex] = v;
        } else {
            // optional: handle overflow error
        }
    }

    // Pop a StackValue
    StackValue pop() {
        if (topIndex >= 0) {
            return data[topIndex--];
        } else {
            // optional: handle underflow error
            StackValue dummy;
            dummy.type = INT_TYPE;
            dummy.i = 0;
            return dummy;
        }
    }

    // Peek at the top StackValue
    StackValue top() const {
        return data[topIndex];
    }

    // Check if empty
    bool empty() const {
        return topIndex == -1;
    }
};