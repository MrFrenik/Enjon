
#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"

namespace Enjon
{ 
	//=========================================================================

	Result Object::SerializeData( ObjectArchiver* archive ) const
	{ 
		return Result::INCOMPLETE;
	}

	//=========================================================================

	Result Object::DeserializeData( ObjectArchiver* archive ) const
	{
		return Result::INCOMPLETE; 
	}

	//=========================================================================
}

/*
	AssetHandle< Texture > mTexture;
	mTexture.Serialize();

	ObjectArchiver archive;
	for ( auto& object : mObjects ) 
	{
		archive.Serialize(object);	
	}

	ObjectArchiver::Serialize( const Object* object )
	{
		// Need to get header data of object			
		Result res = object->Serialize();

		if ( res == Result::INCOMPLETE )
		{
			// Serialize data by default method...
		}
	}
*/














