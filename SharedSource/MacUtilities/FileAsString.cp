#include <fstream>

#include "FileAsString.h"

string FileAsString(const string& fn, ios::openmode mode)
{
	ifstream ifs(fn.c_str(), mode);
	
	ifs.seekg(0, ios::end);
	int length = ifs.tellg();
	vector<char> buffer(length);
	ifs.seekg(0);
	ifs.read(&buffer[0], buffer.size());
	return string(&buffer[0], buffer.size());
}
