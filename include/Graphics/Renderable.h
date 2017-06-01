#pragma once
#ifndef ENJON_RENDERABLE_H
#define ENJON_RENDERABLE_H

#include "Defines.h"
#include "Math/Transform.h"
#include "Math/Quaternion.h"
#include "Graphics/Material.h"
#include "Graphics/Shader.h"
#include "System/Types.h"

namespace Enjon 
{ 
	class Mesh; 			
	class Scene;
	class DeferredRenderer;

	class Renderable
	{
		friend Scene;
		friend DeferredRenderer;

		public:
			Renderable();
			~Renderable();

			/* Get position of transform */
			Vec3 GetPosition() const;

			/* Get scale of transform */
			Vec3 GetScale() const;

			/* Get rotation of transform */
			Quaternion GetRotation() const;

			/* Offset rotation of transform */
			void Renderable::OffsetRotation(const f32& Yaw, const f32& Pitch);

			/* Get material of renderable */
			Material* GetMaterial();

			/* Get mesh of renderable */
			AssetHandle<Mesh> GetMesh() const;

			/* Get scene of renderable */
			Scene* GetScene() const;

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
			void SetMaterial(Material* material);

			/* Set mesh of renderable */
			void SetMesh(const AssetHandle<Mesh>& mesh);

			/* Set scene of renderable */
			void SetScene(Scene* scene);

			/* Set material color */
			void SetColor(TextureSlotType type, const ColorRGBA16& color);	

		protected:
			/*
			* @brief
			*/
			void Submit( const Enjon::Shader* shader );

		private:
			Transform 			mTransform;
			AssetHandle<Mesh> 	mMesh;
			Material* 	mMaterial 	= nullptr;
			Scene* 		mScene 		= nullptr;
	};
}


#endif