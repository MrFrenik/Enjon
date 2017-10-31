// @file Shader.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SHADER_H
#define ENJON_SHADER_H 

#include "Asset/Asset.h"
#include "System/Types.h"
#include "Math/Vec4.h"
#include "Math/Mat4.h"
#include "Defines.h"
#include "Graphics/ShaderGraph.h"

#include <vector>

namespace Enjon
{ 
	class Material;
	class Texture; 

	ENJON_CLASS( )
	class Shader : public Asset
	{
		ENJON_CLASS_BODY( )

		public:
			/**
			* @brief Constructor
			*/
			Shader( );
			
			/**
			* @brief Constructor
			*/
			Shader( const AssetHandle<ShaderGraph>& graph, ShaderPassType passType );

			/**
			* @brief Destructor
			*/
			~Shader( ); 

			/*
			* @brief
			*/
			Enjon::Result Compile( ); 
			
			/*
			* @brief
			*/
			Enjon::Result Recompile( ); 

			/*
			* @brief
			*/
			Enjon::AssetHandle< Enjon::Material > CreateMaterial( );

			/*
			* @brief
			*/
			const Enjon::ShaderGraph* GetGraph( );

			/*
			* @brief
			*/
			void Use( );
			
			/*
			* @brief
			*/
			void Unuse( );

			s32 GetUniformLocation( const Enjon::String& uniformName );
			void SetUniform(const std::string& name, const s32& val);
			void SetUniform(const std::string& name, f32* val, s32 count);
			void SetUniform(const std::string& name, s32* val, s32 count);
			void SetUniform(const std::string& name, const f64& val);
			void SetUniform(const std::string& name, const f32& val);
			void SetUniform(const std::string& name, const Vec2& vector);
			void SetUniform(const std::string& name, const Vec3& vector);
			void SetUniform(const std::string& name, const Vec4& vector);
			void SetUniform(const std::string& name, const Mat4& matrix); 

			void BindTexture(const std::string& name, const u32& TextureID, const u32 Index);

		private:

			/*
			* @brief
			*/
			Enjon::Result CompileShader( const Enjon::String& shaderCode, u32 shaderID );
			
			/*
			* @brief
			*/
			Enjon::Result LinkProgram( );
			
			/*
			* @brief
			*/
			Enjon::Result DestroyProgram( );

		private: 
			u32 mProgramID			= 0; 
			u32 mVertexShaderID		= 0;
			u32 mFragmentShaderID	= 0;
			std::unordered_map< Enjon::String, u32 > mUniformMap;
			ShaderPassType mPassType;
			AssetHandle<ShaderGraph> mGraph;
	}; 
	
	class ShaderUniform
	{ 
		public:
			/*
			* @brief
			*/ 
			ShaderUniform( ) {}

			/*
			* @brief
			*/
			~ShaderUniform( ) {}

			/*
			* @brief
			*/
			virtual void Bind( const Shader* shader ) = 0;
			
			template < typename T >
			T* Cast( )
			{
				static_assert( std::is_base_of<ShaderUniform, T>::value, "ShaderUniform::Cast -  T must inherit from ShaderUniform." );	
				return static_cast< T* >( this );
			}

			/*
			* @brief
			*/
			const Enjon::String& GetName( ) const { return mName; }
			
			/*
			* @brief
			*/
			u32 GetLocation( ) const { return mLocation; }
			
			/*
			* @brief
			*/
			UniformType GetType( ) const { return mType; }

		protected: 
			UniformType mType;
			u32 mLocation = 0;
			Enjon::String mName;
	}; 

	class UniformTexture : public ShaderUniform
	{
		public:	
			/*
			* @brief
			*/
			UniformTexture( const Enjon::String& name, const Enjon::AssetHandle< Enjon::Texture >& texture, u32 location );
			
			/*
			* @brief
			*/
			~UniformTexture( ); 
			
			/*
			* @brief
			*/
			virtual void Bind( const Shader* shader ) override;

			/*
			* @brief
			*/
			void SetTexture( const Enjon::AssetHandle< Enjon::Texture >& texture )
			{
				mTexture = texture;
			}
			
			/*
			* @brief
			*/
			Enjon::AssetHandle< Enjon::Texture > GetTexture( ) const
			{
				return mTexture;
			}
 
		private:
			Enjon::AssetHandle< Enjon::Texture > mTexture;
	};

	template < typename T >
	class UniformPrimitive : public ShaderUniform
	{
		public:
			/*
			* @brief
			*/
			UniformPrimitive( const Enjon::String& name, const T& value, u32 location = 0 )
			{ 
				mName = name;
				mLocation = location;
				mValue = value;

				if ( std::is_base_of<Vec2, T>::value )
				{
					mType = UniformType::Vec2;
				} 
				else if ( std::is_base_of< Vec3, T>::value )
				{
					mType = UniformType::Vec3;
				}
				else if ( std::is_base_of< Vec4, T>::value )
				{
					mType = UniformType::Vec4;
				}
				else if ( std::is_base_of< Mat4, T >::value )
				{
					mType = UniformType::Mat4;
				}
				else if ( std::is_base_of< Enjon::AssetHandle< Enjon::Texture >, T >::value )
				{
					mType = UniformType::TextureSampler2D;
				}
				else
				{
					mType = UniformType::Float;
				} 
			}
			
			/*
			* @brief
			*/
			~UniformPrimitive( ) {}
			
			/*
			* @brief
			*/
			virtual void Bind( const Shader* shader ) override
			{
				const_cast< Enjon::Shader* >( shader )->SetUniform( mName, mValue );
			}

			void SetValue( const T& value )
			{
				mValue = value;
			}

			const T& GetValue( ) const { return mValue; }

		private:
			T mValue;
	}; 
}

#endif










