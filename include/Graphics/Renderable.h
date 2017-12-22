#pragma once
#ifndef ENJON_RENDERABLE_H
#define ENJON_RENDERABLE_H

#include "Defines.h"
#include "Math/Transform.h"
#include "Math/Quaternion.h"
#include "System/Types.h"
#include "Asset/Asset.h"
#include "Base/Object.h"
#include "Graphics/Mesh.h"

namespace Enjon 
{ 
	class Scene;
	class Shader;
	class Material;
	class GraphicsSubsystem;
	class GLSLProgram;
	class ColorRGBA32;

	ENJON_CLASS( )
	class Renderable : public Enjon::Object
	{ 
		friend Scene;
		friend GraphicsSubsystem;

		ENJON_CLASS_BODY( )

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
			const Material* GetMaterial() const;

			/* Get mesh of renderable */
			AssetHandle< Mesh > GetMesh() const;

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
			void SetMaterial(const Material* material);

			/* Set material of renderable */
			//void SetMaterial( const AssetHandle< Material >& material );

			/* Set mesh of renderable */
			void SetMesh(const AssetHandle<Mesh>& mesh);

			/* Set scene of renderable */
			void SetScene(Scene* scene);

			/* Set material color */
			//void SetColor(TextureSlotType type, const ColorRGBA32& color);	

		protected:
			/*
			* @brief
			*/
			void Submit( const Enjon::Shader* shader );
	
			/*
			* @brief
			*/
			void Submit( const Enjon::GLSLProgram* shader );

		private: 
			// Enjon Properties
			ENJON_PROPERTY( Editable )
			AssetHandle<Mesh> mMesh;

			ENJON_PROPERTY( )
			const Material* mMaterial = nullptr;

		private: 
			Transform mTransform; 
			Scene* mScene = nullptr;
	};
}


#endif