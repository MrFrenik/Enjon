// Copyright 2016-2018 John Jackson. All Rights Reserved.
// File: BaseTypeSerializeMethods.cpp

#include "Base/Object.h"
#include "Serialize/BaseTypeSerializeMethods.h"
#include "Graphics/Color.h"
#include "Serialize/UUID.h"
#include "Asset/Asset.h"
#include "Asset/AssetManager.h"
#include "Math/Transform.h"
#include "Serialize/AssetArchiver.h"
#include "Serialize/ObjectArchiver.h"
#include "Serialize/ByteBuffer.h"
#include "Serialize/EntityArchiver.h"
#include "System/Types.h"
#include "Defines.h"
#include "SubsystemCatalog.h"

namespace Enjon
{ 
	//==================================================================================================================

#define WRITE_PROP_SIZE_POD( buffer, valType )\
	buffer->Write< usize >( sizeof( valType ) );

#define WRITE_PROP( buffer, cls, object, prop, valType )\
	buffer->Write< valType >( *cls->GetValueAs< valType >( object, prop ) );

	void PropertyArchiver::Serialize( const Object* object, const MetaProperty* prop, ByteBuffer* buffer )
	{ 
		// Get object class 
		const MetaClass* cls = object->Class( );

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

				case MetaPropertyType::iVec3:
				{
					WRITE_PROP_SIZE_POD( buffer, iVec3 );

					// Get ivec3
					const iVec3* val = cls->GetValueAs< iVec3 >( object, prop );

					// Write out individual elements of ivec2
					buffer->Write< s32 >( val->x );
					buffer->Write< s32 >( val->y );
					buffer->Write< s32 >( val->z ); 
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
					buffer->Write< f32 >( val->GetPosition().x );
					buffer->Write< f32 >( val->GetPosition().y );
					buffer->Write< f32 >( val->GetPosition().z );

					// Write out rotation
					buffer->Write< f32 >( val->GetRotation().x );
					buffer->Write< f32 >( val->GetRotation().y );
					buffer->Write< f32 >( val->GetRotation().z );
					buffer->Write< f32 >( val->GetRotation().w );

					// Write out scale
					buffer->Write< f32 >( val->GetScale().x );
					buffer->Write< f32 >( val->GetScale().y );
					buffer->Write< f32 >( val->GetScale().z ); 
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
						// Write out to temp to write size of object
						const Object* obj = cls->GetValueAs< Object >( object, prop );
						ObjectArchiver::Serialize( obj, &temp );
						buffer->Write< usize >( temp.GetSize( ) );

						// Serialize object data
						ObjectArchiver::Serialize( obj, buffer );
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

						case MetaPropertyType::Object:
						{ 
							const MetaPropertyArray< Object* >* arrProp = base->Cast < MetaPropertyArray< Object * > >( );
							if ( arrProp )
							{
								for ( usize j = 0; j < arrProp->GetSize( object ); ++j )
								{
									Object* obj = arrProp->GetValueAs( object, j );
									if ( obj )
									{
										ObjectArchiver::Serialize( obj, buffer ); 
									}
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
				ObjectArchiver::Serialize( iter->second, buffer );\
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
								case MetaPropertyType::Object:
								{
									const MetaPropertyHashMap< String, Object* >* mapProp = prop->Cast< MetaPropertyHashMap< String, Object* > >();
									for ( auto iter = mapProp->Begin( object ); iter != mapProp->End( object ); ++iter )\
									{
										/*Write Key*/
										buffer->Write< String >( iter->first );
										/*Write Value*/
										ObjectArchiver::Serialize( iter->second, buffer );
									}

								} break;
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

	//==================================================================================================================

#define READ_PROP( buffer, cls, object, prop, valType )\
	valType val = buffer->Read< valType >();\
	cls->SetValue(object, prop, val);

	void PropertyArchiver::Deserialize( const Object* object, ByteBuffer* buffer )
	{
			// Get class from object
			const MetaClass* cls = object->Class( );
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

					case MetaPropertyType::iVec3:
					{
						// Read individual elements of Vec2
						s32 x = buffer->Read< s32 >( );
						s32 y = buffer->Read< s32 >( );
						s32 z = buffer->Read< s32 >( );

						// Set iVec3 property
						cls->SetValue( object, prop, iVec3( x, y, z ) );

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
						Vec3 position;
						position.x = buffer->Read< f32 >( );
						position.y = buffer->Read< f32 >( );
						position.z = buffer->Read< f32 >( );
						val.SetPosition( position );

						// Read in rotation
						Quaternion rotation;
						rotation.x = buffer->Read< f32 >( );
						rotation.y = buffer->Read< f32 >( );
						rotation.z = buffer->Read< f32 >( );
						rotation.w = buffer->Read< f32 >( );
						val.SetRotation( rotation );

						// Read in scale
						Vec3 scale;
						scale.x = buffer->Read< f32 >( );
						scale.y = buffer->Read< f32 >( );
						scale.z = buffer->Read< f32 >( );

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
 
							// Destroy the object for now and recreate it
							if ( actualObj )
							{
								delete actualObj;
								actualObj = nullptr;
							}

							// Grab object from deserializer
							Object* obj = ObjectArchiver::Deserialize( buffer );

							// Set value
							cls->SetValue( object, prop, obj ); 
						}
						else
						{
							// Grab the object pointer
							Object* obj = cls->GetValueAs< Object >( object, prop )->ConstCast< Object >( );
							// Deserialize data
							ObjectArchiver::Deserialize( buffer, obj );
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
							
							case MetaPropertyType::Object:
							{
								const MetaPropertyArray< Object* >* arrProp = prop->Cast< MetaPropertyArray< Object* > >( );
								if ( arrProp )
								{
									for ( usize j = 0; j < arraySize; ++j )
									{
										arrProp->SetValueAt( object, j, ObjectArchiver::Deserialize( buffer ) );
									}
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
					Object* val = ObjectArchiver::Deserialize( buffer );\
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
}
