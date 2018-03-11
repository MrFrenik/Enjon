// @file SceneManager.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SCENEMANAGER_H
#define ENJON_SCENEMANAGER_H 

#include "Subsystem.h"
#include "System/Types.h"
#include "Scene/Scene.h" 

namespace Enjon
{ 
	ENJON_CLASS( Construct )
	class SceneManager : public Subsystem
	{ 
		ENJON_CLASS_BODY( )

		public:

			/**
			* @brief
			*/
			SceneManager( ) = default;

			/**
			*@brief destructor
			*/
			~SceneManager( ) = default;

			/**
			*@brief
			*/
			virtual Result Initialize() override;
			
			/**
			*@brief
			*/
			virtual void Update( const f32 dT ) override;
			
			/**
			*@brief
			*/
			virtual Enjon::Result Shutdown() override;

			/**
			*@brief
			*/
			void LoadScene( const AssetHandle< Scene >& scene );

			/**
			*@brief
			*/
			void LoadScene( const String& sceneName );

			/**
			*@brief
			*/
			void LoadScene( const UUID& uuid );

			/**
			*@brief
			*/
			UUID UnloadScene( );

			/**
			*@brief
			*/
			void ReloadScene( );

			/**
			*@brief
			*/
			AssetHandle< Scene > GetScene( ) const;

		public:

		protected: 

		private: 
			AssetHandle< Scene > mCurrentScene; 
	};
}

#endif
