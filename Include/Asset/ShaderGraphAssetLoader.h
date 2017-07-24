// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ShaderGraphAssetLoader.h

#pragma  once
#ifndef ENJON_SHADER_GRAPH_ASSET_LOADER_H
#define ENJON_SHADER_GRAPH_ASSET_LOADER_H 

#include "Asset/AssetLoader.h"
#include "Graphics/ShaderGraph.h"

namespace Enjon
{
	class ShaderGraphAssetLoader : public AssetLoader
	{
	public:

		/**
		* @brief Constructor
		*/
		ShaderGraphAssetLoader( );

		/**
		* @brief Destructor
		*/
		~ShaderGraphAssetLoader( );


	protected:

		/**
		* @brief
		*/
		virtual void RegisterDefaultAsset( ) override;


	private:
		/**
		* @brief
		*/
		ShaderGraph* LoadResourceFromFile( const String& filePath, const String& name );

		/**
		* @brief
		*/
		ShaderGraph* LoadShaderGraphFromFile( const Enjon::String& filePath );
	};
}

#endif
