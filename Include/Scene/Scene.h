// @file Scene.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SCENE_H
#define ENJON_SCENE_H 

#include "System/Types.h"
#include "Asset/Asset.h" 

namespace Enjon
{
	class SceneAssetLoader;

	ENJON_CLASS( )
	class Scene : public Asset
	{
		friend SceneAssetLoader;

		ENJON_CLASS_BODY( Scene ) 

		public:
			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* archiver ) const override;
			
			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* archiver ) override;

		protected:

		private:

	};
}

#endif
