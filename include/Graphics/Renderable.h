#pragma once
#ifndef ENJON_RENDERABLE_H
#define ENJON_RENDERABLE_H

#include "Defines.h"
#include "Math/Transform.h"
#include "Math/Quaternion.h"
#include "Graphics/Material.h"
#include "System/Types.h"

namespace Enjon {

	class Mesh; 			
	class Scene;

	class Renderable
	{
		friend Scene;

		public:
			Renderable();
			~Renderable();

			/* Get position of transform */
			EM::Vec3 GetPosition() { return mTransform.GetPosition(); }

			/* Get scale of transform */
			EM::Vec3 GetScale() { return mTransform.GetScale(); }

			/* Get rotation of transform */
			EM::Quaternion GetRotation() { return mTransform.GetRotation(); }

			/* Offset rotation of transform */
			void Renderable::OffsetRotation(const f32& Yaw, const f32& Pitch);

			/* Get material of renderable */
			Material* GetMaterial() { return mMaterial; }

			/* Get mesh of renderable */
			Mesh* GetMesh() { return mMesh; }

			/* Get scene of renderable */
			Scene* GetScene() { return mScene; }

			/* Gets world transform */
			EM::Transform GetTransform() { return mTransform; }

			/* Sets world transform */
			void SetTransform(EM::Transform& transform) { mTransform = transform; }

			/* Set position of transform */
			void SetPosition(EM::Vec3& position);

			/* Set scale of transform */
			void SetScale(EM::Vec3& scale);

			/* Set uniform scale of transform */
			void SetScale(float scale);

			/* Set rotation of transform */
			void SetRotation(EM::Quaternion& rotation);

			/* Set material of renderable */
			void SetMaterial(Material* material);

			/* Set mesh of renderable */
			void SetMesh(Mesh* mesh);

			/* Set scene of renderable */
			void SetScene(Scene* scene);

			/* Set material color */
			void SetColor(TextureSlotType type, ColorRGBA16& color);	

		private:
			EM::Transform 	mTransform;
			Mesh* 		mMesh 		= nullptr;
			Material* 	mMaterial 	= nullptr;
			Scene* 		mScene 		= nullptr;
	};
}


#endif