#pragma once
#include <cstdint>
#include <chrono>
#include <fstream>

struct Record
{
	// update time
	uint32_t ut;
};

class Records
{
	time_t id = time(0);
	std::ofstream fileout;
	const char filename[64];
public:
	Records();
	~Records();
	void ToFile(const char* testType, uint32_t updateTime, uint32_t numFighters, uint32_t numBlue, uint32_t numPurple, uint32_t alive);
};

