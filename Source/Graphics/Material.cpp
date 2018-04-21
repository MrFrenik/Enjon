#include "Graphics/Material.h"
#include "Graphics/GLSLProgram.h"
#include "Asset/MaterialAssetLoader.h"
#include "System/Types.h"
#include "Graphics/GraphicsSubsystem.h"
#include "ImGui/ImGuiManager.h"
#include "Asset/AssetManager.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <fmt/format.h>
#include <assert.h>

//#include <windows.h>

namespace Enjon 
{ 
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
		// Free memory
		ClearAllOverrides( );
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

	void Material::ClearAllOverrides( )
	{
		for ( auto& u : mUniformOverrides )
		{
			delete( u.second );
			u.second = nullptr;
		}

		mUniformOverrides.clear( );
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

	void Material::SetUniform( const String& name, const Mat4x4& value )
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

	Result Material::OnEditorUI( )
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );
		AssetManager* am = EngineSubsystem( AssetManager );

		// Get shadergraph assets in asset manager
		const HashMap< String, AssetRecordInfo >* shaderGraphs = am->GetAssets< ShaderGraph >( );

		// List shadergraph
		const MetaClass* cls = Object::GetClass< ShaderGraph >( );
		String label = mShaderGraph ? mShaderGraph->GetName( ) : cls->GetName( );
		if ( ImGui::BeginCombo( fmt::format("##{}", "ShaderGraph" ).c_str(), label.c_str() ) )
		{
			// For each record in assets
			for ( auto& a : *shaderGraphs )
			{
				if ( ImGui::Selectable( a.second.GetAssetName().c_str( ) ) )
				{ 
					// Get asset and load into memory if unloaded
					const Asset* sg = a.second.GetAsset( );

					// Clear all uniform overrides if not the same shadergraph as before
					if ( mShaderGraph.Get( ) != sg )
					{ 
						ClearAllOverrides( );
					}

					// Set shadergraph
					mShaderGraph = a.second.GetAsset( ); 
				}
			}
			ImGui::EndCombo( );
		} 
 
