#ifndef ENJON_RANDOM_H
#define ENJON_RANDOM_H

#include <random>

namespace Enjon { namespace Random {

	
	inline int Roll(int min, int max)
	{
		double x = rand()/static_cast<double>(RAND_MAX + 1);

		int roll = min + static_cast<int>(x * (max - min + 1));
		return roll;
	} 

}}



#endif