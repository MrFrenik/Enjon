#pragma once
#ifndef ENJON_GRAPHICS_COMPONENT_H
#define ENJON_GRAPHICS_COMPONENT_H

#include "Entity/Component.h"
#include "Graphics/Renderable.h"
#include "Graphics/Material.h"
#include "System/Types.h"

namespace Enjon
{
	class GraphicsComponent : public Component
	{
		public:
			GraphicsComponent();
			GraphicsComponent(EG::Renderable& renderable);
			~GraphicsComponent();

			virtual void Update(float dt) override;

			virtual void Destroy() override;

			/* Get position of transform */
			EM::Vec3 GetPosition() { return mRenderable.GetPosition(); }

			/* Get scale of transform */
			EM::Vec3 GetScale() { return mRenderable.GetScale(); }

			/* Get orientation of transform */
			EM::Quaternion GetRotation() { return mRenderable.GetRotation(); }

			/* Get material of renderable */
			EG::Material* GetMaterial() { return mRenderable.GetMaterial(); }

			/* Get mesh of renderable */
			EG::Mesh* GetMesh() { return mRenderable.GetMesh(); }

			/* Get scene of renderable */
			EG::Scene* GetScene() { return mRenderable.GetScene(); }

			/* Get world transform */
			EM::Transform GetTransform() { return mRenderable.GetTransform(); }

			/* Sets world transform */
			void SetTransform(EM::Transform& transform) { mRenderable.SetTransform(transform); }

			/* Get renderable */
			EG::Renderable* GetRenderable() { return &mRenderable; }

			/* Set position of transform */
			void SetPosition(EM::Vec3& position);

			/* Set scale of transform */
			void SetScale(EM::Vec3& scale);

			/* Set scale of transform */
			void SetScale(f32 scale);

			/* Set orientation of transform */
			void SetRotation(EM::Quaternion& rotation);

			/* Set material of renderable */
			void SetMaterial(EG::Material* material);

			/* Set mesh of renderable */
			void SetMesh(EG::Mesh* mesh);

			/* Set scene of renderable */
			void SetScene(EG::Scene* scene);

			/* Set Renderable */
			void SetRenderable(EG::Renderable& renderable);

			/* Set material color */
			void SetColor(EG::TextureSlotType type, EG::ColorRGBA16& color);	

		private:

			EG::Renderable mRenderable;
	};
}

#endif