#include <stdio.h>

typedef int Elem;

#define LOGS_TO_FILE
#define LOGS_TO_CONSOLE

#include "Stack\Stack.h"

int main()
{
    Stack stk = {};
    StackCtor(&stk, 0);

    StackPop(&stk);

    DUMP_STACK(stk);
}