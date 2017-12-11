
// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ObjectArchiver.h

#include "Serialize/ObjectArchiver.h"
#include "Graphics/Color.h"													// Don't like this here, but I'll leave it for now
#include "Serialize/UUID.h"
#include "System/Types.h"
#include "Defines.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
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
			Result res = object->SerializeData( &mBuffer );

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
					const ColorRGBA32* val = cls->GetValueAs< ColorRGBA32 >( object, prop );

					// Write all individual color channels
					mBuffer.Write< f32 >( val->r );
					mBuffer.Write< f32 >( val->g );
					mBuffer.Write< f32 >( val->b );
					mBuffer.Write< f32 >( val->a );

				} break;

				case MetaPropertyType::String:
				{
					WRITE_PROP( mBuffer, cls, object, prop, String )
				} break;

				case MetaPropertyType::Vec2:
				{
					// Get vec2
					const Vec2* val = cls->GetValueAs< Vec2 >( object, prop );

					// Write individual elements of vec2
					mBuffer.Write< f32 >( val->x );
					mBuffer.Write< f32 >( val->y );
				} break;

				case MetaPropertyType::Vec3:
				{
					// Get vec3
					const Vec3* val = cls->GetValueAs< Vec3 >( object, prop );

					// Write individual elements of vec3
					mBuffer.Write< f32 >( val->x );
					mBuffer.Write< f32 >( val->y );
					mBuffer.Write< f32 >( val->z );
				} break;

				case MetaPropertyType::Vec4:
				{
					// Get vec3
					const Vec4* val = cls->GetValueAs< Vec4 >( object, prop );

					// Write individual elements of vec4
					mBuffer.Write< f32 >( val->x );
					mBuffer.Write< f32 >( val->y );
					mBuffer.Write< f32 >( val->z );
					mBuffer.Write< f32 >( val->w );
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

				case MetaPropertyType::Enum:
				{ 
					// Get integral value of enum
					mBuffer.Write( *cls->GetValueAs< s32 >( object, prop ) ); 
				} break;

# define WRITE_ARRAY_PROP_PRIM( object, prop, valType, buffer )\
	{\
		const MetaPropertyArray< valType >* arrayProp = prop->Cast< MetaPropertyArray< valType > >();\
		for ( usize j = 0; j < arrayProp->GetSize( object ); ++j )\
		{\
			buffer.Write< valType >( arrayProp->GetValueAs( object, j ) );\
		}\
	} 
				case MetaPropertyType::Array:
				{ 
					// Get base
					const MetaPropertyArrayBase* base = prop->Cast< MetaPropertyArrayBase >( );

					// Write out size of array to buffer
					mBuffer.Write< usize >( base->GetSize( object ) );

					// Write out array elements
					switch ( base->GetArrayType( ) )
					{
						case MetaPropertyType::Bool:	WRITE_ARRAY_PROP_PRIM( object, base, bool, mBuffer )	break;
						case MetaPropertyType::U8:		WRITE_ARRAY_PROP_PRIM( object, base, u8, mBuffer )		break;
						case MetaPropertyType::U32:		WRITE_ARRAY_PROP_PRIM( object, base, u32, mBuffer )		break;
						case MetaPropertyType::S32:		WRITE_ARRAY_PROP_PRIM( object, base, s32, mBuffer )		break;
						case MetaPropertyType::F32:		WRITE_ARRAY_PROP_PRIM( object, base, f32, mBuffer )		break;
						case MetaPropertyType::F64:		WRITE_ARRAY_PROP_PRIM( object, base, f64, mBuffer )		break;
						case MetaPropertyType::String:	WRITE_ARRAY_PROP_PRIM( object, base, String, mBuffer )	break;
						case MetaPropertyType::UUID:	WRITE_ARRAY_PROP_PRIM( object, base, UUID, mBuffer )	break; 

						case MetaPropertyType::AssetHandle:
						{
							// Write out asset uuids in array
							const MetaPropertyArray< AssetHandle< Asset > >* arrProp = base->Cast< MetaPropertyArray< AssetHandle< Asset > > >( );
							for ( usize j = 0; j < arrProp->GetSize( object ); ++j )
							{
								AssetHandle<Asset> asset;
								arrProp->GetValueAt( object, j, &asset );
								if ( asset )
								{
									mBuffer.Write< UUID >( asset->GetUUID() );
								}
								else
								{
									mBuffer.Write< UUID >( UUID::Invalid( ) );
								}
							} 
						} break;
					} 
				} break;

#define WRITE_MAP_KEY_PRIM_VAL_PRIM( object, prop, keyType, valType, buffer )\
	{\
		const MetaPropertyHashMap< keyType, valType >* mapProp = prop->Cast< MetaPropertyHashMap< keyType, valType > >();\
		for ( auto iter = mapProp->Begin( object ); iter != mapProp->End( object ); ++iter )\
		{\
			buffer.Write< keyType >( iter->first );\
			buffer.Write< valType >( iter->second );\
		}\
	} 

#define WRITE_MAP_KEY_PRIM_VAL_OBJECT( object, prop, keyType, buffer )\
	{\
		const MetaPropertyHashMap< keyType, Object* >* mapProp = prop->Cast< MetaPropertyHashMap< keyType, Object* > >();\
		for ( auto iter = mapProp->Begin( object ); iter != mapProp->End( object ); ++iter )\
		{\
			/*Write Key*/\
			buffer.Write< keyType >( iter->first );\
			/*Write Value*/\
			Serialize( iter->second );\
		}\
	}
				case MetaPropertyType::HashMap:
				{ 
					// Get base
					const MetaPropertyHashMapBase* base = prop->Cast< MetaPropertyHashMapBase >( );

					// Write out size of map to buffer
					mBuffer.Write< usize >( base->GetSize( object ) );

					switch ( base->GetKeyType( ) )
					{
						case MetaPropertyType::U32:
						{
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::U32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, u32, mBuffer )	break;
								case MetaPropertyType::S32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, s32, mBuffer )	break;
								case MetaPropertyType::F32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, f32, mBuffer )	break; 
							}
						} break;

						case MetaPropertyType::String:
						{
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::U32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, String, u32, mBuffer )	break; 
								case MetaPropertyType::Object:	WRITE_MAP_KEY_PRIM_VAL_OBJECT( object, base, String, mBuffer )		break;
							}
						} break;

						case MetaPropertyType::Enum:
						{ 
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::String:	WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, s32, String, mBuffer )	break;
								case MetaPropertyType::Object:	WRITE_MAP_KEY_PRIM_VAL_OBJECT( object, base, s32, mBuffer )			break;
							}
						} break;
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

			Result res = object->DeserializeData( &mBuffer );

			if ( res == Result::INCOMPLETE )
			{
				DeserializeObjectDataDefault( object, cls );
			} 
		}

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
				Result res = object->DeserializeData( &mBuffer );

				// Default deserialization method if not object does not handle its own deserialization
				if ( res == Result::INCOMPLETE )
				{
					res = DeserializeObjectDataDefault( object, cls );
				} 

				// Delete object if not deserialized correctly
				if ( res != Result::SUCCESS )
				{
					delete object;
					object = nullptr;
				} 
				// Otherwise call late init after deserializing
				else
				{
					object->DeserializeLateInit( );
				}
			} 
		}

		// Return object, either null or filled out
		return object; 
	}

	//=====================================================================

	Result ObjectArchiver::Deserialize( const String& filePath, HashMap< const MetaClass*, Vector< Object* > >& out )
	{
		return Result::SUCCESS;
	}

	//=====================================================================

	Object* ObjectArchiver::Deserialize( ByteBuffer* buffer ) 
	{ 
		// Object Header information 
		const MetaClass* cls = Object::GetClass( buffer->Read< String >( ) );	// Read class type
		u32 versionNumber = buffer->Read< u32 >( );								// Read version number id

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
				Result res = object->DeserializeData( &mBuffer );

				// Default deserialization method if not object does not handle its own deserialization
				if ( res == Result::INCOMPLETE )
				{
					res = DeserializeObjectDataDefault( object, cls );
				} 

				// Delete object if not deserialized correctly
				if ( res != Result::SUCCESS )
				{
					delete object;
					object = nullptr;
				}
				// Otherwise call late init after deserializing
				else
				{
					object->DeserializeLateInit( );
				}
			} 
		}

		// Return object, either null or filled out
		return object; 
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
					const AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
					AssetHandle<Asset> val;

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
						val.Set( asset );

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

				case MetaPropertyType::Enum:
				{
					// Read value from buffer
					s32 val = mBuffer.Read< s32 >( );

					// Set property on object
					cls->SetValue( object, prop, val );

				} break;

