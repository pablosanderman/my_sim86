#include "sim86.h"
#include <stdio.h>

int main()
{
    u32 Result = 0;
    FILE *File = fopen("listing_0037_single_register_mov", "rb");
    if (File)
    {
        char buffer[1024];
        Result = fread(buffer, 1, 1024, File);
        printf("Read %u bytes\n", Result);

        for (u32 i = 0; i < Result; i++)
        {
            printf("%02x", (unsigned char)buffer[i]);
        }
        printf("\n");

        fclose(File);
        return 0;
    }
    else
    {
        printf("Failed to open file\n");
        return 1;
    }
}