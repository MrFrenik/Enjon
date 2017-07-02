#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"

#include <assert.h>

namespace Enjon {

	//------------------------------------------------------------------------
	Material::Material()
		: mShader(nullptr), mShaderGraph( nullptr )
	{
	}
	
	Material::Material( const Enjon::ShaderGraph* shaderGraph )
		: mShaderGraph( shaderGraph )
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
			
	void Material::AddOverride( ShaderUniform* uniform )
	{
		auto query = mUniformOverrides.find( uniform->GetName( ) );
		if ( query == mUniformOverrides.end( ) )
		{
			mUniformOverrides[ uniform->GetName( ) ] = uniform;
		}
	} 
 
	//========================================================================
			
	bool Material::HasOverride( const Enjon::String& uniformName )
	{
		return ( mUniformOverrides.find( uniformName ) != mUniformOverrides.end( ) );
	}
 
	//========================================================================

	const ShaderUniform* Material::GetOverride( const Enjon::String& uniformName )
	{
		if ( HasOverride( uniformName ) )
		{
			return mUniformOverrides[ uniformName ];
		}

		return nullptr;
	}

	//========================================================================
			
	const Enjon::ShaderGraph* Material::GetShaderGraph( ) const
	{
		return mShaderGraph;
	}

	//========================================================================
			
	void Material::Bind( const Shader* shader )
	{
		Enjon::ShaderGraph* sg = const_cast< ShaderGraph* > ( mShaderGraph );
		Enjon::Shader* sh = const_cast< Shader* > ( shader );
		if ( sh )
		{
			for ( auto& u : *sg->GetUniforms( ) )
			{ 
				Enjon::String uniformName = u.second->GetName( );
				
				if ( HasOverride( uniformName ) )
				{
					mUniformOverrides[ uniformName ]->Bind( sh );
					continue;
				}
				else
				{
					u.second->Bind( sh );
				}
			}
		} 
	} 

	//========================================================================
			
	void Material::SetUniform( const Enjon::String& name, const Enjon::AssetHandle< Enjon::Texture >& value )
	{
		if ( HasOverride( name ) )
		{ 
			mUniformOverrides[ name ]->Cast< UniformTexture >( )->SetTexture( value );
		}
		// If override doesn't exist
		else
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph );
			if ( sg->HasUniform( name ) )
			{
				UniformTexture* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformTexture >( );;
				UniformTexture* uniOverride = new UniformTexture( *uniform );
				uniOverride->SetTexture( value );
				AddOverride( uniOverride );
			}
		}
	}

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec2& value )
	{
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph );
			if ( sg->HasUniform( name ) )
			{
				UniformPrimitive< Vec2 >* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformPrimitive< Vec2 > >( );;
				UniformPrimitive< Vec2 >* uniOverride = new UniformPrimitive<Vec2>( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformPrimitive< Vec2 > >( )->SetValue( value );
		} 
	} 

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec3& value )
	{
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph );
			if ( sg->HasUniform( name ) )
			{
				UniformPrimitive< Vec3 >* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformPrimitive< Vec3 > >( );;
				UniformPrimitive< Vec3 >* uniOverride = new UniformPrimitive<Vec3>( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformPrimitive< Vec3 > >( )->SetValue( value );
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec4& value )
	{ 
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph );
			if ( sg->HasUniform( name ) )
			{
				UniformPrimitive< Vec4 >* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformPrimitive< Vec4 > >( );;
				UniformPrimitive< Vec4 >* uniOverride = new UniformPrimitive<Vec4>( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformPrimitive< Vec4 > >( )->SetValue( value );
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Mat4& value )
	{ 
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph );
			if ( sg->HasUniform( name ) )
			{
				UniformPrimitive< Mat4 >* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformPrimitive< Mat4 > >( );;
				UniformPrimitive< Mat4 >* uniOverride = new UniformPrimitive<Mat4>( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformPrimitive< Mat4 > >( )->SetValue( value );
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const f32& value )
	{ 
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph );
			if ( sg->HasUniform( name ) )
			{
				UniformPrimitive< f32 >* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformPrimitive< f32 > >( );;
				UniformPrimitive< f32 >* uniOverride = new UniformPrimitive<f32>( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformPrimitive< f32 > >( )->SetValue( value );
		} 
	}

	//=========================================================================================
}