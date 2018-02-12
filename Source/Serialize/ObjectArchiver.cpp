
// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ObjectArchiver.h

#include "Serialize/ObjectArchiver.h"
#include "Serialize/EntityArchiver.h"
#include "Graphics/Color.h"													// Don't like this here, but I'll leave it for now
#include "Serialize/UUID.h"
#include "System/Types.h"
#include "Defines.h"
#include "Asset/AssetManager.h"
#include "Math/Transform.h"
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
		// Make call to static function and return result
		return Serialize( object, &mBuffer );
	}

	//=====================================================================

	Result ObjectArchiver::Serialize( const Object* object, ByteBuffer* buffer )
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
			buffer->Write< String >( cls->GetName( ) );
			// Make shift version number
			buffer->Write( 0 );		

			// Serialize all object specific data ( classes can override at this point how they want to serialize data )
			Result res = object->SerializeData( buffer );

			// Continue with default serialization if the object doesn't handle its own
			if ( res == Result::INCOMPLETE )
			{
				res = SerializeObjectDataDefault( object, cls, buffer );
			}

			// Final result of serialization
			return res;
		}

		// Shouldn't reach here
		return Result::FAILURE; 
	}

	//=====================================================================
 
	Result ObjectArchiver::SerializeObjectDataDefault( const Object* object, const MetaClass* cls )
	{ 
		// Call to static method and return result
		return ObjectArchiver::SerializeObjectDataDefault( object, cls, &mBuffer ); 
	}

	//=====================================================================

#define WRITE_PROP_SIZE_POD( buffer, valType )\
	buffer->Write< usize >( sizeof( valType ) );

