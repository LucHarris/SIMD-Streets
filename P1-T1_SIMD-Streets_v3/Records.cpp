#include "Records.h"
#include "Constants.h"

Records::Records()
	:
	filename("output_records.txt")
{
	fileout.open(filename, std::ios::app);
}

Records::~Records()
{
	fileout.close();
}

void Records::ToFile(const char* testType, uint32_t updateTime, uint32_t numFighters, uint32_t numBlue, uint32_t numPurple, uint32_t numAlive)
{
	if (fileout.is_open())
	{
		fileout
			<< testType << '\t'
			<< updateTime << '\t'
			<< numFighters << '\t'
			<< numBlue << '\t'
			<< numPurple << '\t'
			<< numAlive << '\n';
	}
}
