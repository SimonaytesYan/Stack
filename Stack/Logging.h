#ifndef __LOGGING_SYM__
#define __LOGGING_SYM__
#include <stdio.h>

#include "StackErrors.h"

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

#endif //__LOGGING_SYM__
