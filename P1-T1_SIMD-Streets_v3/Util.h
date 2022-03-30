#pragma once
#include <ctime>
#include <random>
float RandF(float minimum, float maximum)
{

	if (minimum > maximum)
	{
		float tempMax = maximum;
		maximum = minimum;
		minimum = tempMax;
	}

	return minimum + (float) (rand()) / ((float) (RAND_MAX / (maximum - minimum)));
}