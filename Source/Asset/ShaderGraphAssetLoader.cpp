// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ShaderGraphAssetLoader.cpp 

#include "Asset/ShaderGraphAssetLoader.h"

namespace Enjon
{
	//=================================================================================

	ShaderGraphAssetLoader::ShaderGraphAssetLoader( )
	{
	}

	//================================================================================= 

	ShaderGraphAssetLoader::~ShaderGraphAssetLoader( )
	{
	} 

	//================================================================================= 

	void ShaderGraphAssetLoader::RegisterDefaultAsset( ) 
	{ 
		// Create new graph and compile
		Enjon::ShaderGraph* graph = new Enjon::ShaderGraph( ); 
		graph->mName = "DefaultShaderGraph";
		graph->Compile( );

		// Set default
		mDefaultAsset = graph;
	}

	//================================================================================= 
 
	Asset* ShaderGraphAssetLoader::LoadResourceFromFile( const String& filePath )
	{
		ShaderGraph* graph = LoadShaderGraphFromFile( filePath );

		// If valid, then register graph and return
		if ( graph )
		{ 
			return graph; 
		}

		// Otherwise return default
		return nullptr;
	}

	//================================================================================= 

	ShaderGraph* ShaderGraphAssetLoader::LoadShaderGraphFromFile( const Enjon::String& filePath )
	{
		ShaderGraph* graph = new Enjon::ShaderGraph( ); 

		// Create from filepath
		s32 status = graph->Create( filePath ); 

		if ( status != 1 )
		{
			// There was an error
			delete graph;
			return nullptr;
		} 

		return graph;
	}

	//================================================================================= 
} 