#define WRITE_PROP( buffer, cls, object, prop, valType )\
	buffer->Write< valType >( *cls->GetValueAs< valType >( object, prop ) );

	Result ObjectArchiver::SerializeObjectDataDefault( const Object* object, const MetaClass* cls, ByteBuffer* buffer )
	{ 
		// Write out property count to buffer
		buffer->Write< usize >( cls->GetPropertyCount( ) );

		// Serialize all object properties
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
			buffer->Write< s32 >( ( s32 )prop->GetType( ) );

			switch ( prop->GetType( ) )
			{
				case MetaPropertyType::U8:
				{
					WRITE_PROP_SIZE_POD( buffer, u8 )
					WRITE_PROP( buffer, cls, object, prop, u8 )
				} break;

				case MetaPropertyType::U16:
				{
					WRITE_PROP_SIZE_POD( buffer, u16 )
					WRITE_PROP( buffer, cls, object, prop, u16 )
				} break;

				case MetaPropertyType::U32:
				{
					WRITE_PROP_SIZE_POD( buffer, u32 )
					WRITE_PROP( buffer, cls, object, prop, u32 )
				} break;

				case MetaPropertyType::U64:
				{
					WRITE_PROP_SIZE_POD( buffer, u64 )
					WRITE_PROP( buffer, cls, object, prop, u64 )
				} break;

				case MetaPropertyType::S8:
				{
					WRITE_PROP_SIZE_POD( buffer, s8 )
					WRITE_PROP( buffer, cls, object, prop, s8 )
				} break;

				case MetaPropertyType::S16:
				{
					WRITE_PROP_SIZE_POD( buffer, s16 )
					WRITE_PROP( buffer, cls, object, prop, s16 )
				} break;

				case MetaPropertyType::S32:
				{
					WRITE_PROP_SIZE_POD( buffer, s32 )
					WRITE_PROP( buffer, cls, object, prop, s32 )
				} break;

				case MetaPropertyType::S64:
				{
					WRITE_PROP_SIZE_POD( buffer, s64 )
					WRITE_PROP( buffer, cls, object, prop, s64 )
				} break;

				case MetaPropertyType::F32:
				{
					WRITE_PROP_SIZE_POD( buffer, f32 )
					WRITE_PROP( buffer, cls, object, prop, f32 )
				} break;

				case MetaPropertyType::F64:
				{
					WRITE_PROP_SIZE_POD( buffer, f64 )
					WRITE_PROP( buffer, cls, object, prop, f64 )
				} break;

				case MetaPropertyType::Bool:
				{
					WRITE_PROP_SIZE_POD( buffer, bool )
					WRITE_PROP( buffer, cls, object, prop, bool )
				} break;

				case MetaPropertyType::ColorRGBA32:
				{
					// Write out size of ColorRGBA32
					WRITE_PROP_SIZE_POD( buffer, ColorRGBA32 )
					
					// Get color
					const ColorRGBA32* val = cls->GetValueAs< ColorRGBA32 >( object, prop );

					// Write all individual color channels
					buffer->Write< f32 >( val->r );
					buffer->Write< f32 >( val->g );
					buffer->Write< f32 >( val->b );
					buffer->Write< f32 >( val->a );

				} break;

				case MetaPropertyType::String:
				{
					buffer->Write< usize >( cls->GetValueAs< String >( object, prop )->length( ) );
					WRITE_PROP( buffer, cls, object, prop, String )
				} break;

				case MetaPropertyType::Vec2:
				{
					WRITE_PROP_SIZE_POD( buffer, Vec2 )

					// Get vec2
					const Vec2* val = cls->GetValueAs< Vec2 >( object, prop );

					// Write individual elements of vec2
					buffer->Write< f32 >( val->x );
					buffer->Write< f32 >( val->y );
				} break;

				case MetaPropertyType::Vec3:
				{
					WRITE_PROP_SIZE_POD( buffer, Vec3 )

					// Get vec3
					const Vec3* val = cls->GetValueAs< Vec3 >( object, prop );

					// Write individual elements of vec3
					buffer->Write< f32 >( val->x );
					buffer->Write< f32 >( val->y );
					buffer->Write< f32 >( val->z );
				} break;

				case MetaPropertyType::Vec4:
				{
					WRITE_PROP_SIZE_POD( buffer, Vec4 )

					// Get vec3
					const Vec4* val = cls->GetValueAs< Vec4 >( object, prop );

					// Write individual elements of vec4
					buffer->Write< f32 >( val->x );
					buffer->Write< f32 >( val->y );
					buffer->Write< f32 >( val->z );
					buffer->Write< f32 >( val->w );
				} break; 

				case MetaPropertyType::Transform:
				{
					WRITE_PROP_SIZE_POD( buffer, Transform )

					// Get transform
					const Transform* val = cls->GetValueAs< Transform >( object, prop );

					// Write out position
					buffer->Write< f32 >( val->Position.x );
					buffer->Write< f32 >( val->Position.y );
					buffer->Write< f32 >( val->Position.z );

					// Write out rotation
					buffer->Write< f32 >( val->Rotation.x );
					buffer->Write< f32 >( val->Rotation.y );
					buffer->Write< f32 >( val->Rotation.z );
					buffer->Write< f32 >( val->Rotation.w );

					// Write out scale
					buffer->Write< f32 >( val->Scale.x );
					buffer->Write< f32 >( val->Scale.y );
					buffer->Write< f32 >( val->Scale.z ); 
				} break;

				case MetaPropertyType::UUID:
				{
					// Write size of uuid
					buffer->Write< usize >( UUID::Invalid( ).ToString( ).length( ) ); 
					// Write uuid to buffer
					WRITE_PROP( buffer, cls, object, prop, UUID )
				} break;

				case MetaPropertyType::AssetHandle:
				{
					// Write size of uuid to buffer
					buffer->Write< usize >( UUID::Invalid( ).ToString( ).length( ) );

					// Get value of asset 
					AssetHandle<Asset> val;
					cls->GetValue( object, prop, &val );

					// If valid asset, write its UUID
					if ( val )
					{
						buffer->Write( val.GetUUID( ) );
					}
					// Otherwise write out invalid UUID
					else
					{
						buffer->Write( UUID::Invalid( ) );
					}
				} break;

				case MetaPropertyType::Object:
				{
					// Serialize out object to temporary buffer
					ByteBuffer temp;

					// Treat this differently if is pointer
					if ( prop->GetTraits( ).IsPointer( ) )
					{
						const MetaPropertyPointerBase* base = prop->Cast< MetaPropertyPointerBase >( );
						const Object* obj = base->GetValueAsObject( object );
						ObjectArchiver::Serialize( obj, &temp );
						buffer->Write< usize >( temp.GetSize( ) );

						// Serialize object data to actual buffer
						ObjectArchiver::Serialize( obj, buffer );
					}
					else
					{
						// Not supported yet...
					} 
				} break;

				case MetaPropertyType::Enum:
				{
					// Write size of enum integral value
					WRITE_PROP_SIZE_POD( buffer, s32 )

					// Get integral value of enum
					buffer->Write( *cls->GetValueAs< s32 >( object, prop ) );
				} break;

				case MetaPropertyType::EntityHandle:
				{
					// Create temporary buffer to traverse entity to get size
					ByteBuffer temp;
					EntityHandle handle = *cls->GetValueAs< EntityHandle >( object, prop );
					EntityArchiver::Serialize( handle, &temp );
					buffer->Write< usize >( temp.GetSize( ) );

					// Serialize entity data 
					// TODO(): Just copy in the serialized entity data from the other buffer...
					EntityArchiver::Serialize( handle, buffer ); 

				} break;

	# define WRITE_ARRAY_PROP_PRIM( object, prop, valType, buffer )\
		{\
			const MetaPropertyArray< valType >* arrayProp = prop->Cast< MetaPropertyArray< valType > >();\
			for ( usize j = 0; j < arrayProp->GetSize( object ); ++j )\
			{\
				buffer->Write< valType >( arrayProp->GetValueAs( object, j ) );\
			}\
		} 
				case MetaPropertyType::Array:
				{
					// Get base
					const MetaPropertyArrayBase* base = prop->Cast< MetaPropertyArrayBase >( ); 

					// Get total size to write for array + writing size of array element-wise
					usize totalWriteSize = base->GetSizeInBytes( object ) + sizeof( usize );

					// Write out total write size
					buffer->Write< usize >( totalWriteSize );

					// Write out size of array to buffer
					buffer->Write< usize >( base->GetSize( object ) );

					// Write out array elements
					switch ( base->GetArrayType( ) )
					{
						case MetaPropertyType::Bool:	WRITE_ARRAY_PROP_PRIM( object, base, bool, buffer )		break;
						case MetaPropertyType::U8:		WRITE_ARRAY_PROP_PRIM( object, base, u8, buffer )		break;
						case MetaPropertyType::U32:		WRITE_ARRAY_PROP_PRIM( object, base, u32, buffer )		break;
						case MetaPropertyType::S32:		WRITE_ARRAY_PROP_PRIM( object, base, s32, buffer )		break;
						case MetaPropertyType::F32:		WRITE_ARRAY_PROP_PRIM( object, base, f32, buffer )		break;
						case MetaPropertyType::F64:		WRITE_ARRAY_PROP_PRIM( object, base, f64, buffer )		break;
						case MetaPropertyType::String:	WRITE_ARRAY_PROP_PRIM( object, base, String, buffer )	break;
						case MetaPropertyType::UUID:	WRITE_ARRAY_PROP_PRIM( object, base, UUID, buffer )		break;

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
									buffer->Write< UUID >( asset->GetUUID( ) );
								}
								else
								{
									buffer->Write< UUID >( UUID::Invalid( ) );
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
				buffer->Write< keyType >( iter->first );\
				buffer->Write< valType >( iter->second );\
			}\
		} 

	#define WRITE_MAP_KEY_PRIM_VAL_OBJECT( object, prop, keyType, buffer )\
		{\
			const MetaPropertyHashMap< keyType, Object* >* mapProp = prop->Cast< MetaPropertyHashMap< keyType, Object* > >();\
			for ( auto iter = mapProp->Begin( object ); iter != mapProp->End( object ); ++iter )\
			{\
				/*Write Key*/\
				buffer->Write< keyType >( iter->first );\
				/*Write Value*/\
				Serialize( iter->second, buffer );\
			}\
		}
				case MetaPropertyType::HashMap:
				{
					// Get base
					const MetaPropertyHashMapBase* base = prop->Cast< MetaPropertyHashMapBase >( );

					// Get total write size = numElements * ( size of key type + size of val type ) + size of usize
					usize totalWriteSize = base->GetSizeInBytes( object ) + sizeof( usize ); 

					// Write total write size out
					buffer->Write< usize >( totalWriteSize );

					// Write out size of map to buffer
					buffer->Write< usize >( base->GetSize( object ) );

					switch ( base->GetKeyType( ) )
					{
						case MetaPropertyType::U32:
						{
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::U32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, u32, buffer )	break;
								case MetaPropertyType::S32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, s32, buffer )	break;
								case MetaPropertyType::F32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, f32, buffer )	break;
							}
						} break;

						case MetaPropertyType::String:
						{
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::U32:		WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, String, u32, buffer )	break;
								case MetaPropertyType::Object:	WRITE_MAP_KEY_PRIM_VAL_OBJECT( object, base, String, buffer )		break;
							}
						} break;

						case MetaPropertyType::Enum:
						{
							switch ( base->GetValueType( ) )
							{
								case MetaPropertyType::String:	WRITE_MAP_KEY_PRIM_VAL_PRIM( object, base, s32, String, buffer )	break;
								case MetaPropertyType::Object:	WRITE_MAP_KEY_PRIM_VAL_OBJECT( object, base, s32, buffer )			break;
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

	Result ObjectArchiver::Deserialize( const String& filePath, Object* object )
	{
		// Reset the byte buffer
		Reset( );

		// Read contents into buffer
		mBuffer.ReadFromFile( filePath );

		if ( mBuffer.GetStatus( ) == BufferStatus::ReadyToRead )
		{
			// Return result from static method
			return Deserialize( &mBuffer, object ); 
		}
		else
		{
			return Result::FAILURE;
		}
	}

	//=====================================================================

	Result ObjectArchiver::Deserialize( const String& filePath, HashMap< const MetaClass*, Vector< Object* > >& out )
	{
		return Result::SUCCESS;
	}

	//=====================================================================

	/*
	*@brief Takes an existing byte buffer, parses the buffer and then fills out the object passed in using that buffer
	*/ 
	Result ObjectArchiver::Deserialize( ByteBuffer* buffer, Object* object )
	{
		// Object Header information 
		const MetaClass* cls = Object::GetClass( buffer->Read< String >( ) );	// Read class type
		u32 versionNumber = buffer->Read< u32 >( );								// Read version number id 

		if ( cls )
		{
			// If nullptr, then attempt to construct the object
			if ( object == nullptr )
			{
				object = cls->Construct( ); 

				// Couldn't construct object after attempting
				if ( !object )
				{
					delete object;
					object = nullptr;
					return Result::FAILURE;
				} 
			} 
			// Successfully constructed, now deserialize data into it
			else
			{
				Result res = object->DeserializeData( buffer );

				// Default deserialization method if not object does not handle its own deserialization
				if ( res == Result::INCOMPLETE )
				{
					res = DeserializeObjectDataDefault( object, cls, buffer );
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
				Result res = object->DeserializeData( buffer );

				// Default deserialization method if not object does not handle its own deserialization
				if ( res == Result::INCOMPLETE )
				{
					res = DeserializeObjectDataDefault( object, cls, buffer );
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

	Result ObjectArchiver::DeserializeObjectDataDefault( const Object* object, const MetaClass* cls )
	{
		return DeserializeObjectDataDefault( object, cls, &mBuffer );
	}

	//===========================================================

#define READ_PROP( buffer, cls, object, prop, valType )\
	valType val = buffer->Read< valType >();\
	cls->SetValue(object, prop, val);

	Result ObjectArchiver::DeserializeObjectDataDefault( const Object* object, const MetaClass* cls, ByteBuffer* buffer )
	{ 
		// Read in property count
		usize propCount = buffer->Read< usize >( );

		for ( usize i = 0; i < propCount; ++i )
		{
			// Read in meta property
			const MetaProperty* prop = cls->GetPropertyByName( buffer->Read< String >( ) );
			// Read the type
			MetaPropertyType propType = ( MetaPropertyType )buffer->Read< s32 >( );
			// Read in the total size in bytes written for this property
			usize propSize = buffer->Read< usize >( );

			if ( prop && propType == prop->GetType( ) )
			{
				switch ( prop->GetType( ) )
				{
					case MetaPropertyType::U8:
					{
						READ_PROP( buffer, cls, object, prop, u8 )
					} break;

					case MetaPropertyType::U16:
					{
						READ_PROP( buffer, cls, object, prop, u16 )
					} break;

					case MetaPropertyType::U32:
					{
						// Set value of object from read buffer
						READ_PROP( buffer, cls, object, prop, u32 )
					} break;

					case MetaPropertyType::U64:
					{
						// Set value of object from read buffer
						READ_PROP( buffer, cls, object, prop, u64 )
					} break;

					case MetaPropertyType::F32:
					{
						// Set value of object from read buffer
						READ_PROP( buffer, cls, object, prop, f32 )
					} break;

					case MetaPropertyType::String:
					{
						READ_PROP( buffer, cls, object, prop, String )
					} break;

					case MetaPropertyType::S8:
					{
						READ_PROP( buffer, cls, object, prop, s8 )
					} break;

					case MetaPropertyType::S16:
					{
						READ_PROP( buffer, cls, object, prop, s16 )
					} break;

					case MetaPropertyType::S32:
					{
						READ_PROP( buffer, cls, object, prop, s32 )
					} break;

					case MetaPropertyType::S64:
					{
						READ_PROP( buffer, cls, object, prop, s64 )
					} break;

					case MetaPropertyType::UUID:
					{
						READ_PROP( buffer, cls, object, prop, UUID )
					} break;

					case MetaPropertyType::Bool:
					{
						READ_PROP( buffer, cls, object, prop, bool )
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
						cls->SetValue( object, prop, val );

					} break;

					case MetaPropertyType::Vec2:
					{
						// Read individual elements of Vec2
						f32 x = buffer->Read< f32 >( );
						f32 y = buffer->Read< f32 >( );

						// Set Vec2 property
						cls->SetValue( object, prop, Vec2( x, y ) );
					} break;

					case MetaPropertyType::Vec3:
					{
						// Read individual elements of Vec3
						f32 x = buffer->Read< f32 >( );
						f32 y = buffer->Read< f32 >( );
						f32 z = buffer->Read< f32 >( );

						// Set Vec3 property
						cls->SetValue( object, prop, Vec3( x, y, z ) );
					} break;

					case MetaPropertyType::Vec4:
					{
						// Read individual elements of Vec4
						f32 x = buffer->Read< f32 >( );
						f32 y = buffer->Read< f32 >( );
						f32 z = buffer->Read< f32 >( );
						f32 w = buffer->Read< f32 >( );

						// Set Vec4 property
						cls->SetValue( object, prop, Vec4( x, y, z, w ) );
					} break;

					case MetaPropertyType::Transform:
					{ 
						Transform val;
		 
						// Read in position
						val.Position.x = buffer->Read< f32 >( );
						val.Position.y = buffer->Read< f32 >( );
						val.Position.z = buffer->Read< f32 >( );

						// Read in rotation
						val.Rotation.x = buffer->Read< f32 >( );
						val.Rotation.y = buffer->Read< f32 >( );
						val.Rotation.z = buffer->Read< f32 >( );
						val.Rotation.w = buffer->Read< f32 >( );

						// Read in scale
						val.Scale.x = buffer->Read< f32 >( );
						val.Scale.y = buffer->Read< f32 >( );
						val.Scale.z = buffer->Read< f32 >( );

						// Set transform property
						cls->SetValue( object, prop, val ); 
					} break;

					case MetaPropertyType::ColorRGBA32:
					{
						// Read all individual color channels
						f32 r = buffer->Read< f32 >( );
						f32 g = buffer->Read< f32 >( );
						f32 b = buffer->Read< f32 >( );
						f32 a = buffer->Read< f32 >( );

						// Set ColorRGBA32 property
						cls->SetValue( object, prop, ColorRGBA32( r, g, b, a ) );
					} break;

					case MetaPropertyType::Object:
					{
						// If is pointer
						if ( prop->GetTraits( ).IsPointer( ) )
						{
							const MetaPropertyPointerBase* base = prop->Cast< MetaPropertyPointerBase >( );
							Object* actualObj = base->GetValueAsObject( object )->ConstCast<Object>( );
 
							if ( actualObj )
							{
								// Grab object from deserializer
								ObjectArchiver::Deserialize( buffer, actualObj ); 

								// Set value
								cls->SetValue( object, prop, actualObj ); 
							}
							else
							{
								// Grab object from deserializer
								Object* obj = ObjectArchiver::Deserialize( buffer );

								// Set value
								cls->SetValue( object, prop, obj ); 
							} 
						}
						else
						{
							// Not handled yet...
						}
					} break;

					case MetaPropertyType::Enum:
					{
						// Read value from buffer
						s32 val = buffer->Read< s32 >( );

						// Set property on object
						cls->SetValue( object, prop, val );

					} break;

					case MetaPropertyType::EntityHandle:
					{
						// Get the entity handle
						EntityHandle handle = EntityArchiver::Deserialize( buffer );

						// Set value on object
						cls->SetValue( object, prop, handle );
					} break;

	# define READ_ARRAY_PROP_PRIM( object, prop, valType, arraySize, buffer )\
		{\
			const MetaPropertyArray< valType >* arrayProp = prop->Cast< MetaPropertyArray< valType > >();\
			for ( usize j = 0; j < arraySize; ++j )\
			{\
				/*Grab value from buffer and set at index in array*/\
				arrayProp->SetValueAt( object, j, buffer->Read< valType >( ) );\
			}\
		} 
				case MetaPropertyType::Array:
				{
					// Get base
					const MetaPropertyArrayBase* base = prop->Cast< MetaPropertyArrayBase >( );

					// Read size of array from buffer
					usize arraySize = buffer->Read< usize >( );

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
							case MetaPropertyType::Bool:	READ_ARRAY_PROP_PRIM( object, base, bool, arraySize, buffer )	break;
							case MetaPropertyType::U8:		READ_ARRAY_PROP_PRIM( object, base, u8, arraySize, buffer )		break;
							case MetaPropertyType::U32:		READ_ARRAY_PROP_PRIM( object, base, u32, arraySize, buffer )	break;
							case MetaPropertyType::S32:		READ_ARRAY_PROP_PRIM( object, base, s32, arraySize, buffer )	break;
							case MetaPropertyType::F32:		READ_ARRAY_PROP_PRIM( object, base, f32, arraySize, buffer )	break;
							case MetaPropertyType::F64:		READ_ARRAY_PROP_PRIM( object, base, f64, arraySize, buffer )	break;
							case MetaPropertyType::String:	READ_ARRAY_PROP_PRIM( object, base, String, arraySize, buffer )	break;
							case MetaPropertyType::UUID:	READ_ARRAY_PROP_PRIM( object, base, UUID, arraySize, buffer )	break;
							case MetaPropertyType::AssetHandle:
							{
								MetaArrayPropertyProxy proxy = base->GetProxy( );
								const MetaPropertyTemplateBase* arrBase = static_cast< const MetaPropertyTemplateBase* > ( proxy.mArrayPropertyTypeBase );
								const MetaClass* assetCls = const_cast< Enjon::MetaClass* >( arrBase->GetClassOfTemplatedArgument( ) );

								// Write out asset uuids in array
								const MetaPropertyArray< AssetHandle< Asset > >* arrProp = base->Cast< MetaPropertyArray< AssetHandle< Asset > > >( );
								for ( usize j = 0; j < arraySize; ++j )
								{
									AssetHandle<Asset> newAsset = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->GetAsset( assetCls, buffer->Read< UUID >( ) );
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
					keyType key = buffer->Read< keyType >( );\
					/*Read Value*/\
					valType val = buffer->Read< valType >( );\
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
					keyType key = buffer->Read< keyType >();\
					/* Read Value */\
					Object* val = Deserialize( buffer );\
					/* Set Value */\
					mapProp->SetValueAt( object, key, val );\
				}\
			}
					case MetaPropertyType::HashMap:
					{
						// Get base
						const MetaPropertyHashMapBase* base = prop->Cast< MetaPropertyHashMapBase >( );

						// Read size of map to buffer
						usize mapSize = buffer->Read< usize >( );

						switch ( base->GetKeyType( ) )
						{
							case MetaPropertyType::U32:
							{
								switch ( base->GetValueType( ) )
								{
									case MetaPropertyType::U32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, u32, u32, mapSize, buffer )	break;
									case MetaPropertyType::S32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, s32, u32, mapSize, buffer )	break;
									case MetaPropertyType::F32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, f32, u32, mapSize, buffer )	break;
								}
							} break;

							case MetaPropertyType::String:
							{
								switch ( base->GetValueType( ) )
								{
									case MetaPropertyType::U32:		READ_MAP_KEY_PRIM_VAL_PRIM( object, base, String, u32, mapSize, buffer )	break;
									case MetaPropertyType::Object:	READ_MAP_KEY_PRIM_VAL_OBJ( object, base, String, mapSize, buffer )			break;
								}

							} break;

							case MetaPropertyType::Enum:
							{
								switch ( base->GetValueType( ) )
								{
									case MetaPropertyType::String:	READ_MAP_KEY_PRIM_VAL_PRIM( object, base, s32, String, mapSize, buffer )	break;
									case MetaPropertyType::Object:	READ_MAP_KEY_PRIM_VAL_OBJ( object, base, s32, mapSize, buffer )			break;
								}
							} break;
						}
					} break;
				}
			}
			// Otherwise skip the property in the buffer
			else
			{
				buffer->AdvanceReadPosition( propSize );
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
