#include "trace.h"
#include <stdio.h>

static int tracing_enabled = 0;

void trace_enable(int enable)
{
    tracing_enabled = enable;
}

void trace(const char *message)
{
    if (tracing_enabled)
    {
        printf("%s\n", message);
    }
}