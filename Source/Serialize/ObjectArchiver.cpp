// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ObjectArchiver.h

#include "Serialize/ObjectArchiver.h"
#include "Graphics/Color.h"													// Don't like this here, but I'll leave it for now
#include "Serialize/UUID.h"
#include "System/Types.h"
#include "Defines.h"
#include "Asset/AssetManager.h"
#include "Engine.h"

namespace Enjon
{ 
	//=====================================================================

	ObjectArchiver::ObjectArchiver( )
	{ 
	}

	//=====================================================================

	ObjectArchiver::~ObjectArchiver( )
	{ 
	}

	//===================================================================== 

	Result ObjectArchiver::Serialize( const Object* object )
	{
		// Get meta class from object
		const MetaClass* cls = object->Class( ); 

		// TODO(): Get versioning structure from meta class this for object...  
		/*
			const MetaClassVersionArchive* archive = cls->GetVersionArchive();
			buffer->Write( archive->GetClassName() );		
			buffer->Write( archive->GetVersionNumber() );
		*/

		// Write out class header information using meta class
		// Class name
		if ( cls )
		{ 
			mBuffer.Write< String >( cls->GetName( ) );
			// Make shift version number
			mBuffer.Write( 0 );		

			// Serialize all object specific data ( classes can override at this point how they want to serialize data )
			Result res = object->SerializeData( this );

			// Continue with default serialization if the object doesn't handle its own
			if ( res == Result::INCOMPLETE )
			{
				res = SerializeObjectDataDefault( object, cls );
			}

			// Final result of serialization
			return res;
		}

		// Shouldn't reach here
		return Result::FAILURE; 
	}

	//=====================================================================

#define WRITE_PROP( buffer, cls, object, prop, valType )\
	buffer.Write< valType >( *cls->GetValueAs< valType >( object, prop ) );

