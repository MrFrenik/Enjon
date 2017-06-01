#pragma once
#ifndef ENJON_MATERIAL_H
#define ENJON_MATERIAL_H 

#include "Graphics/GLTexture.h"
#include "Graphics/Texture.h"
#include "Graphics/Color.h"
#include "Graphics/Shader.h"
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

		public:
			Material();
			~Material(); 

			void SetTexture(const TextureSlotType& type, const AssetHandle<Texture>& textureHandle);
			AssetHandle<Texture> GetTexture(const TextureSlotType& type) const;

			void SetColor(TextureSlotType type, const ColorRGBA16& color);
			ColorRGBA16 GetColor(TextureSlotType type) const;

			//GLSLProgram* GetShader();
			//void SetShader(GLSLProgram* shader);

			void SetShader( const Enjon::Shader* shader );
			const Enjon::Shader* GetShader( ) const;

			bool TwoSided( ) const { return mTwoSided; }
			void TwoSided( bool enable ) { mTwoSided = enable; }

			template < typename T >
			void SetUniform( const Enjon::String& name, const T& value )
			{
				auto query = mUniforms.find( name );
				if ( query != mUniforms.end( ) )
				{
					ShaderUniform* uniform = mUniforms[ name ];
					switch ( uniform->GetType() )
					{
						case Enjon::UniformType::TextureSampler:
						{
							static_cast< UniformTexture* > ( uniform )->SetTexture( value );
						} break;

						//case Enjon::UniformType::Vec4:
						//case Enjon::UniformType::Vec3:
						//case Enjon::UniformType::Vec2:
						//case Enjon::UniformType::Mat4:
						//case Enjon::UniformType::Float:
						//{
						//	static_cast< UniformPrimitive< T >* > ( uniform )->SetValue( value );
						//} break;

						default:
						{ 
						} break;
					}
				}
			}

			/*
			* @brief
			*/
			void SetUniforms( );

		protected:
			void AddUniform( ShaderUniform* uniform );

		protected:
			AssetHandle<Texture> mTextureHandles[(u32)TextureSlotType::Count]; 
			ColorRGBA16 mColors[(u32)TextureSlotType::Count];
			GLSLProgram* mShader = nullptr;
			bool mTwoSided = false;
			std::unordered_map< Enjon::String, ShaderUniform* > mUniforms;
			const Enjon::Shader* mMaterialShader = nullptr;
	}; 
}


#endif