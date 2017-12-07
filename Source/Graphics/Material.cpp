#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"

#include <assert.h>

namespace Enjon {
 
	//======================================================================== 

	Material::Material()
	{
		for ( u32 i = 0; i < 50; ++i )
		{
			mTestVector.push_back( i );
		}
	}
	
	//======================================================================== 
			
	Material::Material( const Enjon::AssetHandle< Enjon::ShaderGraph >& shaderGraph )
		: mShaderGraph( shaderGraph )
	{ 
		for ( u32 i = 0; i < 50; ++i )
		{
			mTestVector.push_back( i );
		}
	}
	
	//======================================================================== 

	Material::~Material()
	{
	} 
	
	//======================================================================== 
	
	void Material::SetTexture(const TextureSlotType& type, const AssetHandle<Texture>& textureHandle)
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		mTextureHandles[(u32)type] = textureHandle;
	}

	//========================================================================

	AssetHandle<Texture> Material::GetTexture(const TextureSlotType& type) const
	{ 
		assert((u32)type < (u32)TextureSlotType::Count);
		return mTextureHandles[(u32)type];
	} 

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
			
	Enjon::AssetHandle< Enjon::ShaderGraph > Material::GetShaderGraph( ) const
	{
		return mShaderGraph;
	}

	//========================================================================

	void Material::SetShaderGraph( const Enjon::AssetHandle< Enjon::ShaderGraph >& graph )
	{
		mShaderGraph = graph;
	}

	//=========================================================================================
			
	void Material::Bind( const Shader* shader )
	{
		Enjon::ShaderGraph* sg = const_cast< ShaderGraph* > ( mShaderGraph.Get( ) );
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
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
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
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformVec2* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformVec2 >( );;
				UniformVec2* uniOverride = new UniformVec2( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformVec2 >( )->SetValue( value );
		} 
	} 

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec3& value )
	{
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformVec3* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformVec3 >( );;
				UniformVec3* uniOverride = new UniformVec3( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformVec3 >( )->SetValue( value );
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Vec4& value )
	{ 
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformVec4* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformVec4 >( );;
				UniformVec4* uniOverride = new UniformVec4( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformVec4 >( )->SetValue( value );
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const Enjon::Mat4& value )
	{ 
		// Figure this one out later...
		/*
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformMat4* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformPrimitive< Mat4 > >( );;
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
		*/
	}

	//=========================================================================================

	void Material::SetUniform( const Enjon::String& name, const f32& value )
	{ 
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformFloat* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformFloat >( );;
				UniformFloat* uniOverride = new UniformFloat( *uniform );
				uniOverride->SetValue( value );
				AddOverride( uniOverride );
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformFloat >( )->SetValue( value );
		} 
	} 

	//========================================================================================= 
}



















