#ifndef __STACK_SYM__
#define __STACK_SYM__

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <cstdint>

#include "Hash.h"
#include "Logging.h"
#include "StackErrors.h"

const char LOGS[]           = "StackLogs.txt";
const int  DUMP_LEVEL       = 2; //!This constant is used to print stack elements to logs in right format

#define CANARY_PROTECTION 1
#define HASH_PROTECTION   2
#define PROTECTION_LEVEL  CANARY_PROTECTION | HASH_PROTECTION

const double   FOR_RESIZE  = 2; 
const Elem     POISON      = 0X7FFFFFFF;
const void*    POISON_PTR  = (void*)13;
const uint64_t LEFT_KENAR  = 0xABABABABADEADDED;
const uint64_t RIGHT_KENAR = (uint64_t)(-1)^LEFT_KENAR;

//!------------------------------
//!status = true  if stack is ready to be used
//!status = false if stack wasn`t constructed or was destructed
//!
//!------------------------------
typedef struct LogInfo
{
    const char* name     = "(null)";
    const char* function = "(null)";
    const char* file     = "(null)";
    int  line            = POISON;
    bool status          = false;
} LogInfo;

typedef struct Stack
{
    uint64_t left_border = LEFT_KENAR;

    LogInfo debug = {};

    Elem   *data    = (Elem*)POISON_PTR;
    size_t size     = POISON;
    size_t capacity = POISON - 1;

    size_t struct_hash = 0;
    size_t data_hash   = 0;

    uint64_t right_border = RIGHT_KENAR;
} Stack;


size_t LogPrintf(FILE* fp, const char *format, ...);
void   DumpStack(Stack *stk, int deep, const char function[], const char file[], int line);

size_t StackCheck(Stack* stk, int line, const char function[], const char file[]);
size_t StackConstructor(Stack* stk, int capacity, int line, const char function[], const char file[], const char name[]);
size_t StackDtor(Stack* stk);
size_t StackResizeUp(Stack* stk);
size_t StackPush(Stack* stk, Elem value);
size_t StackResizeDown(Stack* stk);
Elem   StackPop(Stack* stk, size_t *err);

#define DUMP_STACK(stk) { DumpStack(&stk, DUMP_LEVEL, __PRETTY_FUNCTION__, __FILE__, __LINE__); }

void DoDumpStack(Stack* stk)
{
    #ifdef SAVE_MODE
    DUMP_STACK(*stk);
    #endif
}

#define OK_ASSERT(stk) {             \
    StackCheck(&stk, __LINE__, __PRETTY_FUNCTION__, __FILE__); \
    DoDumpStack(&stk); \
}

#define StackCtor(stk, capacity) StackConstructor(stk, capacity, __LINE__, __PRETTY_FUNCTION__, __FILE__, #stk)

size_t LogPrintf(FILE* fp, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    #ifdef LOGS_TO_CONSOLE
        vprintf(format, args);
    #endif

    #ifdef LOGS_TO_FILE
        if (fp == nullptr)
            return ERROR_LOGS_OPEN;
        vfprintf(fp, format, args);
    #endif

    return NO_ERROR;
    va_end(args);
}

