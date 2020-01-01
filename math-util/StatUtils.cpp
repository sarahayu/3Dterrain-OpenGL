#include "StatUtils.h"
#include <iostream>

const bool StatUtils::addStat(const std::string & mode, const float & fps)
{
	auto &fpss = m_fpss[mode];
	if (fpss.size() < MAX_DATA - 1)
	{
		fpss.push_back(fps);
		return true;
	}
	
	return false;
}

void StatUtils::clearStats()
{
	for (auto &fpss : m_fpss) fpss.second.clear();
}

void StatUtils::printStats() const
{
	for (auto &fpss : m_fpss)
	{
		std::cout << "\n" << fpss.first.c_str() << "\n";
		for (const float &fps : fpss.second) std::cout << fps << " ";
	}
}
