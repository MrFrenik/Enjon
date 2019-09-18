#include "Serialize/JSONBuffer.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <fstream>
#include <sstream> 
#include <algorithm>

using namespace rapidjson;

#define STRSIZE( string ) static_cast< SizeType >( string.length() ) 

namespace Enjon
{
	//============================================================================================

	JSONBuffer::JSONBuffer()
	{ 
	}

	//============================================================================================

	JSONBuffer::JSONBuffer( const String& filePath )
	{ 
		// Load file
		std::ifstream f;
		f.open( filePath );
		std::stringstream buffer;
		buffer << f.rdbuf( );

		Enjon::String str = buffer.str( );

		Document document;
		if ( document.Parse( str.c_str( ), STRSIZE( str ) ).HasParseError( ) )
		{
			auto parseError = document.Parse( str.c_str( ), STRSIZE( str ) ).GetParseError( );
			std::cout << parseError << "\n";
		}

		//mDocument = document;
	}

	//============================================================================================

	JSONBuffer::~JSONBuffer()
	{ 
	}

	//============================================================================================

	// This doesn't really make sense here, unfortunately...I just need an archiver for this
	template < typename T >
	T Read()
	{ 
	}

	//============================================================================================

	template < typename T >
	void Write( const T& val )
	{ 
	} 

	//============================================================================================
}

/*
	ENJON_CLASS( Construct )
	class TestObject : public Object
	{
		ENJON_CLASS_BODY( TestObject ) 
		public:
			ENJON_PROPERTY( )
			u32 mU32Val;

			ENJON_PROPERTY( )
			String mStringVal;

			ENJON_PROPERTY( )
			f32 mFloatVal;

			ENJON_PROPERTY( )
			Vector< u32 > mVector;
	} 

	JSON serialization: 

	{
		"Class": "TestObject", 
		"PropertyCount": 4,
		"Properties": [
			"mU32Val": {
				"Type": "u32" 
			},
			"mStringVal": {
				"Type": "String"
			},
			"mFloatVal": {
				"Type": "f32"
			},
			"mVector": {
				"Type": "Array",
				"ArrayType": "String",
				"ArrayLength": 3,
				"Contents": [
					"Blah", 
					"Another String",
					"And Another..."
				]
			}
		]
	}
*/

/*
	ObjectArchiver::Serialize( const Object* object, JSONBuffer* buffer )
	{
		// Check for default serialization method here... I suppose? 

		// Write out object into buffer
		buffer->BeginObject(); 
		{ 
			const MetaClass* cls = object->Class();
			for ( u32 i = 0; i < cls->GetPropertyCount(); ++i )
			{
				const MetaProperty* prop = cls->GetProperty( i );		
				switch ( prop->GetType() )
				{ 
				}
			}
		}
		buffer->EndObject(); 
	}
*/