	Result ObjectArchiver::SerializeObjectDataDefault( const Object* object, const MetaClass* cls )
	{ 
		// Serialize all object properties
		for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
		{
			// Get property
			const MetaProperty* prop = cls->GetProperty( i );

			if ( !prop )
			{
				continue;
			}

			switch ( prop->GetType( ) )
			{
				case MetaPropertyType::U8:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, u8 )
				} break;

				case MetaPropertyType::U16:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, u16 )
				} break;

				case MetaPropertyType::U32:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, u32 )
				} break;

				case MetaPropertyType::U64:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, u64 )
				} break;
				
				case MetaPropertyType::S8:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, s8 )
				} break;

				case MetaPropertyType::S16:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, s16 )
				} break;

				case MetaPropertyType::S32:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, s32 )
				} break;

				case MetaPropertyType::S64:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, s64 )
				} break;

				case MetaPropertyType::F32:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, f32 )
				} break;

				case MetaPropertyType::F64:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, f64 )
				} break;

				case MetaPropertyType::Bool:
				{ 
					WRITE_PROP( mBuffer, cls, object, prop, bool )
				} break;

				case MetaPropertyType::ColorRGBA32:
				{ 
					// Get color
					ColorRGBA32 val = *cls->GetValueAs< ColorRGBA32 >( object, prop );

					// Write all individual color channels
					mBuffer.Write< f32 >( val.r );
					mBuffer.Write< f32 >( val.g );
					mBuffer.Write< f32 >( val.b );
					mBuffer.Write< f32 >( val.a );

				} break;

				case MetaPropertyType::String:
				{
					WRITE_PROP( mBuffer, cls, object, prop, String )
				} break;

				case MetaPropertyType::Vec2:
				{
					// Get vec2
					Vec2 val = *cls->GetValueAs< Vec2 >( object, prop );

					// Write individual elements of vec2
					mBuffer.Write< f32 >( val.x );
					mBuffer.Write< f32 >( val.y );
				} break;

				case MetaPropertyType::Vec3:
				{
					// Get vec3
					Vec3 val = *cls->GetValueAs< Vec3 >( object, prop );

					// Write individual elements of vec3
					mBuffer.Write< f32 >( val.x );
					mBuffer.Write< f32 >( val.y );
					mBuffer.Write< f32 >( val.z );
				} break;

				case MetaPropertyType::Vec4:
				{
					// Get vec3
					Vec4 val = *cls->GetValueAs< Vec4 >( object, prop );

					// Write individual elements of vec4
					mBuffer.Write< f32 >( val.x );
					mBuffer.Write< f32 >( val.y );
					mBuffer.Write< f32 >( val.z );
					mBuffer.Write< f32 >( val.w );
				} break;

				case MetaPropertyType::UUID:
				{
					WRITE_PROP( mBuffer, cls, object, prop, UUID )
				} break;

				case MetaPropertyType::AssetHandle:
				{ 
					// Get value of asset 
					AssetHandle<Asset> val; 
					cls->GetValue( object, prop, &val );

					// If valid asset, write its UUID
					if ( val )
					{
						mBuffer.Write( val.GetUUID() );
					} 
					// Otherwise write out invalid UUID
					else
					{
						mBuffer.Write( UUID::Invalid( ) );
					} 
				} break;
			}
		} 

		return Result::SUCCESS;
	}

	//=====================================================================


	Result ObjectArchiver::Deserialize( const String& filePath, Vector< Object* >& out )
	{
		// Reset all data before de-serializing
		Reset( );

		// Read contents into buffer
		mBuffer.ReadFromFile( filePath ); 

		// Header information 
		const MetaClass* cls = Object::GetClass( mBuffer.Read< String >( ) );	// Read class type
		u32 versionNumber = mBuffer.Read< u32 >( );								// Read version number id

		if ( cls )
		{ 
			// Construct new object based on class
			Object* object = cls->Construct( );

			// Couldn't construct object
			if ( !object )
			{
				delete object;
				object = nullptr;
				return Result::FAILURE;
			}

			// Push back object into objects vector
			out.push_back( object );

			// Fill out its properties ( // Perhaps, in order to be somewhat "backwards compatible", 
			// instead of contiguously iterating over each property and assuming that it lines up with 
			// the cached object file should instead search for the property by name using the meta class
			// and then fill it out using that - will be trickier to implement of course
			// need to know where to stop with de-serializing data...
			// Should I serialize a json object then? That seems redundant as hell.  
			
			/*
				... Started doing object data de-serialization after header...

				usize propertyCount = mBuffer.Read< usize > (); 
				for ( usize i = 0; i < propertyCount; ++i )
				{
					// Read the property name from the buffer
					String propName = mBuffer.Read< String >();

					// Read the size of the property ( used for skipping data in buffer if property doesn't exist anymore )
					usize propDataSize = mBuffer.Read< usize >();

					// Get the property from the meta class
					const MetaClass* prop = cls->GetPropertyByName( propName );

					// If property exists
					if ( prop )
					{
						// Find its type to set properly
						switch ( prop->GetType() )
						{ 
							...
							// Assume its a u32 and read into class
							case MetaPropertyType::U32:
							{
								u32 val = mBuffer.Read< u32 >();\
								cls->SetValue( object, prop, val );
							} break;
						}
					} 
					// If doesn't exist, we need to skip over some data to realign ourselves
					else
					{
						mBuffer.SkipAhead( propDataSize );
					}
				}
			*/

			Result res = object->DeserializeData( this );

			if ( res == Result::INCOMPLETE )
			{
				DeserializeObjectDataDefault( object, cls );
			} 
		}

		// How do I want binary files to be structured?
		// Get class type at top of class?
		/*
			Header:
				ClassType(String)
				VerisonNumber(u32)
			ObjData:
				NumProperties() - could get this from the class itself, but it's safer to store this in the binary and use that 
				For each property:
					PropertyName() - used to search for property ( could use for migrating data versions as well )
						- What if not found? Should the type also be stored so that the buffer can skip the upcoming data?
						- What if the data is another Object type? Would be more useful to store the amount of bytes required
						- Yeah, but now each binary file is being inflated with a String as well as a u32 for each property...
						- Could have a separate binary file that's used to describe what this class is, but that's more file dependencies than I'd like
					PropertyValue() - used to fill out property 
		*/

		return Result::SUCCESS;
	} 

	//=====================================================================

	Object* ObjectArchiver::Deserialize( const String& filePath )
	{
		// Reset the buffer
		Reset( );

		// Read contents into buffer
		mBuffer.ReadFromFile( filePath );
 
		// Object Header information 
		const MetaClass* cls = Object::GetClass( mBuffer.Read< String >( ) );	// Read class type
		u32 versionNumber = mBuffer.Read< u32 >( );								// Read version number id

		// Object to construct and fill out
		Object* object = nullptr;

		if ( cls )
		{
			// Construct new object based on class
			object = cls->Construct( );

			// Couldn't construct object
			if ( !object )
			{
				delete object;
				object = nullptr;
				return nullptr;
			} 
			// Successfully constructed, now deserialize data into it
			else
			{
				Result res = object->DeserializeData( this );

				// Default deserialization method if not object does not handle its own deserialization
				if ( res == Result::INCOMPLETE )
				{
					res = DeserializeObjectDataDefault( object, cls );
				} 

				if ( res != Result::SUCCESS )
				{
					delete object;
					object = nullptr;
				} 
			} 
		}

		// Return object, either null or filled out
		return object; 
	}

	//=====================================================================

	Result ObjectArchiver::Deserialize( const String& filePath, HashMap< const MetaClass*, Object* >& out )
	{
		return Result::SUCCESS;
	}

	//=====================================================================

