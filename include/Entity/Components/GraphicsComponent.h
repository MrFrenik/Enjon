#pragma once
#ifndef ENJON_GRAPHICS_COMPONENT_H
#define ENJON_GRAPHICS_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/Renderable.h"
#include "Graphics/Material.h"
#include "System/Types.h"

namespace Enjon
{
	ENJON_CLASS( Construct )
	class GraphicsComponent : public Component
	{
		ENJON_CLASS_BODY( ) 

		// Don't really like that I have to do BOTH of these... Would rather this just be within one single macro or at least generated...
		ENJON_COMPONENT( GraphicsComponent )

		public:

			/* 
			* @brief
			*/
			GraphicsComponent();

			/*
			* @brief
			*/
			GraphicsComponent(const Renderable& renderable);

			/*
			* @brief
			*/
			~GraphicsComponent();

			/*
			* @brief
			*/
			virtual void PostConstruction( ) override;

			/*
			* @brief
			*/
			virtual void Update( const f32& dT) override; 
 
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
			AssetHandle< Material > GetMaterial() const;

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
			Renderable* GetRenderable();

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
			void SetMaterial( const AssetHandle< Material >& material );

			/* 
			* @brief Set mesh of renderable 
			*/
			void SetMesh(const AssetHandle<Mesh>& mesh);

			/* 
			* @brief Set scene of renderable 
			*/
			void SetGraphicsScene(GraphicsScene* scene);

			/* 
			* @brief Set Renderable 
			*/
			void SetRenderable(const Renderable& renderable); 

			/*
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/*
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

		protected:


		private: 
			
			ENJON_PROPERTY( )
			Renderable mRenderable;
	};
}

#endif