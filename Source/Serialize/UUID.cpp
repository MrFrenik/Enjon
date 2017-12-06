// @file UUID.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Serialize/UUID.h"

// If windows
#if _WIN32 
	#define GUID_WINDOWS
#elif _WIN64
	#define GUID_WINDOWS
#endif

namespace Enjon
{
	//====================================================================

	UUID::UUID( )
	{
	}

	//====================================================================

	UUID::UUID( const Enjon::String& string )
	{
		GuidGenerator gen;
		mID = Guid( string );
	}

	//====================================================================

	UUID::~UUID( )
	{
	}

	//====================================================================

	Enjon::String UUID::ToString( ) const
	{
		std::stringstream ss;
		ss << mID;
		return ss.str( );
	}

	//====================================================================

	UUID::operator bool( ) const
	{
		return ( ToString( ).compare( UUID::Invalid( ).ToString( ) ) != 0 );
	}

	//====================================================================

	UUID UUID::GenerateUUID( )
	{ 
		UUID uuid;
		GuidGenerator gen;
		uuid.mID = gen.newGuid( );
		return uuid; 
	}

	//====================================================================

	UUID UUID::Invalid( )
	{
		return UUID();
	} 
}
