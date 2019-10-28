#pragma once
#include <vector>
#include <map>

class StatUtils
{
public:
	const bool addStat(const std::string &mode, const float &fps);
	void clearStats();
	void printStats() const;

private:
	const static unsigned int MAX_DATA = 30;
	std::map<std::string, std::vector<float>> m_fpss;
};