# define READ_ARRAY_PROP_PRIM( object, prop, valType, arraySize, buffer )\
	{\
		const MetaPropertyArray< valType >* arrayProp = prop->Cast< MetaPropertyArray< valType > >();\
		for ( usize j = 0; j < arraySize; ++j )\
		{\
			/*Grab value from buffer and set at index in array*/\
			arrayProp->SetValueAt( object, j, mBuffer.Read< valType >( ) );\
		}\
	} 
				case MetaPropertyType::Array:
				{ 
					// Get base
					const MetaPropertyArrayBase* base = prop->Cast< MetaPropertyArrayBase >( );

					// Read size of array from buffer
					usize arraySize = mBuffer.Read< usize >( );

					// If a dynamic vector then need to resize vector to allow for placement
					switch ( base->GetArraySizeType( ) )
					{
						case ArraySizeType::Dynamic:
						{ 
							base->Resize( object, arraySize );
						} break;
					}

					// Read out array elements
					switch ( base->GetArrayType( ) )
					{
						case MetaPropertyType::Bool:	READ_ARRAY_PROP_PRIM( object, base, bool, arraySize, mBuffer )		break;
						case MetaPropertyType::U8:		READ_ARRAY_PROP_PRIM( object, base, u8, arraySize, mBuffer )		break;
						case MetaPropertyType::U32:		READ_ARRAY_PROP_PRIM( object, base, u32, arraySize, mBuffer )		break;
						case MetaPropertyType::S32:		READ_ARRAY_PROP_PRIM( object, base, s32, arraySize, mBuffer )		break;
						case MetaPropertyType::F32:		READ_ARRAY_PROP_PRIM( object, base, f32, arraySize, mBuffer )		break;
						case MetaPropertyType::F64:		READ_ARRAY_PROP_PRIM( object, base, f64, arraySize, mBuffer )		break;
						case MetaPropertyType::String:	READ_ARRAY_PROP_PRIM( object, base, String, arraySize, mBuffer )	break;
						case MetaPropertyType::UUID:	READ_ARRAY_PROP_PRIM( object, base, UUID, arraySize, mBuffer )		break; 
						case MetaPropertyType::AssetHandle:
						{ 
							MetaArrayPropertyProxy proxy = base->GetProxy( ); 
							const MetaPropertyTemplateBase* arrBase = static_cast<const MetaPropertyTemplateBase*> ( proxy.mArrayPropertyTypeBase );
							const MetaClass* assetCls = const_cast<Enjon::MetaClass*>( arrBase->GetClassOfTemplatedArgument( ) ); 

							// Write out asset uuids in array
							const MetaPropertyArray< AssetHandle< Asset > >* arrProp = base->Cast< MetaPropertyArray< AssetHandle< Asset > > >( );
							for ( usize j = 0; j < arraySize; ++j )
							{
								AssetHandle<Asset> newAsset = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->GetAsset( assetCls, mBuffer.Read< UUID >( ) );
								arrProp->SetValueAt( object, j, newAsset );
							} 
						} break;
					}

				} break;


