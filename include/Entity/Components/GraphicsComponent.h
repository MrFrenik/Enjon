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
			GraphicsComponent(const Renderable& renderable);
			~GraphicsComponent();

			virtual void Update(f32 dT) override;

			virtual void Destroy() override;

			/* Get position of transform */
			Vec3 GetPosition() const;

			/* Get scale of transform */
			Vec3 GetScale() const;

			/* Get orientation of transform */
			Quaternion GetRotation() const;

			/* Get material of renderable */
			Material* GetMaterial();

			/* Get mesh of renderable */
			Mesh* GetMesh() const;

			/* Get scene of renderable */
			Scene* GetScene() const;

			/* Get world transform */
			Transform GetTransform() const;
			
			/* Get renderable */
			Renderable* GetRenderable();

			/* Sets world transform */
			void SetTransform(const Transform& transform); 

			/* Set position of transform */
			void SetPosition(const Vec3& position);

			/* Set scale of transform */
			void SetScale(const Vec3& scale);

			/* Set scale of transform */
			void SetScale(const f32& scale);

			/* Set orientation of transform */
			void SetRotation(const Quaternion& rotation);

			/* Set material of renderable */
			void SetMaterial(Material* material);

			/* Set mesh of renderable */
			void SetMesh(Mesh* mesh);

			/* Set scene of renderable */
			void SetScene(Scene* scene);

			/* Set Renderable */
			void SetRenderable(const Renderable& renderable);

			/* Set material color */
			void SetColor(TextureSlotType type, const ColorRGBA16& color);	

		private: 

			Renderable mRenderable;
	};
}

#endif