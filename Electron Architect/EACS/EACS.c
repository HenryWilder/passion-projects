#include <stdio.h>
#include "Parse.h"
#include "Output.h"

int main(int argc, char* argv[])
{
    FILE* input;

    if (argc != 2)
    {
        puts("Missing input arg");
        return 1;
    }

    fopen_s(&input, argv[1], "r");
    if (!input)
    {
        puts("File could not be opened");
        return 2;
    }

    puts("Success!");
    fseek(input, 0L, SEEK_END);
    long size = ftell(input);
    rewind(input);
    printf("Size: %i\n", size);

    Parse(input, size);

    fclose(input);
    return 0;
}