#define READ_MAP_KEY_PRIM_VAL_PRIM( object, prop, keyType, valType, mapSize, buffer )\
	{\
		const MetaPropertyHashMap< keyType, valType >* mapProp = prop->Cast< MetaPropertyHashMap< keyType, valType > >();\
		for ( usize j = 0; j < mapSize; ++j )\
		{\
			/*Read Key*/\
			keyType key = buffer.Read< keyType >( );\
			/*Read Value*/\
			valType val = buffer.Read< valType >( );\
			/*Set Value at key*/\
			mapProp->SetValueAt( object, key, val );\
		}\
	} 

#define READ_MAP_KEY_PRIM_VAL_OBJ( object, prop, keyType, mapSize, buffer )\
	{\
		const MetaPropertyHashMap< keyType, Object* >* mapProp = prop->Cast< MetaPropertyHashMap< keyType, Object* > >();\
		for ( usize j = 0; j < mapSize; ++j )\
		{\
			/* Read Key */\
			keyType key = buffer.Read< keyType >();\
			/* Read Value */\
			Object* val = Deserialize( &buffer );\
			/* Set Value */\
			mapProp->SetValueAt( object, key, val );\
		}\
	}
				case MetaPropertyType::HashMap:
				{ 
					// Get base
					const MetaPropertyHashMapBase* base = prop->Cast< MetaPropertyHashMapBase >( );

					// Read size of map to buffer
					usize mapSize = mBuffer.Read< usize >( );

					switch ( base->GetKeyType( ) )
					{
						case MetaPropertyType::U32:
						{
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::U32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, u32, mapSize, mBuffer )	break;
								case MetaPropertyType::S32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, s32, u32, mapSize, mBuffer )	break;
								case MetaPropertyType::F32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, f32, u32, mapSize, mBuffer )	break;
							}
						} break;

						case MetaPropertyType::String:
						{
							switch( base->GetValueType( ) )
							{
								case MetaPropertyType::U32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, String, u32, mapSize, mBuffer )	break; 
								case MetaPropertyType::Object:	READ_MAP_KEY_PRIM_VAL_OBJ( object, base, String, mapSize, mBuffer )			break;
							}

						} break;

						case MetaPropertyType::Enum:
						{ 
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::String:	READ_MAP_KEY_PRIM_VAL_PRIM( object, base, s32, String, mapSize, mBuffer )	break;
								case MetaPropertyType::Object:	READ_MAP_KEY_PRIM_VAL_OBJ( object, base, s32, mapSize, mBuffer )			break;
							}
						} break;
					} 
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
