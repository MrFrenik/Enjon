// @file MetaClassRegistry.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.
#pragma once
#ifndef ENJON_META_CLASS_REGISTRY_H
#define ENJON_META_CLASS_REGISTRY_H 

#include "System/Types.h"
#include "Defines.h" 
#include "Base/MetaClass.h"
#include "Graphics/Texture.h"
#include "Serialize/ByteBuffer.h"
#include "Asset/AssetManager.h"
#include "Engine.h"

#include <unordered_map> 

namespace Enjon
{
	namespace TestNamespace
	{
		namespace AnotherSpace
		{
			ENJON_CLASS( Namespace = [ TestNamespace, AnotherSpace ] )
			class AnotherObject : public Enjon::Object
			{
				ENJON_CLASS_BODY( AnotherObject )

			public:

				ENJON_PROPERTY( Editable )
				f32 mFloatValue;

				ENJON_PROPERTY( Editable )
				u32 mUintValue; 

				ENJON_PROPERTY( )
				s32 mIntValue;

				ENJON_PROPERTY( Editable )
				Enjon::AssetHandle<Enjon::Texture> mTexture;

				ENJON_PROPERTY( )
				String mName;
			}; 
		}
	} 

	class EnjonObjectSerializer
	{
		public:
			EnjonObjectSerializer( ) = default;
			~EnjonObjectSerializer( ) = default;

#define WRITE_VAL( valType )\
	writeBuffer.Write< valType >( *cls->GetValueAs< valType >( object, prop) );

			void Serialize( ByteBuffer& writeBuffer, Object* object )
			{
				MetaClass* cls = const_cast< MetaClass* >( object->Class( ) );
				PropertyTable& pt = cls->GetProperties( );

				// Iterate through properties and write to buffer
				for ( auto& prop : pt ) 
				{ 
					switch ( prop->GetType( ) )
					{
						case MetaPropertyType::U32:
						{ 
							WRITE_VAL( u32 )
						} break;

						case MetaPropertyType::F32:
						{ 
							WRITE_VAL( f32 )
						} break; 

						case MetaPropertyType::S32:
						{
							WRITE_VAL( s32 )
						} break;

						case MetaPropertyType::String:
						{
							WRITE_VAL( String )
						} break;

						case MetaPropertyType::AssetHandle:
						{
							// Get value of asset 
							AssetHandle<Asset> val; 
							cls->GetValue( object, prop, &val );
							if ( val )
							{
								writeBuffer.Write( val.GetUUID().ToString() );
							} 

						} break; 
					}
				}
			}

# define READ_SET_VAL(valType)\
	valType val = readBuffer.Read< valType >();\
	cls->SetValue(object, prop, val);

			void Deserialize( ByteBuffer& readBuffer, Object* object )
			{
				MetaClass* cls = const_cast< MetaClass* > ( object->Class( ) );
				PropertyTable& pt = cls->GetProperties( ); 

				// Iterate through properties and get from read buffer
				for ( usize i = 0; i < cls->GetPropertyCount( ); ++i )
				{
					// Grab property at index from metaclass
					MetaProperty* prop = const_cast<MetaProperty*> ( cls->GetProperty( i ) );

					switch ( prop->GetType( ) )
					{
						case MetaPropertyType::U32:
						{
							// Set value of object from read buffer
							READ_SET_VAL( u32 )
						} break;
						
						case MetaPropertyType::F32:
						{
							// Set value of object from read buffer
							READ_SET_VAL( f32 )
						} break;

						case MetaPropertyType::String:
						{
							READ_SET_VAL( String )
						} break;

						case MetaPropertyType::S32:
						{
							READ_SET_VAL( s32 )
						} break;
 
						case MetaPropertyType::AssetHandle:
						{
							// Grab asset manager
							MetaPropertyTemplateBase* base = static_cast< MetaPropertyTemplateBase* >( prop );
							AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
							AssetHandle<Asset> val;
							cls->GetValue( object, prop, &val ); 

							// Get meta class of the asset
							MetaClass* assetCls = const_cast< MetaClass* >( base->GetClassOfTemplatedArgument( ) );

							// Get uuid from read buffer
							UUID id = readBuffer.Read< UUID >( );

							// Get asset
							Asset* asset = am->GetAsset( assetCls, id ); 

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
					}
				}
			}
	};
}

#endif