//!------------------------------
//!deep = 1 - dump without printing stack elements
//!deep = 2 - 10 elements from start and 10 elements from end of the stack will be printed 
//!deep > 2 - all elements of the stack will be printed
//!
//!-----------------------------
void DumpStack(Stack *stk, int deep, const char function[], const char file[], int line)
{
    FILE* fp = fopen(LOGS, "a");
    if (fp == nullptr)
        return;
    
    LogPrintf(fp, "%s at %s(%d):\n", function, file, line);
    if (stk == nullptr)
    {
        LogPrintf(fp, "Null pointer to stack\n");
        fclose(fp);
        return;
    }
    LogPrintf(fp, "Stack[%p] \"%s\" at %s at %s(%d):\n", stk, stk->debug.name, stk->debug.function, stk->debug.file, stk->debug.line);
    LogPrintf(fp, "Status: ");
    if (stk->debug.status)
        LogPrintf(fp, "enable\n");
    else
        LogPrintf(fp, "disable\n");

    #if (PROTECTION_LEVEL & CANARY_PROTECTION)
    {
        LogPrintf(fp, "Left border  = %llu\n", stk->left_border);
        LogPrintf(fp, "Rigth border = %llu\n", stk->right_border);
    }
    #endif

    LogPrintf(fp, "{\n");
    LogPrintf(fp, "\tsize        = %d\n", stk->size);
    LogPrintf(fp, "\tcapacity    = %d\n", stk->capacity);
    LogPrintf(fp, "\tdata_hash   = %llu\n", stk->data_hash);
    LogPrintf(fp, "\tstruct_hash = %llu\n", stk->struct_hash);

    LogPrintf(fp, "\tdata[%p]\n", stk->data);
    LogPrintf(fp, "\t{\n");


    if (deep > 1 && stk->data != nullptr && stk->size != POISON && stk->capacity != POISON && stk->size <= stk->capacity)
    {
        #if (PROTECTION_LEVEL & CANARY_PROTECTION)
        {
            LogPrintf(fp, "\t\tLeftCan  = %llu\n", ((size_t*)stk->data)[-1]);
            LogPrintf(fp, "\t\tRightCan = %llu\n", *(size_t*)((char*)stk->data + sizeof(Elem)*stk->capacity));
        }
        #endif

        if (deep > 2 || stk->capacity <= 20)
        {
            size_t i = 0;
            for(i = 0; i < stk->size; i++)
            {
                LogPrintf(fp, "\t\t*[%d] = ", i);
                PrintElem(stk->data[i], fp);
                LogPrintf(fp, "\n");
            }

            for(i; i < stk->capacity; i++)
            {
                LogPrintf(fp, "\t\t[%d] = ", i);
                PrintElem(stk->data[i], fp);
                LogPrintf(fp, "\n");
            }

        }
        else
        {
            size_t out = 10;
            for(size_t i = 0; i < out; i++)
            {
                LogPrintf(fp, "\t\t");
                if (stk->size > i)
                    LogPrintf(fp, "*");
                LogPrintf(fp, "[%d] = ", i);
                PrintElem(stk->data[i], fp);
                LogPrintf(fp, "\n");
            }
            LogPrintf(fp, "\t\t...\n");
            for(size_t i = stk->capacity - out; i < stk->capacity; i++)
            {
                LogPrintf(fp, "\t\t");
                if (stk->size > i)
                    LogPrintf(fp, "*");
                LogPrintf(fp, "[%d] = ", i);
                PrintElem(stk->data[i], fp);
                LogPrintf(fp, "\n");
            }
        }
    }

    LogPrintf(fp, "\t}\n}\n\n");

    fclose(fp);
}

size_t StackCheck(Stack* stk, int line, const char function[], const char file[])
{
    FILE* fp = fopen(LOGS, "a");
    if (fp == nullptr)
        return ERROR_LOGS_OPEN;

    size_t error = NO_ERROR;
    if (stk == nullptr)
        error |= NULL_STACK_POINTER;
    else
    {
        if (stk->size == POISON)
            error |= WRONG_SIZE;
        if (stk->capacity == POISON || stk->capacity == POISON - 1)
            error |= WRONG_CAPACITY;
        if (stk->size > stk->capacity)
            error |= SIZE_BIGGER_CAPACITY;
        if (stk->data == nullptr || stk->data == POISON_PTR)
            error |= NULL_DATA;
        else
        {
            #if (PROTECTION_LEVEL & HASH_PROTECTION)
            {
                size_t hash = GetHash(stk->data, stk->capacity*sizeof(Elem));
                if (hash != stk->data_hash)
                    error |= DATA_HASH_MISMATCH;
            }
            #endif

            #if (PROTECTION_LEVEL & CANARY_PROTECTION)
            {
                if (((uint64_t*)stk->data)[-1] != LEFT_KENAR)
                    error |= LEFT_BORDER_DAMAGED;
                if (stk->capacity != POISON - 1 && 
                    *(uint64_t*)((char*)stk->data + stk->capacity*sizeof(Elem)) != RIGHT_KENAR)
                    error |= RIGHT_BORDER_DAMAGED;
            }
            #endif
        }
        
        #if (PROTECTION_LEVEL & HASH_PROTECTION)
        {
            size_t old_hash = stk->struct_hash;
            stk->struct_hash = 0;
            size_t now_hash = GetHash(stk, sizeof(Stack));
            stk->struct_hash = old_hash;
            if (stk->struct_hash != now_hash)
            {
                error |= STRUCT_HASH_MISMATCH;
            }
        }
        #endif

        if (stk->debug.file == nullptr)
            error |= DEBUG_INFO_DAMAGED; 
        if (stk->debug.name == nullptr)
            error |= DEBUG_INFO_DAMAGED; 
        if (stk->debug.line == POISON)
            error |= DEBUG_INFO_DAMAGED; 
    }

    
    LogPrintf(fp, "Stack = %p\n" "Chech status = %d\n", stk, error);
    LogPrintf(fp, "At %s in %s(%d)\n", function, file, line); 
    if (error != 0)
    {
        for(size_t i = 0; i < 32; i++)
            if (error & (1 << i))
                LogPrintf(fp, ERROR_DESCRIPTION[i]);
        LogPrintf(fp, "\n");
    }
    LogPrintf(fp, "\n");
    fclose(fp);

    return error;
}
 
