#ifndef ENJON_COMMON_H
#define ENJON_COMMON_H

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Constants.h"
#include "Defines.h"
#include "System/Types.h"

namespace Enjon {
		
	//Cardinal directions
	#define NORTH		Enjon::Math::Vec2(0, 1)
	#define NORTHEAST   Enjon::Math::Vec2(1, 1)
	#define NORTHWEST   Enjon::Math::Vec2(-1, 1)
	#define EAST		Enjon::Math::Vec2(1, 0)
	#define WEST		Enjon::Math::Vec2(-1, 0)
	#define SOUTH		Enjon::Math::Vec2(0, -1)
	#define SOUTHEAST   Enjon::Math::Vec2(1, -1)
	#define SOUTHWEST   Enjon::Math::Vec2(-1, -1)

	// Bools
	#ifndef TRUE 		
		#define TRUE 	1
	#endif
	#ifndef FALSE
		#define FALSE	0
	#endif

	#ifndef PI
		#define PI 3.14159265359
	#endif
		
	inline float ToRadians(float degrees)
	{
		return degrees * (float)(PI / 180.0f);
	} 

	inline float ToDegrees(float radians)
	{
		return radians * (180.0f / (float)PI); 
	}
	
	inline Enjon::Vec2 IsoToCartesian(const Enjon::Vec2& point)
	{
		return Enjon::Vec2((2 * point.y + point.x) / 2.0f, (2 * point.y - point.x) / 2.0f);
	}

	inline Enjon::Vec2 CartesianToIso(const Enjon::Vec2& point)
	{
		return Enjon::Vec2(point.x - point.y, (point.x + point.y) / 2.0f); 
	}

	inline float Lerp(float goal, float current, float dt)
	{
		float difference = goal - current;

		if (difference > dt)
			return current + dt;
		if (difference < -dt)
			return current - dt;

		return goal;
	}

	template <typename T>
	inline T Clamp(T val, T min, T max)
	{
		if (val < min) return min;
		if (val > max) return max;
		return val;
	}	

	template <typename T>
	inline T MinClamp(T val, T min)
	{
		if (val < min) return min;
		return val;
	}

}

#endif