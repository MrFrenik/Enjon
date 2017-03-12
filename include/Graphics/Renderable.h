#pragma once
#ifndef ENJON_RENDERABLE_H
#define ENJON_RENDERABLE_H

#include "Defines.h"
#include "Math/Transform.h"
#include "Math/Quaternion.h"
#include "Graphics/Material.h"

namespace Enjon { namespace Graphics { 

	class Mesh; 			
	class Scene;

	class Renderable
	{
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
			EG::Material* GetMaterial() { return mMaterial; }

			/* Get mesh of renderable */
			EG::Mesh* GetMesh() { return mMesh; }

			/* Get scene of renderable */
			EG::Scene* GetScene() { return mScene; }

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
			void SetMaterial(EG::Material* material);

			/* Set mesh of renderable */
			void SetMesh(EG::Mesh* mesh);

			/* Set scene of renderable */
			void SetScene(EG::Scene* scene);

			/* Set material color */
			void SetColor(EG::TextureSlotType type, EG::ColorRGBA16& color);	

		private:
			EM::Transform 	mTransform;
			EG::Mesh* 		mMesh 		= nullptr;
			EG::Material* 	mMaterial 	= nullptr;
			EG::Scene* 		mScene 		= nullptr;
	};
}}


#endif