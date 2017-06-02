#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"

#include <assert.h>

namespace Enjon {

	//------------------------------------------------------------------------
	Material::Material()
		: mShader(nullptr)
	{
	}

	//------------------------------------------------------------------------
	Material::~Material()
	{
	} 
	
	//------------------------------------------------------------------------
	
	void Material::SetTexture(const TextureSlotType& type, const AssetHandle<Texture>& textureHandle)
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		mTextureHandles[(u32)type] = textureHandle;
	}

	AssetHandle<Texture> Material::GetTexture(const TextureSlotType& type) const
	{ 
		assert((u32)type < (u32)TextureSlotType::Count);
		return mTextureHandles[(u32)type];
	}

	//------------------------------------------------------------------------
	void Material::SetColor(TextureSlotType type, const ColorRGBA16& color)
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		mColors[(u32)type] = color;
	}

	//------------------------------------------------------------------------
	ColorRGBA16 Material::GetColor(TextureSlotType type) const
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		return mColors[(u32)type];
	}

	//------------------------------------------------------------------------

	//GLSLProgram* Material::GetShader() 
	//{
	//	return mShader;
	//}

	//------------------------------------------------------------------------

	//void Material::SetShader(GLSLProgram* shader)
	//{
	//	mShader = shader;

	//}

	//========================================================================

	void Material::SetShader( const Enjon::Shader* shader )
	{
		mMaterialShader = shader;
	}

	//========================================================================

	const Enjon::Shader* Material::GetShader( ) const
	{
		return mMaterialShader;
	}

	//========================================================================
			
	void Material::AddUniform( ShaderUniform* uniform )
	{
		auto query = mUniforms.find( uniform->GetName( ) );
		if ( query == mUniforms.end( ) )
		{
			mUniforms[ uniform->GetName( ) ] = uniform;
		}
	} 

	//========================================================================

	void Material::SetUniforms( )
	{
		// Make sure that material shader is valid
		assert( mMaterialShader != nullptr );

		// Iterate through uniforms and set with shader
		for ( auto& u : mUniforms )
		{
			switch ( u.second->GetType() )
			{
				case UniformType::TextureSampler:
				{
					UniformTexture* texUni = u.second->Cast< UniformTexture >( );
					const_cast< Enjon::Shader* >( mMaterialShader )->BindTexture( texUni->GetName( ), texUni->GetTexture( ).Get( )->GetTextureId(), texUni->GetLocation( ) );
				} break;
				
				case UniformType::Float:
				{
					UniformPrimitive< f32 >* uni = u.second->Cast< UniformPrimitive< f32 > >( );
					const_cast< Enjon::Shader* >( mMaterialShader )->SetUniform( uni->GetName( ), uni->GetValue( ) );
				}

				case UniformType::Vec2:
				{
					UniformPrimitive< Vec2 >* uni = u.second->Cast< UniformPrimitive< Vec2 > >( );
					const_cast< Enjon::Shader* >( mMaterialShader )->SetUniform( uni->GetName( ), uni->GetValue( ) );
				}
				
				case UniformType::Vec3:
				{
					UniformPrimitive< Vec3 >* uni = u.second->Cast< UniformPrimitive< Vec3 > >( );
					const_cast< Enjon::Shader* >( mMaterialShader )->SetUniform( uni->GetName( ), uni->GetValue( ) );
				}
				
				case UniformType::Vec4:
				{
					UniformPrimitive< Vec4 >* uni = u.second->Cast< UniformPrimitive< Vec4 > >( );
					const_cast< Enjon::Shader* >( mMaterialShader )->SetUniform( uni->GetName( ), uni->GetValue( ) );
				}
				
				case UniformType::Mat4:
				{
					UniformPrimitive< Mat4 >* uni = u.second->Cast< UniformPrimitive< Mat4 > >( );
					const_cast< Enjon::Shader* >( mMaterialShader )->SetUniform( uni->GetName( ), uni->GetValue( ) );
				}

				default:
				{

				} break;
			}
		}
	}
			
	void Material::SetUniform( const Enjon::String& name, const Enjon::AssetHandle< Enjon::Texture >& value )
	{
		auto query = mUniforms.find( name );
		if ( query != mUniforms.end( ) )
		{
			ShaderUniform* uniform = mUniforms[ name ];
			if ( uniform->GetType( ) == Enjon::UniformType::TextureSampler )
			{
				uniform->Cast< UniformTexture >( )->SetTexture( value );
			}
		}
	}

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec2& value )
	{
		auto query = mUniforms.find( name );
		if ( query != mUniforms.end( ) )
		{
			ShaderUniform* uniform = mUniforms[ name ];
			if ( uniform->GetType( ) == Enjon::UniformType::Vec2 )
			{
				uniform->Cast< UniformPrimitive< Vec2 > >( )->SetValue( value );
			}
		} 
	} 

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec3& value )
	{
		auto query = mUniforms.find( name );
		if ( query != mUniforms.end( ) )
		{
			ShaderUniform* uniform = mUniforms[ name ];
			if ( uniform->GetType( ) == Enjon::UniformType::Vec3 )
			{
				uniform->Cast< UniformPrimitive< Vec3 > >( )->SetValue( value );
			}
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec4& value )
	{ 
		auto query = mUniforms.find( name );
		if ( query != mUniforms.end( ) )
		{
			ShaderUniform* uniform = mUniforms[ name ];
			if ( uniform->GetType( ) == Enjon::UniformType::Vec4 )
			{
				uniform->Cast< UniformPrimitive< Vec4 > >( )->SetValue( value );
			}
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Mat4& value )
	{ 
		auto query = mUniforms.find( name );
		if ( query != mUniforms.end( ) )
		{
			ShaderUniform* uniform = mUniforms[ name ];
			if ( uniform->GetType( ) == Enjon::UniformType::Mat4 )
			{
				uniform->Cast< UniformPrimitive< Mat4 > >( )->SetValue( value );
			}
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const f32& value )
	{ 
		auto query = mUniforms.find( name );
		if ( query != mUniforms.end( ) )
		{
			ShaderUniform* uniform = mUniforms[ name ];
			if ( uniform->GetType( ) == Enjon::UniformType::Float )
			{
				uniform->Cast< UniformPrimitive< f32 > >( )->SetValue( value );
			}
		} 
	}

	//=========================================================================================
}