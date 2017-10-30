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
	ENJON_CLASS()
	class AnotherObject : public Enjon::Object
	{
		ENJON_CLASS_BODY( AnotherObject ) 

		public:

			ENJON_PROPERTY( )
			f32 mFloatValue; 

			ENJON_PROPERTY( )
			u32 mUintValue;
			
			ENJON_PROPERTY( )
			AssetHandle<Texture> mTexture;
	}; 

	class EnjonObjectSerializer
	{
		public:
			EnjonObjectSerializer( ) = default;
			~EnjonObjectSerializer( ) = default;

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
							writeBuffer.Write( *cls->GetValueAs< u32 >( object, prop ) );
						} break;

						case MetaPropertyType::F32:
						{ 
							writeBuffer.Write( *cls->GetValueAs< f32 >( object, prop ) );
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

			void Deserialize( ByteBuffer& readBuffer, Object* object )
			{
				MetaClass* cls = const_cast< MetaClass* > ( object->Class( ) );
				PropertyTable& pt = cls->GetProperties( ); 

				// Iterate through properties and get from read buffer
				for ( auto& prop : pt ) 
				{
					switch ( prop->GetType( ) )
					{
						case MetaPropertyType::U32:
						{
							// Set value of object from read buffer
							cls->SetValue( object, prop, readBuffer.Read<u32>( ) );
						} break;
						
						case MetaPropertyType::F32:
						{
							// Set value of object from read buffer
							cls->SetValue( object, prop, readBuffer.Read<f32>( ) );
						} break;
 
						case MetaPropertyType::AssetHandle:
						{
							// Grab asset manager
							AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get<AssetManager>( );
							AssetHandle<Asset> val = nullptr;
							cls->GetValue( object, prop, &val ); 

							// Get meta class of the asset
							MetaClass* assetCls = const_cast< MetaClass* >( val.GetAssetClass( ) );

							// Get uuid from read buffer
							UUID id = readBuffer.Read< UUID >( );

							// Get asset
							Asset* asset = am->GetAsset( assetCls, id ); 

							// If valid asset
							if ( asset )
							{
								// Set asset handle to default asset
								val.Set( asset ); 

								// Set value of object
								cls->SetValue( object, prop, &val );
							} 

						} break;
					}
				}
			}
	};
}

#endif