#define READ_PROP( buffer, cls, object, prop, valType )\
	valType val = buffer.Read< valType >();\
	cls->SetValue(object, prop, val);

	Result ObjectArchiver::DeserializeObjectDataDefault( const Object* object, const MetaClass* cls )
	{
		for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
		{
			// Grab property at index from metaclass
			MetaProperty* prop = const_cast<MetaProperty*> ( cls->GetProperty( i ) );

			if ( !prop )
			{
				continue;
			}

			switch ( prop->GetType( ) )
			{
				case MetaPropertyType::U8:
				{
					READ_PROP( mBuffer, cls, object, prop, u8 )
				} break;

				case MetaPropertyType::U16:
				{
					READ_PROP( mBuffer, cls, object, prop, u16 )
				} break;

				case MetaPropertyType::U32:
				{
					// Set value of object from read buffer
					READ_PROP( mBuffer, cls, object, prop, u32 )
				} break;

				case MetaPropertyType::U64:
				{
					// Set value of object from read buffer
					READ_PROP( mBuffer, cls, object, prop, u64 )
				} break;

				case MetaPropertyType::F32:
				{
					// Set value of object from read buffer
					READ_PROP( mBuffer, cls, object, prop, f32 )
				} break;

				case MetaPropertyType::String:
				{
					READ_PROP( mBuffer, cls, object, prop, String )
				} break;

				case MetaPropertyType::S8:
				{
					READ_PROP( mBuffer, cls, object, prop, s8 )
				} break;

				case MetaPropertyType::S16:
				{
					READ_PROP( mBuffer, cls, object, prop, s16 )
				} break;

				case MetaPropertyType::S32:
				{
					READ_PROP( mBuffer, cls, object, prop, s32 )
				} break;

				case MetaPropertyType::S64:
				{
					READ_PROP( mBuffer, cls, object, prop, s64 )
				} break;

				case MetaPropertyType::UUID:
				{
					READ_PROP( mBuffer, cls, object, prop, UUID )
				} break;

				case MetaPropertyType::Bool:
				{
					READ_PROP( mBuffer, cls, object, prop, bool )
				} break;

				case MetaPropertyType::AssetHandle:
				{
					// Grab asset manager
					const MetaPropertyTemplateBase* base = prop->Cast< MetaPropertyTemplateBase >();
					AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
					AssetHandle<Asset> val;
					cls->GetValue( object, prop, &val );

					// Get meta class of the asset
					const MetaClass* assetCls = base->GetClassOfTemplatedArgument( );

					// Get uuid from read buffer
					UUID id = mBuffer.Read< UUID >( );

					// Get asset
					const Asset* asset = am->GetAsset( assetCls, id );

					// If valid asset
					if ( asset )
					{
						// Set asset handle to default asset
						val.Set( const_cast< Asset*> ( asset ) );

					}
					// Otherwise get default asset for this class type
					else
					{
						val.Set( am->GetDefaultAsset( assetCls ) );
					}

					// Set value of object
					cls->SetValue( object, prop, val );

				} break;

				case MetaPropertyType::Vec2:
				{
					// Read individual elements of Vec2
					f32 x = mBuffer.Read< f32 >( );
					f32 y = mBuffer.Read< f32 >( );

					// Set Vec2 property
					cls->SetValue( object, prop, Vec2( x, y ) );
				} break;

				case MetaPropertyType::Vec3:
				{
					// Read individual elements of Vec3
					f32 x = mBuffer.Read< f32 >( );
					f32 y = mBuffer.Read< f32 >( );
					f32 z = mBuffer.Read< f32 >( );

					// Set Vec3 property
					cls->SetValue( object, prop, Vec3( x, y, z ) );
				} break;

				case MetaPropertyType::Vec4:
				{
					// Read individual elements of Vec4
					f32 x = mBuffer.Read< f32 >( );
					f32 y = mBuffer.Read< f32 >( );
					f32 z = mBuffer.Read< f32 >( );
					f32 w = mBuffer.Read< f32 >( );

					// Set Vec4 property
					cls->SetValue( object, prop, Vec4( x, y, z, w ) );
				} break;

				case MetaPropertyType::ColorRGBA32:
				{
					// Read all individual color channels
					f32 r = mBuffer.Read< f32 >( );
					f32 g = mBuffer.Read< f32 >( );
					f32 b = mBuffer.Read< f32 >( );
					f32 a = mBuffer.Read< f32 >( );

					// Set ColorRGBA32 property
					cls->SetValue( object, prop, ColorRGBA32( r, g, b, a ) );
				} break;
			}
		}

		return Result::SUCCESS;
	}

	//===========================================================

	Result ObjectArchiver::WriteToFile( const String& filePath )
	{ 
		mBuffer.WriteToFile( filePath ); 
		return Result::SUCCESS;
	}

	//=====================================================================

	void ObjectArchiver::Reset( )
	{
		mBuffer.Reset( );
	} 

	//=====================================================================
}
