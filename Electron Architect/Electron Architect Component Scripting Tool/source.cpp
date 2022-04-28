#include <fstream>

std::ifstream fin;
std::ifstream fout;

int main(int argc, char* argv[])
{
	if (argc != 2) // Not passed enough arguments
		return 1;

	fin.open(argv[1]);
	{

	}
	fin.close();
	return 0;
}