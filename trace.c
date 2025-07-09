#include "trace.h"
#include <stdarg.h>
#include <stdio.h>

static int tracing_enabled = 0;

void TraceEnabled(int enable)
{
    tracing_enabled = enable;
}

void Trace(const char *format, ...)
{
    if (tracing_enabled)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}