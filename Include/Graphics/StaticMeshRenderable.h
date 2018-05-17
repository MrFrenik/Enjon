// @file StaticMeshRenderable.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_STATIC_MESH_RENDERABLE_H
#define ENJON_STATIC_MESH_RENDERABLE_H

#include "Graphics/Renderable.h"
#include "Graphics/Mesh.h"

namespace Enjon
{
	ENJON_CLASS( )
	class StaticMeshRenderable : public Renderable
	{
		ENJON_CLASS_BODY( StaticMeshRenderable )

		public: 

			/**
			* @brief
			*/
			virtual void SetMesh( const Mesh* mesh ) override;

			/**
			* @brief
			*/
			virtual const Mesh* GetMesh() const override; 

			/**
			* @brief
			*/
			void SetMesh( const AssetHandle< Mesh >& mesh ); 

		protected:

			ENJON_PROPERTY( Editable, Delegates[ Mutator = SetMesh ] )
			AssetHandle< Mesh > mMesh; 
	};
}


#endif
