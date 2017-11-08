
#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"

namespace Enjon
{
	//=========================================================================

	Result Object::Serialize( ObjectArchiver* archiver ) const
	{ 
		// Just do this in the archiver...
		Result res = archiver->Serialize( this ); 

		return res;
	}

	//========================================================================= 

	Result Object::Deserialize( ObjectArchiver* archiver ) const
	{ 
		// How exactly is this supposed to work? 
		// I'm supposed to have the constructed class ready to be deserialized at this point, right?

		// Get meta class
		const MetaClass* cls = this->Class( ); 

		// Write out class header information using meta class
		//archiver->WriteRaw( cls->GetName( ) );
		//archiver->WriteRaw( 0 );		// Make shift version number

		// Serialize all object specific data ( classes can override at this point how they want to serialize data )
		Result res = DeserializeData( archiver );

		return res;
	}

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














