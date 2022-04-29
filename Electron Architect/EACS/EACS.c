#include <stdio.h>

int main(int argc, char* argv[])
{
    FILE* file;

    if (argc != 2)
    {
        puts("Missing input arg");
        return 1;
    }

    fopen_s(&file, argv[1], "r");
    if (!file)
    {
        puts("File could not be opened");
        return 2;
    }

    puts("Success!");

    fclose(file);
    return 0;
}