size_t StackConstructor(Stack* stk, int capacity, int line, const char function[], const char file[], const char name[]) 
{
    size_t error = 0;
    *stk = {};
    stk->capacity = capacity;

    size_t new_capacity = stk->capacity*sizeof(Elem);
    #if (PROTECTION_LEVEL & CANARY_PROTECTION)
        new_capacity += sizeof(LEFT_KENAR) + sizeof(RIGHT_KENAR);
    #endif

    char* mem_block = (char*)calloc(new_capacity, 1);
    if (mem_block == nullptr)
    {
        stk->data = nullptr;
        error |= NULL_DATA | MEMORY_ALLOCATION_ERROR;
    }
    else
    {
        #if (PROTECTION_LEVEL & CANARY_PROTECTION)
        {
            *(uint64_t*)mem_block = LEFT_KENAR;
            *(uint64_t*)(mem_block + stk->capacity*sizeof(Elem) + sizeof(LEFT_KENAR)) = RIGHT_KENAR;
            stk->data = (Elem*)(mem_block + sizeof(LEFT_KENAR));
        }
        #else
            stk->data = (Elem*)(mem_block);
        #endif

        #if (PROTECTION_LEVEL & HASH_PROTECTION)
            stk->data_hash = GetHash(stk->data, stk->capacity*sizeof(Elem));
        #endif
    }
    stk->size = 0;

    stk->debug          = {};
    stk->debug.file     = file;
    stk->debug.function = function;
    stk->debug.name     = name;
    stk->debug.line     = line;
    stk->debug.status   = true;

    #if (PROTECTION_LEVEL & HASH_PROTECTION) 
        stk->struct_hash = 0;
        stk->struct_hash = GetHash(stk, sizeof(*stk));
    #endif

    OK_ASSERT(*stk);
    return error;
}


size_t StackDtor(Stack* stk)
{
    OK_ASSERT(*stk);
    stk->capacity    = POISON - 1;
    stk->size        = POISON;

    #if (PROTECTION_LEVEL & CANARY_PROTECTION)
        free((char*)stk->data - sizeof(LEFT_KENAR));
    #else
        free(stk->data);
    #endif
    stk->data         = (Elem*)POISON_PTR;

    stk->data_hash    = 0;
    stk->struct_hash  = 0;

    stk->debug.status = false;

    return NO_ERROR;
}

size_t StackResizeUp(Stack* stk)
{
    OK_ASSERT(*stk);

    if (stk->capacity == 0)
    {
        stk->capacity = 10;

        size_t new_capacity = stk->capacity*sizeof(Elem);
        #if (PROTECTION_LEVEL & CANARY_PROTECTION)
            new_capacity += sizeof(LEFT_KENAR) + sizeof(RIGHT_KENAR);
        #endif

        char* mem_block = (char*)calloc(new_capacity, 1);

        if (mem_block == nullptr)
        {
            OK_ASSERT(*stk);
            return MEMORY_ALLOCATION_ERROR;
        }

        #if (PROTECTION_LEVEL & CANARY_PROTECTION)
        {
            *(uint64_t*)mem_block = LEFT_KENAR;
            *(uint64_t*)(mem_block + stk->capacity*sizeof(Elem) + sizeof(LEFT_KENAR)) = RIGHT_KENAR;
            stk->data = (Elem*)(mem_block + sizeof(LEFT_KENAR));
        }
        #else
            stk->data = (Elem*)(mem_block);
        #endif

        return NO_ERROR;
    }

    stk->capacity = stk->capacity * FOR_RESIZE;
    size_t new_capacity = stk->capacity*sizeof(Elem);
    #if (PROTECTION_LEVEL & CANARY_PROTECTION)
        new_capacity += sizeof(LEFT_KENAR) + sizeof(RIGHT_KENAR);
    #endif

    char* mem_block = nullptr;
    #if (PROTECTION_LEVEL & CANARY_PROTECTION)
    {
        mem_block = (char*)realloc((char*)stk->data - sizeof(LEFT_KENAR), new_capacity);
        if (mem_block == nullptr)
            return MEMORY_ALLOCATION_ERROR;
        
        *(uint64_t*)mem_block = LEFT_KENAR;
        *(uint64_t*)(mem_block + stk->capacity*sizeof(Elem) + sizeof(LEFT_KENAR)) = RIGHT_KENAR;
        stk->data = (Elem*)(mem_block + sizeof(LEFT_KENAR));
    }
    #else
    {
        mem_block = (char*)realloc((char*)stk->data, new_capacity);
        if (mem_block == nullptr)
        {
            stk->data = nullptr;
            return MEMORY_ALLOCATION_ERROR;
        }
        stk->data = (Elem*)(mem_block);
    }
    #endif

    if (stk->data == nullptr)
        return MEMORY_ALLOCATION_ERROR;
    
    OK_ASSERT(*stk);
    return NO_ERROR;
}

