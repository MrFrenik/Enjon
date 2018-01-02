// Project include
#include "TestProject.h"

// Module definition for this project
ENJON_MODULE_IMPL( TestProject ) 

// Engine includes
#include <Enjon.h> 

using namespace Enjon; 

// ==========================================================================================

Result TestProject::Initialize( )
{ 
	return Enjon::Result::SUCCESS;
}

Result TestProject::Update( Enjon::f32 dt )
{ 
	return Result::PROCESS_RUNNING;
}

Result TestProject::ProcessInput( Enjon::f32 dt )
{
	return Enjon::Result::SUCCESS; 
}

Result TestProject::Shutdown( )
{ 
	return Result::SUCCESS;
}














Enjon::Result TestProject::SerializeData( Enjon::ByteBuffer* buffer ) const
{
	const MetaClass* cls = this->Class( );

	// Write out property count to buffer
	buffer->Write< usize >( cls->GetPropertyCount( ) );

	// Iterate over properties and write out
	for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
	{
		// Get property
		const MetaProperty* prop = cls->GetProperty( i );

		if ( !prop )
		{
			continue;
		}

		// Write out property name
		buffer->Write< String >( prop->GetName( ) );

		// Write out property type
		buffer->Write< s32 >( (s32)prop->GetType( ) ); 

		switch ( prop->GetType( ) )
		{
			case MetaPropertyType::F32:
			{
				buffer->Write< usize >( sizeof( f32 ) );
				buffer->Write< f32 >( *cls->GetValueAs< f32 >( this, prop ) );
			} break;

			case MetaPropertyType::Vec3:
			{
				buffer->Write< usize >( sizeof( Vec3 ) ); 
				Vec3 v = *cls->GetValueAs< Vec3 >( this, prop );
				buffer->Write< f32 >( v.x );
				buffer->Write< f32 >( v.y );
				buffer->Write< f32 >( v.z );
			} break;

			case MetaPropertyType::EntityHandle:
			{ 
				// Need size, so create another ByteBuffer and serialize once, then get the size of the buffer after the write 
				// Better way would be to be able to write, calculate the delta, shift the buffer over by writing this BEFORE 
				// serializing out the entity, but I can worry about that later...
				ByteBuffer temp;
				EntityArchiver::Serialize( *cls->GetValueAs< EntityHandle >( this, prop ), &temp );
				usize bufferSize = temp.GetSize( ); 
				buffer->Write< usize >( bufferSize );

				// Serialize entity data
				EntityArchiver::Serialize( *cls->GetValueAs< EntityHandle >( this, prop ), buffer ); 
			} break;

			case MetaPropertyType::AssetHandle:
			{
				ByteBuffer temp;
				temp.Write< UUID >( UUID::Invalid( ) );
				buffer->Write< usize >( temp.GetSize( ) );

				AssetHandle<Asset> val;
				cls->GetValue( this, prop, &val );
				if ( val )
				{
					buffer->Write( val.GetUUID( ) );
				}
				else
				{
					buffer->Write( UUID::Invalid( ) );
				}
			} break;
		} 
	}

	return Enjon::Result::SUCCESS;
}

Enjon::Result TestProject::DeserializeData( Enjon::ByteBuffer* buffer )
{
	// Get property count
	usize propCount = buffer->Read< usize >( );

	const MetaClass* cls = this->Class( );

	for ( usize i = 0; i < propCount; ++i )
	{
		const MetaProperty* prop = cls->GetPropertyByName( buffer->Read< String >( ) );
		MetaPropertyType propType = ( MetaPropertyType )buffer->Read< s32 >( ); 
		usize propSize = buffer->Read< usize >( );

		// Can proceed to deserialize IFF the property exists and its type is correct 
		if ( prop && propType == prop->GetType() )
		{ 
			switch ( propType )
			{
				case MetaPropertyType::F32:
				{
					cls->SetValue( this, prop, buffer->Read< f32 >() );
				} break;

				case MetaPropertyType::Vec3:
				{
					f32 x = buffer->Read< f32 >( );
					f32 y = buffer->Read< f32 >( );
					f32 z = buffer->Read< f32 >( );
					cls->SetValue( this, prop, Vec3( x, y, z ) ); 
				} break;

				case MetaPropertyType::EntityHandle:
				{
					EntityHandle handle = EntityArchiver::Deserialize( buffer );
					cls->SetValue( this, prop, handle ); 
				} break;

				case MetaPropertyType::AssetHandle:
				{
					// Grab asset manager
					const MetaPropertyTemplateBase* base = prop->Cast< MetaPropertyTemplateBase >( );
					const AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
					AssetHandle<Asset> val;

					// Get meta class of the asset
					const MetaClass* assetCls = base->GetClassOfTemplatedArgument( );

					// Get uuid from read buffer
					UUID id = buffer->Read< UUID >( );

					// Get asset
					const Asset* asset = am->GetAsset( assetCls, id );

					// If valid asset
					if ( asset )
					{
						// Set asset handle to default asset
						val.Set( asset );

					}
					// Otherwise get default asset for this class type
					else
					{
						val.Set( am->GetDefaultAsset( assetCls ) );
					}

					// Set value of object
					cls->SetValue( this, prop, val );
				} break;

			} 
		}
		// Otherwise property doesn't exist, so yeah...
		else
		{
			// Error... must skip ahead in buffer by size provided by type
			buffer->AdvanceReadPosition( propSize ); 
		}
	} 

	return Enjon::Result::SUCCESS; 
}

