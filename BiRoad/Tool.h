#pragma once
#include <string>
#include <vector>

class Tool
{
public:
	static std::vector<std::string> split(const std::string& s, const std::string &delimiter);
};
