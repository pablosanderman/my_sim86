#include "sim86.h"
#include "trace.h"
#include <stdio.h>
#include <string.h>

#include "trace.c"

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-t") == 0)
    {
        trace_enable(1);
    }

    char FileName[] = "listing_0037_single_register_mov";
    u32 Result = 0;

    char buffer[1024];

    FILE *File = fopen(FileName, "rb");
    if (File)
    {
        Result = fread(buffer, 1, 1024, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
    }

    return 0;
}