size_t StackPush(Stack* stk, Elem value)
{
    OK_ASSERT(*stk);

    if (stk->capacity == stk->size)
    {
        size_t error = 0;
        if ((error = StackResizeUp(stk)))
        {   
            OK_ASSERT(*stk);
            return error;
        }
    }
    stk->data[stk->size++] = value;

    #if (PROTECTION_LEVEL & HASH_PROTECTION)
    {
        stk->data_hash = GetHash(stk->data, sizeof(Elem)*stk->capacity);

        stk->struct_hash = 0;
        stk->struct_hash = GetHash(stk, sizeof(Stack));
    }
    #endif

    OK_ASSERT(*stk);
    return NO_ERROR;
}

size_t StackResizeDown(Stack* stk)
{
    OK_ASSERT(*stk);

    if (stk->capacity == 0)
        return NO_ERROR;

    if (stk->capacity/(double)stk->size >= FOR_RESIZE*FOR_RESIZE)
    {
        stk->capacity = stk->capacity / FOR_RESIZE;

        size_t new_capacity = stk->capacity*sizeof(Elem);
        #if (PROTECTION_LEVEL & CANARY_PROTECTION)
        {
            new_capacity += sizeof(LEFT_KENAR) + sizeof(RIGHT_KENAR);
            char* mem_block = (char*)realloc((char*)stk->data - sizeof(LEFT_KENAR), new_capacity);
            if (mem_block == nullptr)
            {
                stk->data = nullptr;
                return MEMORY_ALLOCATION_ERROR;
            }

            *(uint64_t*)mem_block = LEFT_KENAR;
            *(uint64_t*)(mem_block + stk->capacity*sizeof(Elem) + sizeof(LEFT_KENAR)) = RIGHT_KENAR;
            stk->data = (Elem*)(mem_block + sizeof(LEFT_KENAR));
        }
        #else
            stk->data = (Elem*)realloc(stk->data, new_capacity);
        #endif
    }
    
    OK_ASSERT(*stk);
    if (stk->data == nullptr)
        return MEMORY_ALLOCATION_ERROR;
    return NO_ERROR;
}

Elem StackPop(Stack* stk, size_t *err = nullptr)
{
    OK_ASSERT(*stk);

    if (stk->size > 0)
        stk->size--;
    if (stk->size >= 0)
    {
        if (stk->capacity <= 0)
        {
            if (err != nullptr)
                *err |= WRONG_CAPACITY;
            return POISON;
        }
        Elem result = stk->data[stk->size];
        stk->data[stk->size] = POISON;

        int now_error = StackResizeDown(stk);
        if (err != nullptr)
            *err = now_error;
        if (now_error != NO_ERROR)
        {
            OK_ASSERT(*stk);
            return POISON;
        }

        #if (PROTECTION_LEVEL & HASH_PROTECTION)
        {
            stk->data_hash = GetHash(stk->data, stk->capacity*sizeof(Elem));
            stk->struct_hash = 0;
            stk->struct_hash = GetHash(stk, sizeof(Stack));
        }
        #endif
        return result;
    }
    else
    {
        StackCheck(stk, __LINE__, __PRETTY_FUNCTION__, __FILE__);
        if (err != nullptr) 
            *err = WRONG_SIZE;
        return POISON;
    }
}

#endif  //__STACK_SYM__