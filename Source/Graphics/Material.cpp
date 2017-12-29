#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"
#include "Asset/MaterialAssetLoader.h"
#include "System/Types.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <assert.h>

#include <windows.h>

namespace Enjon {
 
	//======================================================================== 

	Material::Material()
	{
	}
	
	//======================================================================== 
			
	Material::Material( const AssetHandle< ShaderGraph >& shaderGraph )
		: mShaderGraph( shaderGraph )
	{ 
	}
	
	//======================================================================== 

	Material::~Material()
	{
	} 
	
	//======================================================================== 
	
	void Material::SetTexture(const TextureSlotType& type, const AssetHandle<Texture>& textureHandle) const
	{
		assert((u32)type < (u32)TextureSlotType::Count);
		const_cast< Material* >( this )->mTextureHandles[(u32)type] = textureHandle;
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
			
	bool Material::HasOverride( const String& uniformName ) const
	{
		return ( mUniformOverrides.find( uniformName ) != mUniformOverrides.end( ) );
	}
 
	//========================================================================

	const ShaderUniform* Material::GetOverride( const String& uniformName ) const
	{
		if ( HasOverride( uniformName ) )
		{
			return const_cast< Material* >( this )->mUniformOverrides[ uniformName ];
		}

		return nullptr;
	}

	//========================================================================
			
	AssetHandle< ShaderGraph > Material::GetShaderGraph( ) const
	{
		return mShaderGraph;
	}

	//========================================================================

	void Material::SetShaderGraph( const AssetHandle< ShaderGraph >& graph ) const
	{
		const_cast< Material* >( this )->mShaderGraph = graph;
	}

	//=========================================================================================
			
	void Material::Bind( const Shader* shader ) const
	{
		ShaderGraph* sg = const_cast< ShaderGraph* > ( mShaderGraph.Get( ) );
		Shader* sh = const_cast< Shader* > ( shader );
		if ( sh )
		{
			for ( auto& u : *sg->GetUniforms( ) )
			{ 
				String uniformName = u.second->GetName( );
				
				if ( HasOverride( uniformName ) )
				{
					ShaderUniform* uniform = const_cast< Material *> ( this )->mUniformOverrides[ uniformName ];
					if ( uniform )
					{
						uniform->Bind( sh );
					}
				}
				else
				{
					u.second->Bind( sh );
				}
			}
		} 
	} 

	//======================================================================== 
			
	void Material::SetUniform( const String& name, const AssetHandle< Texture >& value )
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

				// Construct new override from graphics subsystem
				const GraphicsSubsystem* gfx = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem > ( );
				UniformTexture* uniOverride = (UniformTexture*)gfx->NewShaderUniform( Object::GetClass< UniformTexture >() );
				if ( uniOverride )
				{
					uniOverride->CopyFields( uniform );
					uniOverride->SetTexture( value ); 
					AddOverride( uniOverride );
				} 
			}
		}
	}

	void Material::SetUniform( const String& name, const Vec2& value )
	{
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformVec2* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformVec2 >( );;

				// Construct new override from graphics subsystem
				const GraphicsSubsystem* gfx = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem > ( );
				UniformVec2* uniOverride = (UniformVec2*)gfx->NewShaderUniform( Object::GetClass< UniformVec2 >() );
				if ( uniOverride )
				{
					uniOverride->CopyFields( uniform );
					uniOverride->SetValue( value ); 
					AddOverride( uniOverride );
				} 
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformVec2 >( )->SetValue( value );
		} 
	} 

	//=========================================================================================

	void Material::SetUniform( const String& name, const Vec3& value )
	{
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformVec3* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformVec3 >( );;

				// Construct new override from graphics subsystem
				const GraphicsSubsystem* gfx = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem > ( );
				UniformVec3* uniOverride = (UniformVec3*)gfx->NewShaderUniform( Object::GetClass< UniformVec3 >() );
				if ( uniOverride )
				{
					uniOverride->CopyFields( uniform );
					uniOverride->SetValue( value ); 
					AddOverride( uniOverride );
				} 
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformVec3 >( )->SetValue( value );
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const String& name, const Vec4& value )
	{ 
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformVec4* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformVec4 >( );;

				// Construct new override from graphics subsystem
				const GraphicsSubsystem* gfx = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem > ( );
				UniformVec4* uniOverride = (UniformVec4*)gfx->NewShaderUniform( Object::GetClass< UniformVec4 >() );
				if ( uniOverride )
				{
					uniOverride->CopyFields( uniform );
					uniOverride->SetValue( value ); 
					AddOverride( uniOverride );
				} 
			}
		}
		// Otherwise set override
		else
		{
			mUniformOverrides[ name ]->Cast< UniformVec4 >( )->SetValue( value );
		} 
	}

	//=========================================================================================

	void Material::SetUniform( const String& name, const Mat4& value )
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

	void Material::SetUniform( const String& name, const f32& value )
	{ 
		// If override doesn't exist
		if ( !HasOverride( name ) )
		{
			ShaderGraph* sg = const_cast< ShaderGraph* >( mShaderGraph.Get( ) );
			if ( sg->HasUniform( name ) )
			{
				UniformFloat* uniform = const_cast< ShaderUniform* >( sg->GetUniform( name ) )->Cast< UniformFloat >( );;

				// Construct new override from graphics subsystem
				const GraphicsSubsystem* gfx = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< GraphicsSubsystem > ( );
				UniformFloat* uniOverride = (UniformFloat*)gfx->NewShaderUniform( Object::GetClass< UniformFloat >() );
				if ( uniOverride )
				{
					uniOverride->CopyFields( uniform );
					uniOverride->SetValue( value ); 
					AddOverride( uniOverride );
				} 
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



















