// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: Asset.h

#pragma once
#ifndef ENJON_ASSET_H
#define ENJON_ASSET_H 

#include "Defines.h"
#include "System/Types.h"
#include "Base/Object.h"
#include "Serialize/UUID.h"
#include "Serialize/ByteBuffer.h"
#include "Asset/AssetUtils.h"

#include <assert.h>
#include <memory>

namespace Enjon
{ 
	class AssetLoader;
	class AssetManager; 
	class AssetArchiver;
	class AssetRecordInfo;

	template <typename T>
	class AssetHandle;


	ENJON_CLASS( Abstract )
	class Asset : public Enjon::Object
	{ 
		ENJON_CLASS_BODY( Asset )

		public: 
			friend AssetLoader;
			friend AssetManager;
			friend AssetArchiver; 

			/**
			*@brief
			*/
			ENJON_FUNCTION( )
			Enjon::String GetName( ) const 
			{ 
				return mName; 
			} 
			
			/**
			*@brief
			*/
			ENJON_FUNCTION( )
			Enjon::String GetFilePath( ) const 
			{ 
				return mFilePath; 
			}

			/**
			*@brief
			*/
			ENJON_FUNCTION( )
			UUID GetUUID( ) const 
			{ 
				return mUUID; 
			} 

			/**
			*@brief
			*/
			bool IsDefault( ) const;

			/**
			*@brief
			*/
			const AssetLoader* GetLoader( ) const 
			{ 
				return mLoader; 
			} 

			/*
			* @brief
			*/
			const AssetRecordInfo* GetAssetRecordInfo( ) const; 

		public:
			/*
			* @brief
			*/
			virtual Result Reload( )
			{ 
				return Result::SUCCESS;
			} 

			/**
			*@brief
			*/
			Result Unload( );

			/**
			*@brief
			*/
			Result Save( ) const;

			/**
			*@brief
			*/
			virtual Result CopyFromOther( const Asset* other )
			{
				return Result::INCOMPLETE;
			} 

		protected: 

			ENJON_PROPERTY( HideInEditor )
			UUID mUUID;

			ENJON_PROPERTY( HideInEditor )
			Enjon::String mFilePath;

			ENJON_PROPERTY( HideInEditor )
			Enjon::String mName; 

			ENJON_PROPERTY( HideInEditor )
			u32 mIsDefault = false;

			const AssetLoader* mLoader = nullptr;

			const AssetRecordInfo* mRecordInfo = nullptr;

		private:
	};

	template <typename T> 
	class AssetHandle
	{
		public:
			/*
			* @brief Constructor
			*/
			AssetHandle() 
			{
				static_assert(std::is_base_of<Asset, T>::value, "AssetHandle:: T must inherit from Asset.");	
			}

			/*
			* @brief Constructor
			*/
			AssetHandle(const Asset* asset)
			{
				static_assert(std::is_base_of<Asset, T>::value, "AssetHandle:: T must inherit from Asset.");	

				mAsset = asset;
			} 

			/*
			* @brief Destructor
			*/
			~AssetHandle( )
			{ 
			}
			
			/*
			* @brief
			*/
			explicit operator bool( ) 
			{
				return IsValid( );
			} 

			/*
			* @brief
			*/
			bool operator==( const AssetHandle< T >& other )
			{
				return ( Get( ) == other.Get( ) );
			}

			/*
			* @brief
			*/
			bool operator!=( const AssetHandle< T >& other )
			{
				return !( Get( ) == other.Get( ) );
			}

			/*
			* @brief
			*/
			UUID GetUUID( ) const
			{
				// Return UUID if valid
				if ( IsValid( ) )
				{
					return mAsset->GetUUID();
				}

				return UUID::Invalid( );
			}

			/*
			* @brief Implementation in AssetLoader.h
			*/
			void Unload( );

			/*
			* @brief Implementation in AssetLoader.h
			*/
			void Reload( );

			/*
			* @brief
			*/
			const T* Get() const 
			{ 
				return mAsset->Cast<T>(); 
			} 

			/*
			* @brief
			*/
			const MetaClass* GetAssetClass( ) 
			{ 
				return Object::GetClass<T>( );
			}
			
			/*
			* @brief
			*/
			T* operator->( )
			{
				return const_cast< T* >( Get( ) );
			} 

			/*
			* @brief 
			*/
			bool IsValid() const
			{
				return (mAsset != nullptr);
			} 
			
			/*
			* @brief 
			*/
			Result Set( const Asset* asset ) 
			{
				// Set to new asset
				mAsset = asset;

				// Return success
				return Result::SUCCESS;
			} 

			/*
			* @brief
			*/
			Result Save( )
			{
				if ( mAsset && !mAsset->IsDefault() )
				{
					return mAsset->Save( );
				} 
				return Result::FAILURE;
			}

			/*
			* @brief Gets loader based on asset loader type
			*/
			const AssetLoader* GetLoader( ) const;

		protected:

		private: 
			const Asset* mAsset = nullptr;
	};
} 

#endif
