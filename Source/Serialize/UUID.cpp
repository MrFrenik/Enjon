// @file UUID.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

/*
The MIT License (MIT)

Copyright (c) 2014 Graeme Hill (http://graemehill.ca)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "System/Types.h"
#include "Serialize/UUID.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <iomanip>

// If windows
#ifdef ENJON_SYSTEM_WINDOWS 
	#include <objbase.h>
#endif

namespace Enjon
{
	//==================================================================== 

	unsigned char hexDigitToChar( char ch )
	{
		if ( ch > 47 && ch < 58 )
			return ch - 48;

		if ( ch > 96 && ch < 103 )
			return ch - 87;

		if ( ch > 64 && ch < 71 )
			return ch - 55;

		return 0;
	} 

	//==================================================================== 

	unsigned char hexPairToChar( char a, char b )
	{
		return hexDigitToChar( a ) * 16 + hexDigitToChar( b );
	}

	//==================================================================== 

	UUID::UUID( )
	{
		// Fill bytes with '0'
		for ( u32 i = 0; i < 16; ++i )
		{
			mBytes.push_back( '0' );
		}
	}

	//====================================================================

	UUID::UUID( const Enjon::String& string )
	{
		//GuidGenerator gen;
		//mID = Guid( string );

		mBytes.clear( );

		s8 charOne, charTwo;
		bool lookingForFirstChar = true;

		for ( const s8 &ch : string )
		{
			if ( ch == '-' )
				continue;

			if ( lookingForFirstChar )
			{
				charOne = ch;
				lookingForFirstChar = false;
			}
			else
			{
				charTwo = ch;
				auto byte = hexPairToChar( charOne, charTwo );
				mBytes.push_back( byte );
				lookingForFirstChar = true;
			}
		}
	}

	std::ostream &operator<<( std::ostream &s, const UUID &uuid )
	{
		return s << std::hex << std::setfill( '0' )
			<< std::setw( 2 ) << (s32)uuid.mBytes[0]
			<< std::setw( 2 ) << (s32)uuid.mBytes[1]
			<< std::setw( 2 ) << (s32)uuid.mBytes[2]
			<< std::setw( 2 ) << (s32)uuid.mBytes[3]
			<< "-"
			<< std::setw( 2 ) << (s32)uuid.mBytes[4]
			<< std::setw( 2 ) << (s32)uuid.mBytes[5]
			<< "-"
			<< std::setw( 2 ) << (s32)uuid.mBytes[6]
			<< std::setw( 2 ) << (s32)uuid.mBytes[7]
			<< "-"
			<< std::setw( 2 ) << (s32)uuid.mBytes[8]
			<< std::setw( 2 ) << (s32)uuid.mBytes[9]
			<< "-"
			<< std::setw( 2 ) << (s32)uuid.mBytes[10]
			<< std::setw( 2 ) << (s32)uuid.mBytes[11]
			<< std::setw( 2 ) << (s32)uuid.mBytes[12]
			<< std::setw( 2 ) << (s32)uuid.mBytes[13]
			<< std::setw( 2 ) << (s32)uuid.mBytes[14]
			<< std::setw( 2 ) << (s32)uuid.mBytes[15];
	}

	//====================================================================

	UUID::~UUID( )
	{
	}

	//====================================================================

	UUID::UUID( const u8* bytes )
	{
		mBytes.assign( bytes, bytes + 16 );
	}

	//====================================================================

	Enjon::String UUID::ToString( ) const
	{
		std::stringstream ss;
		ss << *this;
		//ss << mID;
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
		UUID uuid = NewUUID();
		//GuidGenerator gen;
		//uuid.mID = gen.newGuid( );
		return uuid; 
	}

	//====================================================================

	UUID UUID::Invalid( )
	{
		return UUID();
	} 

	//====================================================================

	UUID &UUID::operator=( const UUID &other )
	{
		mBytes = other.mBytes;
		return *this;
	}

	//====================================================================

	bool UUID::operator==( const UUID &other ) const
	{
		return mBytes == other.mBytes;
	}

	//====================================================================

	bool UUID::operator!=( const UUID &other ) const 
	{
		return !( ( *this ) == other );
	}

	//====================================================================

#ifdef ENJON_SYSTEM_WINDOWS
	UUID UUID::NewUUID( )
	{ 
		GUID newId;
		CoCreateGuid( &newId );

		u8 bytes[16] =
		{
			static_cast< u8 >( ( newId.Data1 >> 24 ) & 0xFF ),
			static_cast< u8 >( ( newId.Data1 >> 16 ) & 0xFF ),
			static_cast< u8 >( ( newId.Data1 >> 8 ) & 0xFF ),
			static_cast< u8 >( ( newId.Data1 ) & 0xff ),

			static_cast< u8 >( ( newId.Data2 >> 8 ) & 0xFF ),
			static_cast< u8 >( ( newId.Data2 ) & 0xff ),

			static_cast< u8 >( ( newId.Data3 >> 8 ) & 0xFF ),
			static_cast< u8 >( ( newId.Data3 ) & 0xFF ),

			static_cast< u8 >( newId.Data4[0] ),
			static_cast< u8 >( newId.Data4[1] ),
			static_cast< u8 >( newId.Data4[2] ),
			static_cast< u8 >( newId.Data4[3] ),
			static_cast< u8 >( newId.Data4[4] ),
			static_cast< u8 >( newId.Data4[5] ),
			static_cast< u8 >( newId.Data4[6] ),
			static_cast< u8 >( newId.Data4[7] )
		};

		return UUID(bytes); 
	}
#endif

#ifdef ENJON_SYSTEM_OSX
	#include "CoreFoundation/CFUUID.h"
	UUID UUID::NewUUID()
	{
		CFUUIDRef id = CFUUIDCreate( NULL );
	  	CFUUIDBytes bytes = CFUUIDGetUUIDBytes( id );
	  	CFRelease( id );
	  	u8 byte_arr[16] 
	  	{
	  			(u8)bytes.byte0,
	  			(u8)bytes.byte1,
	  			(u8)bytes.byte2,
	  			(u8)bytes.byte3,
	  			(u8)bytes.byte4,
	  			(u8)bytes.byte5,
	  			(u8)bytes.byte6,
	  			(u8)bytes.byte7,
	  			(u8)bytes.byte8,
	  			(u8)bytes.byte9,
	  			(u8)bytes.byte10,
	  			(u8)bytes.byte11,
	  			(u8)bytes.byte12,
	  			(u8)bytes.byte13,
	  			(u8)bytes.byte14,
	  			(u8)bytes.byte15
	  	};
	  	return UUID( byte_arr );
	}
#endif

	//====================================================================


}
