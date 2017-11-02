// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ObjectBinarySerializer.h

#include "Serialize/ObjectBinarySerializer.h"
#include "Graphics/Color.h"													// Don't like this here, but I'll leave it for now
#include "Serialize/UUID.h"
#include "System/Types.h"
#include "Defines.h"
#include "Asset/AssetManager.h"
#include "Engine.h"

namespace Enjon
{
	//=====================================================================

	ObjectBinarySerializer::ObjectBinarySerializer( )
	{ 
	}

	//=====================================================================

	ObjectBinarySerializer::~ObjectBinarySerializer( )
	{ 
	}

	//=====================================================================

#define WRITE_PROP( buffer, cls, object, prop, valType )\
	buffer.Write< valType >( *cls->GetValueAs< valType >( object, prop ) );

	Result ObjectBinarySerializer::Serialize( const Object* object )
	{
		// Get meta class from object
		const MetaClass* cls = object->Class( ); 

		if ( cls )
		{
			// Write class type as string
			mBuffer.Write< String >( cls->GetName( ) );

			// Serialize all object properties
			for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
			{
				// Get property
				const MetaProperty* prop = cls->GetProperty( i );

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

		// Shouldn't reach here
		return Result::FAILURE;
	}

	//===================================================================== 
	
	Result ObjectBinarySerializer::WriteToFile( const String& filePath )
	{
		mBuffer.WriteToFile( filePath );

		return Result::SUCCESS;
	}

	//===================================================================== 

	ObjectBinaryDeserializer::ObjectBinaryDeserializer( )
	{ 
	}

	//===================================================================== 

	void ObjectBinaryDeserializer::ReleaseData( )
	{
		// Release memory for all objects
		for ( auto& v : mObjects )
		{
			for ( auto& o : v.second )
			{
				delete o;
				o = nullptr; 
			}

			// Clear vector
			v.second.clear( );
		}

		// Clear objects vector
		mObjects.clear( ); 
	}

	//===================================================================== 

	void ObjectBinaryDeserializer::Reset( )
	{
		// Release object data
		ReleaseData( );

		// Reset read buffer
		mBuffer.Reset( );
	}

	//===================================================================== 

	ObjectBinaryDeserializer::~ObjectBinaryDeserializer( )
	{ 
		ReleaseData( );
	}

	//===================================================================== 

#define READ_PROP( buffer, cls, object, prop, valType )\
	valType val = buffer.Read< valType >();\
	cls->SetValue(object, prop, val);

	Result ObjectBinaryDeserializer::Deserialize( const String& filePath, Vector< Object* >& objectsOut )
	{ 
		// Reset all data before de-serializing
		Reset( );

		mBuffer.ReadFromFile( filePath ); 

		// Read class type
		const MetaClass* cls = Object::GetClass( mBuffer.Read< String >( ) ); 

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
			objectsOut.push_back( object );

			// Fill out its properties
			for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
			{
				// Grab property at index from metaclass
				MetaProperty* prop = const_cast<MetaProperty*> ( cls->GetProperty( i ) );

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
						const MetaPropertyTemplateBase* base = static_cast<MetaPropertyTemplateBase*>( prop );
						AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
						AssetHandle<Asset> val;
						cls->GetValue( object, prop, &val );

						// Get meta class of the asset
						const MetaClass* assetCls = const_cast<MetaClass*>( base->GetClassOfTemplatedArgument( ) );

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
		}

		// How do I want binary files to be structured?
		// Get class type at top of class?
		/*
			Header:
				ClassType(String)
				VerisonNumber(u32)
		*/

		return Result::SUCCESS;
	}

	//===================================================================== 
}
