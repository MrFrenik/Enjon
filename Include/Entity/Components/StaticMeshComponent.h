#pragma once
#ifndef ENJON_STATIC_MESH_COMPONENT_H
#define ENJON_STATIC_MESH_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/Renderable.h"
#include "Graphics/Material.h"
#include "Graphics/StaticMeshRenderable.h"
#include "System/Types.h"
#include "Graphics/GraphicsScene.h"

namespace Enjon
{
	ENJON_CLASS( )
	class StaticMeshComponent : public Component
	{
		ENJON_COMPONENT( StaticMeshComponent  )

		public:

			/* 
			* @brief
			*/
			virtual void ExplicitConstructor() override; 

			/*
			* @brief
			*/
			virtual void ExplicitDestructor() override;

			/*
			* @brief
			*/
			virtual void PostConstruction( ) override; 

			/*
			* @brief
			*/
			virtual void Update( ) override; 

			/*
			* @brief
			*/
			virtual Result OnEditorUI( ) override;

			/*
			* @brief
			*/
			virtual void AddToWorld( World* world ) override;

			/*
			* @brief
			*/
			virtual void RemoveFromWorld( ) override;
 
			/* 
			* @brief Get position of transform 
			*/
			Vec3 GetPosition() const;

			/* 
			* @brief Get scale of transform 
			*/
			Vec3 GetScale() const;

			/* 
			* @brief Get orientation of transform 
			*/
			Quaternion GetRotation() const;

			/* 
			* @brief Get material of renderable 
			*/
			AssetHandle< Material > GetMaterial( const u32& idx = 0 ) const;

			/* 
			* @brief Get mesh of renderable 
			*/
			AssetHandle< Mesh > GetMesh() const;

			/* 
			* @brief Get scene of renderable 
			*/
			GraphicsScene* GetGraphicsScene() const;

			/* 
			* @brief Get world transform 
			*/
			Transform GetTransform() const;
			
			/* 
			* @brief Get renderable 
			*/
			StaticMeshRenderable* GetRenderable();

			/* 
			* @brief Sets world transform 
			*/
			void SetTransform(const Transform& transform); 

			/* 
			* @brief Set position of transform 
			*/
			void SetPosition(const Vec3& position);

			/* 
			* @brief Set scale of transform 
			*/
			void SetScale(const Vec3& scale);

			/* 
			* @brief Set scale of transform 
			*/
			void SetScale(const f32& scale);

			/* 
			* @brief Set orientation of transform 
			*/
			void SetRotation(const Quaternion& rotation); 

			/* 
			* @brief Set material of renderable 
			*/
			void SetMaterial( const AssetHandle< Material >& material, const u32& idx = 0 );

			/* 
			* @brief Set mesh of renderable 
			*/
			void SetMesh(const AssetHandle<Mesh>& mesh);

			/* 
			* @brief Set scene of renderable 
			*/
			void SetGraphicsScene(GraphicsScene* scene); 

			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

		protected: 

		public: 
			
			ENJON_PROPERTY( )
			StaticMeshRenderable mRenderable;

			ENJON_PROPERTY( NonSerializeable, HideInEditor )
			u32 mRenderableHandle = INVALID_RESOURCE_HANDLE;
	};

	ENJON_CLASS( )
	class StaticMeshComponentSystem : public IComponentSystem
	{
		ENJON_CLASS_BODY( StaticMeshComponentSystem )

		public: 

			/*
			* @brief
			*/
			virtual void ExplicitConstructor( ) override;

			/*
			* @brief
			*/
			virtual void Update( ) override; 

		protected:

			Result PostComponentConstruction( const u32& id, IComponentInstanceData* data ); 
	};
}

#endif