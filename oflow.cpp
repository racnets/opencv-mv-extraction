#include "oflow.hpp"
#include "logger.h"

#include <cstdlib>

using namespace std;

int Oflow::process(std::string filename, int frames, bool analyse)
{
	return EXIT_SUCCESS;
}

int Oflow::log(int frames)
{
	// logging 
	doLogging(frames, mvCount, mvCountZero, mvCount - mvCountZero, mvSum.x, mvSum.y);

	return EXIT_SUCCESS;
}
