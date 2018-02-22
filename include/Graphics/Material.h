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
	class MaterialAssetLoader;

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

	ENJON_CLASS( Construct )
	class Material : public Asset
	{
		ENJON_CLASS_BODY( )

		friend Shader;
		friend ShaderGraph;
		friend MaterialAssetLoader;

		public:

			/*
			* @brief
			*/
			Material();
			
			/*
			* @brief
			*/
			Material( const ShaderGraph* shaderGraph );
			
			/*
			* @brief
			*/
			Material( const AssetHandle< ShaderGraph >& shaderGraph );

			/*
			* @brief
			*/
			~Material(); 

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
			bool HasOverride( const String& uniformName ) const;

			/*
			* @brief
			*/
			const ShaderUniform* GetOverride( const String& uniformName ) const;

			/*
			* @brief
			*/
			void Bind( const Shader* shader ) const;

			/*
			* @brief
			*/
			void SetUniform( const String& name, const AssetHandle< Texture >& value ); 

			/*
			* @brief
			*/
			void SetUniform( const String& name, const Vec2& value ); 

			/*
			* @brief
			*/
			void SetUniform( const String& name, const Vec3& value ); 

			/*
			* @brief
			*/
			void SetUniform( const String& name, const Vec4& value );

			/*
			* @brief
			*/
			void SetUniform( const String& name, const Mat4& value );

			/*
			* @brief
			*/
			void SetUniform( const String& name, const f32& value ); 

			/*
			* @brief
			*/
			void SetShaderGraph( const AssetHandle< ShaderGraph >& graph ) const; 

			/*
			* @brief
			*/
			AssetHandle< ShaderGraph > GetShaderGraph( ) const;

		protected:

			/*
			* @brief
			*/
			void AddOverride( ShaderUniform* uniform );

		protected: 
			ENJON_PROPERTY( Editable )
			AssetHandle< ShaderGraph > mShaderGraph; 
			
			ENJON_PROPERTY( )
			HashMap< String, ShaderUniform* > mUniformOverrides;

			ENJON_PROPERTY( Editable ) 
			bool mTwoSided = false; 
	}; 
}


#endif