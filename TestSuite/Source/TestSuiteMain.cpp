// @file TestSuiteMain.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include <Enjon.h>
 
#include <filesystem> 
#include <iostream> 
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define ENJON_MEMORY_LEAK_DETECTION 1
#if ENJON_MEMORY_LEAK_DETECTION
	#include <vld.h> 
#endif

namespace FS = std::experimental::filesystem; 

using namespace Enjon; 

bool Equals( const Quaternion& q1, const glm::quat& q2 )
{
	return ( ( q1.x == q2.x ) && ( q1.y == q2.y ) && ( q1.z == q2.z ) && ( q1.w == q2.w ) );
}

bool Equals( const Vec3& v1, const glm::vec3& v2 )
{
	return ( ( v1.x == v2.x ) && ( v1.y == v2.y ) && ( v1.z == v2.z ) );
}

String QuatToString( const glm::quat& q )
{
	return ( "glm::quat( " + std::to_string(q.x) + ", " + std::to_string(q.y) + ", " + std::to_string(q.z) + ", " + std::to_string(q.w) + " )" );
}

String Vec3ToString( const glm::vec3& v )
{
	return ( "glm::vec3( " + std::to_string( v.x ) + ", " + std::to_string( v.y ) + ", " + std::to_string( v.z ) + " )" );
}

#ifdef main
	#undef main
#endif
int main(int argc, char** argv)
{ 
	// Not sure how to set this up, so fuck it
	Quaternion q1 = Quaternion::AngleAxis( Math::ToRadians( 90.0f ), Vec3::YAxis( ) ) * Quaternion::AngleAxis( Math::ToRadians( 40.0f ), Vec3::ZAxis( ) ) * Quaternion::AngleAxis( Math::ToRadians( -10.0f ), Vec3::XAxis( ) );
	glm::quat q2 = glm::angleAxis( Math::ToRadians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) ) * glm::angleAxis( Math::ToRadians( 40.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) ) * glm::angleAxis( Math::ToRadians( -10.0f ), glm::vec3( 1.0f, 0.0f, 0.0f ) );
 
	//std::cout << q1 << ", " << QuatToString( q2 ) << ", Equals: " << Equals(q1, q2) << "\n";

	for ( u32 i = 0; i < 100; ++i )
	{
		Quaternion d1 = q1 * Quaternion::AngleAxis( (f32)Math::ToRadians(i), Vec3::ZAxis( ) );
		glm::quat d2 = q2 * glm::angleAxis( glm::radians((f32)i), glm::vec3(0.0f, 0.0f, 1.0f) ); 

		Vec3 v1 = d1 * Vec3::YAxis();
		glm::vec3 v2 = d2 * glm::vec3( 0.0f, 1.0f, 0.0f );

		std::cout << v1 << ", " << Vec3ToString(v2) << ", Equals: " << Equals( v1, v2 ) << "\n";
	} 
	
	return 0;
}
