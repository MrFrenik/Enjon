// @file SceneManager.cpp
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#include "Scene/SceneManager.h"
#include "Entity/EntityManager.h"
#include "Asset/AssetManager.h"
#include "Engine.h"
#include "SubsystemCatalog.h"

namespace Enjon
{
	//==================================================================

	Result SceneManager::Initialize( )
	{
		return Result::SUCCESS;
	}

	//==================================================================

	void SceneManager::Update( const f32 dT )
	{ 
		// Do nothing...
	}

	//==================================================================
	
	Enjon::Result SceneManager::Shutdown( )
	{
		return Result::SUCCESS;
	}

	//==================================================================

	void SceneManager::LoadScene( const AssetHandle< Scene >& scene ) 
	{ 
		// Set if current scene not the one trying to be loaded
		if ( scene.Get() != mCurrentScene.Get() )
		{
			UnloadScene( );

			mCurrentScene = scene;
		}
	}

	//==================================================================

	void SceneManager::LoadScene( const String& sceneName )
	{ 
		// Unload the previous scene
		UnloadScene( );

		// Get scene from asset manager
		AssetHandle< Scene > scene = EngineSubsystem( AssetManager )->GetAsset< Scene >( sceneName ); 

		// Set current scene
		mCurrentScene = scene;
	}

	//==================================================================

	void SceneManager::LoadScene( const UUID& uuid )
	{ 
		// Unload the previous scene
		UnloadScene( );

		// Get scene from asset manager
		AssetHandle< Scene > scene = EngineSubsystem( AssetManager )->GetAsset< Scene >( uuid ); 

		// Set current scene
		mCurrentScene = scene;
	}

	//==================================================================

	UUID SceneManager::UnloadScene( )
	{ 
		EntityManager* em = EngineSubsystem( EntityManager );

		// Destroy all entities
		em->DestroyAll( );
		// Force cleanup all cleared entities
		em->ForceCleanup( );

		UUID returnUUID;

		// Relase current scene if set
		if ( mCurrentScene )
		{
			// Cache the uuid to return
			returnUUID = mCurrentScene->GetUUID( );
			// Unload the asset
			mCurrentScene.Unload( ); 
			// Set to null
			mCurrentScene = nullptr;
		}

		return returnUUID;
	}

	//==================================================================

	void SceneManager::ReloadScene( )
	{
		if ( mCurrentScene )
		{
			mCurrentScene.Reload( );
		}
	}

	//==================================================================

	AssetHandle< Scene > SceneManager::GetScene( ) const
	{
		return mCurrentScene;
	}

	//==================================================================
}
