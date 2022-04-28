#include <stdio.h>

int main(int argc, char* argv[])
{
	if (argc != 2) // Not passed enough arguments
		return 1;

	FILE* file;
	fopen_s(&file, argv[1], "r");

	if (!file)
	{
		printf("Failed to load file.");
		return 1;
	}

	printf("Successfully loaded file!");

	fclose(file);

	return 0;
}