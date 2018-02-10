#pragma once

#include <string>
#include <vector>

std::string replacefile(std::string name);

class libRQTS
{
public:
    libRQTS();
    ~libRQTS(void);
	void sendfile();
	bool recvfile();
	std::string checkfile();
	std::string getfile();
	std::string checkbound(std::string filename, unsigned char side, unsigned char* mask);
	bool checksplit(std::string filename);
	bool checkmerge(std::vector<std::string> filenames);
	void merge(std::string filename);
	void split(std::string filename);
	bool isdone();

private:

};
