
#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectBinarySerializer.h"

namespace Enjon
{
	//=========================================================================

	Result Object::Serialize( ByteBuffer* buffer ) const
	{ 
		// Get meta class
		const MetaClass* cls = this->Class( );

		// TODO(): Get versioning structure from meta class this for object...  
		/*
			const MetaClassVersionArchive* archive = cls->GetVersionArchive();
			buffer->Write( archive->GetClassName() );		
			buffer->Write( archive->GetVersionNumber() );
		*/

		// Write out class header information using meta class
		buffer->Write( cls->GetName( ) );
		buffer->Write( 0 );		// Make shift version number

		// Serialize all object specific data ( classes can override at this point how they want to serialize data )
		Result res = SerializeData( buffer );

		return res;
	}

	//========================================================================= 

	Result Object::Deserialize( ByteBuffer* buffer ) const
	{ 
		// How exactly is this supposed to work? 
		// I'm supposed to have the constructed class ready to be deserialized at this point, right?

		// Get meta class
		const MetaClass* cls = this->Class( ); 

		// Write out class header information using meta class
		buffer->Write( cls->GetName( ) );
		buffer->Write( 0 );		// Make shift version number

		// Serialize all object specific data ( classes can override at this point how they want to serialize data )
		Result res = DeserializeData( buffer );

		return res;
	}

	//=========================================================================

	Result Object::SerializeData( ByteBuffer* buffer ) const
	{ 
		ObjectArchiver arhiver;
		arhiver.Serialize( this ); 

		return Result::SUCCESS;
	}

	//=========================================================================

	Result Object::DeserializeData( ByteBuffer* buffer ) const
	{
		return Result::SUCCESS; 
	}

	//=========================================================================
}

/*
	AssetHandle< Texture > mTexture;
	mTexture.Serialize();
*/