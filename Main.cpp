#include <stdio.h>

typedef int Elem;

#define LOGS_TO_FILE
#define LOGS_TO_CONSOLE

#include "Stack/StackErrors.h"

const int    OUTPUT_TYPE = 0;   //!This constant is used to print stack elements to logs in right format
                                //!0 - int
                                //!1 - char
                                //!2 - float
                                //!3 - double
                                //!4 - long long

size_t PrintElem(int value, FILE *fp)
{
    char format[5] = "%";
    switch (OUTPUT_TYPE)
    {
        case 0:
            format[1] = 'd';
        break;
        case 1:
            format[1] = 'c';
        break;
        case 2:
            format[1] = 'f';
        break;
        case 3:
            format[1] = 'l';
            format[2] = 'g';
        break;
        case 4:
            format[1] = 'l';
            format[2] = 'l';
            format[3] = 'u';
        break;
    }
        
    #ifdef LOGS_TO_CONSOLE
        printf(format, value);
    #endif
    #ifdef LOGS_TO_FILE
        if (fp == nullptr)
            return ERROR_LOGS_OPEN;
        fprintf(fp, format, value);
    #endif

    return NO_ERROR; 
}

#include "Stack\Stack.h"

int main()
{
    printf("%d\n", WRONG_SIZE);
    Stack stk = {};
    StackCtor(&stk, 0);
    
    printf("%d\n", StackPush(&stk, 1000));
    printf("%d\n", StackPush(&stk, 100));
    printf("%d\n", StackPush(&stk, 10));
    printf("%d\n", StackPush(&stk, 1));
    printf("%d\n", StackPush(&stk, -1000));
    printf("%d\n", StackPush(&stk, -100));
    printf("%d\n", StackPush(&stk, -10));
    printf("%d\n", StackPush(&stk, -1));
    printf("%d\n", StackPush(&stk, 0));
    printf("%d\n", StackPush(&stk, 1));
    printf("%d\n", StackPush(&stk, 2));
    printf("%d\n", StackPush(&stk, 3));
    printf("%d\n", StackPush(&stk, 4));

    DUMP_STACK(stk);
}