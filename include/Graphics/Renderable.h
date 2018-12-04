// @file Renderable.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_RENDERABLE_H
#define ENJON_RENDERABLE_H

#include "Defines.h"
#include "Math/Transform.h"
#include "Math/Quaternion.h"
#include "System/Types.h"
#include "Asset/Asset.h"
#include "Base/Object.h"
#include "Entity/EntityDefines.h"
#include "Graphics/Material.h"

namespace Enjon 
{ 
	class SubMesh;
	class Mesh;
	class GraphicsScene;
	class GraphicsSubsystem;
	class GraphicsSubsystemContext;
	class GLSLProgram;
	class ColorRGBA32;

	ENJON_CLASS( Abstract )
	class Renderable : public Enjon::Object
	{ 
		friend GraphicsScene;
		friend GraphicsSubsystem;

		ENJON_CLASS_BODY( Renderable )

		public:

			/*
			* @brief
			*/
			virtual Result OnEditorUI( ) override;

			/*
			* @brief
			*/
			virtual void ExplicitDestructor( ) override;

			/* Get position of transform */
			Vec3 GetPosition() const;

			/* Get scale of transform */
			Vec3 GetScale() const;

			/* Get rotation of transform */
			Quaternion GetRotation() const;

			/* Offset rotation of transform */
			void Renderable::OffsetRotation(const f32& Yaw, const f32& Pitch);

			/* Get material of renderable */
			AssetHandle< Material > GetMaterial( const u32& idx = 0 ) const; 

			/* Get scene of renderable */
			GraphicsScene* GetGraphicsScene() const;

			/* Gets world transform */
			Transform GetTransform() const;

			/* Sets world transform */
			void SetTransform(const Transform& transform);

			/* Set position of transform */
			void SetPosition(const Vec3& position);

			/* Set scale of transform */
			void SetScale(const Vec3& scale);

			/* Set uniform scale of transform */
			void SetScale(const f32& scale);

			/* Set rotation of transform */
			void SetRotation(const Quaternion& rotation); 

			/* Set material of renderable */
			void SetMaterial( const AssetHandle< Material >& material, const u32& idx = 0 ); 

			/* Set scene of renderable */
			void SetGraphicsScene(GraphicsScene* scene); 

			/* Set mesh of renderable */
			virtual void SetMesh( const Mesh* mesh ) = 0;

			/* Get mesh of renderable */
			virtual const Mesh* GetMesh() const = 0;

			/*
			* @brief
			*/
			u32 GetMaterialsCount( ) const;

			/*
			* @brief
			*/
			const Vector<AssetHandle<Material>>& GetMaterials( ) const;
 
			/** 
			* @brief
			*/
			void SetRenderableID( const u32& id );

			/** 
			* @brief
			*/
			u32 GetRenderableID( ) const; 

			/** 
			* @brief
			*/
			const Mat4x4 GetModelMatrix( ) const;

			/** 
			* @brief
			*/
			const Mat4x4 GetPreviousModelMatrix( ) const;

		public:

			/** 
			* @brief
			*/
			static ColorRGBA32 IdToColor( const u32& id, const u32& subMeshIdx );

			/** 
			* @brief
			*/
			static u32 ColorToID( const ColorRGBA32& color ); 

			/** 
			* @brief
			*/
			static u32 ColorToSubMeshIdx( const ColorRGBA32& color );

		protected:

			/*
			* @brief
			*/
			void Bind( );

			/*
			* @brief
			*/
			void Unbind( );

			/*
			* @brief
			*/
			const void Submit( const Enjon::Shader* shader );

			/*
			* @brief
			*/
			const void Submit( const Enjon::Shader* shader, const SubMesh* subMesh, const u32& subMeshIdx );

			/*
			* @brief
			*/
			const void Submit( const GLSLProgram* shader, const SubMesh* subMesh, const u32& subMeshIdx );
	
			/*
			* @brief
			*/
			const void Submit( const Enjon::GLSLProgram* shader );

		protected: 
			
			ENJON_PROPERTY( )
			Vector<AssetHandle<Material>> mMaterialElements; 

			u32 mRenderableID = MAX_ENTITIES; 
			Transform mTransform; 
			GraphicsScene* mGraphicsScene = nullptr;
			Mat4x4 mPreviousModelMatrix = Mat4x4::Identity( );
			Mat4x4 mCurrentModelMatrix = Mat4x4::Identity( );
	};
}


#endif