		// Display all uniforms of shader graph
		if ( mShaderGraph )
		{
			for ( auto& u : *( mShaderGraph->GetUniforms( ) ) )
			{
				// Get the uniform
				ShaderUniform* uniform = u.second;

				bool bHasOverride = false;

				// If the uniform exists then display its information
				if ( HasOverride( uniform->GetName() ) )
				{
					uniform = GetOverride( uniform->GetName( ) )->ConstCast< ShaderUniform >( );
					bHasOverride = true;
				} 

				switch ( uniform->GetType( ) )
				{
					case UniformType::Float:
					{ 
						// Display uniform name
						ImGui::Text( uniform->GetName( ).c_str() );
						ImGui::SameLine( );

						// Get value from uniform
						UniformFloat* uFloat = uniform->ConstCast< UniformFloat >( );
						f32 val = uFloat->GetValue( );

						if ( ImGui::DragFloat( fmt::format( "##{}", uniform->GetName() ).c_str( ), &val ) )
						{
							// If override exists, set value
							if ( bHasOverride )
							{
								uFloat->SetValue( val );
							}
							// Otherwise, need to construct new uniform
							else
							{ 
								// Construct new uniform float and copy fields of other
								UniformFloat* newUniform = new UniformFloat( );
								newUniform->CopyFields( uFloat );

								// Set new value on new uniform
								newUniform->SetValue( val );

								// Add uniform to overrides
								AddOverride( newUniform );
							}
						} 
					} break;

					case UniformType::Vec2:
					{ 
						// Display uniform name
						ImGui::Text( uniform->GetName( ).c_str() );
						ImGui::SameLine( );

						// Get value from uniform
						UniformVec2* uVec2 = uniform->ConstCast< UniformVec2 >( );
						Vec2 val = uVec2->GetValue( );
						f32 vals[2] = { val.x, val.y };

						if ( ImGui::DragFloat2( fmt::format( "##{}", uVec2->GetName() ).c_str( ), (f32*)vals ) )
						{
							// If override exists, set value
							if ( bHasOverride )
							{
								uVec2->SetValue( Vec2( vals[0], vals[1] ) );
							}
							// Otherwise, need to construct new uniform
							else
							{ 
								// Construct new uniform float and copy fields of other
								UniformVec2* newUniform = new UniformVec2( );
								newUniform->CopyFields( uVec2 );

								// Set new value on new uniform
								newUniform->SetValue( val );

								// Add uniform to overrides
								AddOverride( newUniform );
							}
						} 
					} break;

					case UniformType::Vec3:
					{ 
						// Display uniform name
						ImGui::Text( uniform->GetName( ).c_str() );
						ImGui::SameLine( );

						// Get value from uniform
						UniformVec3* uVec3 = uniform->ConstCast< UniformVec3 >( );
						Vec3 val = uVec3->GetValue( );
						f32 vals[3] = { val.x, val.y, val.z };

						if ( ImGui::DragFloat3( fmt::format( "##{}", uVec3->GetName() ).c_str( ), (f32*)vals ) )
						{
							// If override exists, set value
							if ( bHasOverride )
							{
								uVec3->SetValue( Vec3( vals[0], vals[1], vals[2] ) );
							}
							// Otherwise, need to construct new uniform
							else
							{ 
								// Construct new uniform float and copy fields of other
								UniformVec3* newUniform = new UniformVec3( );
								newUniform->CopyFields( uVec3 );

								// Set new value on new uniform
								newUniform->SetValue( val );

								// Add uniform to overrides
								AddOverride( newUniform );
							}
						} 
					} break;

					case UniformType::Vec4:
					{ 
						// Display uniform name
						ImGui::Text( uniform->GetName( ).c_str() );
						ImGui::SameLine( );

						// Get value from uniform
						UniformVec4* uVec4 = uniform->ConstCast< UniformVec4 >( );
						Vec4 val = uVec4->GetValue( );
						f32 vals[4] = { val.x, val.y, val.z, val.w };

						if ( ImGui::DragFloat4( fmt::format( "##{}", uVec4->GetName() ).c_str( ), (f32*)vals ) )
						{
							// If override exists, set value
							if ( bHasOverride )
							{
								uVec4->SetValue( Vec4( vals[0], vals[1], vals[2], vals[3] ) );
							}
							// Otherwise, need to construct new uniform
							else
							{ 
								// Construct new uniform float and copy fields of other
								UniformVec4* newUniform = new UniformVec4( );
								newUniform->CopyFields( uVec4 );

								// Set new value on new uniform
								newUniform->SetValue( val );

								// Add uniform to overrides
								AddOverride( newUniform );
							}
						} 
					} break;

					case UniformType::TextureSampler2D:
					{
						// Display uniform name
						ImGui::Text( uniform->GetName( ).c_str( ) );
						ImGui::SameLine( );

						// Get value from uniform
						UniformTexture* uTexture = uniform->ConstCast< UniformTexture >( );
						AssetHandle< Texture > tex = uTexture->GetTexture( );

						String comboLabel = tex ? tex->GetName( ) : "Texture Uniform";

						if ( ImGui::BeginCombo( fmt::format( "##{}", uTexture->GetName( ) ).c_str( ), comboLabel.c_str( ) ) )
						{
							// For each record in assets
							for ( auto& a : *am->GetAssets< Texture >() )
							{
								if ( ImGui::Selectable( a.second.GetAssetName().c_str( ) ) )
								{ 
									// If override exists, simply set the texture
									if ( bHasOverride )
									{
										uTexture->SetTexture( a.second.GetAsset( ) );
									}
									else
									{
										// Construct new texture uniform and copy fields from original
										UniformTexture* newUniform = new UniformTexture( );
										newUniform->CopyFields( uTexture );

										// Set new texture value
										newUniform->SetTexture( a.second.GetAsset( ) );

										// Add override
										AddOverride( newUniform ); 
									}
								}
							} 
							ImGui::EndCombo( );
						} 

					} break;
				}
			}
		}

		// Option to save the material
		if ( ImGui::Button( "Save Material" ) )
		{
			Save( );
		}

		return Result::SUCCESS;
	}

	//========================================================================================= 
}



















