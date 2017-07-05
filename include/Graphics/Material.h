#pragma once
#ifndef ENJON_MATERIAL_H
#define ENJON_MATERIAL_H 

#include "Graphics/GLTexture.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderGraph.h"
#include "System/Types.h"
#include "Asset/Asset.h"

#include <unordered_map>

namespace Enjon { 

	class GLSLProgram;

	enum class TextureSlotType
	{
		Albedo,
		Normal,
		Emissive,
		Metallic,
		Roughness,
		AO,
		Count
	};

	class Material : public Asset
	{
		ENJON_OBJECT( Material )

		friend Shader;
		friend ShaderGraph;

		public:

			/*
			* @brief
			*/
			Material();
			
			/*
			* @brief
			*/
			Material( const Enjon::ShaderGraph* shaderGraph );

			/*
			* @brief
			*/
			~Material(); 

			/*
			* @brief
			*/
			void SetTexture(const TextureSlotType& type, const AssetHandle<Texture>& textureHandle);

			/*
			* @brief
			*/
			AssetHandle<Texture> GetTexture(const TextureSlotType& type) const; 

			/*
			* @brief
			*/
			bool TwoSided( ) const { return mTwoSided; }

			/*
			* @brief
			*/
			void TwoSided( bool enable ) { mTwoSided = enable; }

			/*
			* @brief
			*/
			bool HasOverride( const Enjon::String& uniformName );

			/*
			* @brief
			*/
			const ShaderUniform* GetOverride( const Enjon::String& uniformName );

			/*
			* @brief
			*/
			void Bind( const Shader* shader );

			/*
			* @brief
			*/
			void SetUniform( const Enjon::String& name, const Enjon::AssetHandle< Enjon::Texture >& value ); 

			/*
			* @brief
			*/
			void SetUniform( const Enjon::String& name, const Enjon::Vec2& value ); 

			/*
			* @brief
			*/
			void SetUniform( const Enjon::String& name, const Enjon::Vec3& value ); 

			/*
			* @brief
			*/
			void SetUniform( const Enjon::String& name, const Enjon::Vec4& value );

			/*
			* @brief
			*/
			void SetUniform( const Enjon::String& name, const Enjon::Mat4& value );

			/*
			* @brief
			*/
			void SetUniform( const Enjon::String& name, const f32& value ); 

			/*
			* @brief
			*/
			const Enjon::ShaderGraph* GetShaderGraph( ) const;


		protected:

			/*
			* @brief
			*/
			void AddOverride( ShaderUniform* uniform );

		protected:
			AssetHandle<Texture> mTextureHandles[(u32)TextureSlotType::Count]; 
			bool mTwoSided = false;
			std::unordered_map< Enjon::String, ShaderUniform* > mUniforms;
			std::unordered_map< Enjon::String, ShaderUniform* > mUniformOverrides;
			const Enjon::ShaderGraph* mShaderGraph = nullptr;
	}; 
}


#endif