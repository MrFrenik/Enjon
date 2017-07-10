#include "Graphics/DeferredRenderer.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/GBuffer.h"
#include "Graphics/FullScreenQuad.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/FontManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Renderable.h"
#include "Graphics/Font.h"
#include "Graphics/FontManager.h"
#include "Graphics/Color.h"
#include "Graphics/DirectionalLight.h"
#include "Graphics/PointLight.h"
#include "Graphics/SpotLight.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderGraph.h"
#include "IO/ResourceManager.h"
#include "Asset/AssetManager.h"
#include "Console.h"
#include "CVarsSystem.h"
#include "ImGui/ImGuiManager.h"
#include "Graphics/Texture.h"
#include "Engine.h"
#include "Graphics/ShaderGraph.h"

#include <string>
#include <cassert>
#include <limits>
#include <random>

#include <STB/stb_image.h> 
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h> 
#include <fmt/string.h>
#include <fmt/printf.h>

Enjon::Renderable mRenderable; 
std::vector < Enjon::Renderable > mRenderables;
Enjon::AssetHandle< Enjon::Texture > mBRDFHandle;
Enjon::Material* mMaterial = nullptr;
bool brdfset = false;

bool useOther = false;

Enjon::ShaderGraph mShaderGraph;

namespace Enjon 
{ 
	//======================================================================================================

	DeferredRenderer::DeferredRenderer()
	{
	}

	//======================================================================================================

	DeferredRenderer::~DeferredRenderer()
	{
	}

	//======================================================================================================

	Enjon::Result DeferredRenderer::Shutdown()
	{
		delete(mGbuffer);
		delete(mDebugTarget);
		delete(mSmallBlurHorizontal);
		delete(mSmallBlurVertical);
		delete(mMediumBlurHorizontal);
		delete(mMediumBlurVertical);
		delete(mLargeBlurHorizontal);
		delete(mLargeBlurVertical);
		delete(mCompositeTarget);
		delete(mLightingBuffer);
		delete(mLuminanceTarget);
		delete(mFXAATarget);
		delete(mShadowDepth);

		return Result::SUCCESS; 
	}

	//======================================================================================================

	Enjon::Result DeferredRenderer::Initialize()
	{
		// TODO(John): Need to have a way to have an .ini that's read or grab these values from a static
		// engine config file
		// mWindow.Init("Game", 1920, 1080, WindowFlagsMask((u32)WindowFlags::FULLSCREEN)); 
		mWindow.Init( "Game", 1440, 900, WindowFlags::RESIZABLE ); 

		// Initialize shader manager
		Enjon::ShaderManager::Init();

		// Initialize font manager
		Enjon::FontManager::Init();

		// Initialize scene camera
		mSceneCamera = Enjon::Camera(mWindow.GetViewport());
		mSceneCamera.SetProjection(ProjectionType::Perspective);
		mSceneCamera.SetPosition(Vec3(0, 5, 10));
		mSceneCamera.LookAt(Vec3(0, 0, 0));

		// Initialize frame buffers
		InitializeFrameBuffers();
		// Calcualte blur weights
		CalculateBlurWeights();
		// Register cvars
		RegisterCVars();

		GLSLProgram* shader = Enjon::ShaderManager::Get("GBuffer");
		shader->Use();
			shader->SetUniform("u_albedoMap", 0);
			shader->SetUniform("u_normalMap", 1);
		shader->Unuse();

		mShowGraphicsOptionsWindow = true;
		auto graphicsMenuOption = [&]()
		{
        	ImGui::MenuItem("Graphics##options", NULL, &mShowGraphicsOptionsWindow);
		};

		mShowGame = true;
		auto showGameViewportFunc = [&]()
		{
			// Docking windows
			if (ImGui::BeginDock("Game View", &mShowGame, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				// Print docking information
				ShowGameViewport(&mShowGame);
			}
			ImGui::EndDock();
		};

		auto showGraphicsViewportFunc = [&]()
		{
			// Docking windows
			if (ImGui::BeginDock("Graphics", &mShowGraphicsOptionsWindow))
			{
				// Print docking information
				ShowGraphicsWindow(&mShowGraphicsOptionsWindow);
			}
			ImGui::EndDock();
		};

	 	mShowStyles = true;
	 	auto showStylesWindowFunc = [&]()
	 	{
			if (ImGui::BeginDock("Styles##options", &mShowStyles))
			{
				ImGui::ShowStyleEditor();	
			}
			ImGui::EndDock();
	 	};

		// ImGuiManager::Register(graphicsMenuOption);
		// TODO(John): I HATE the way this looks
		ImGuiManager::RegisterMenuOption("View", graphicsMenuOption);
		ImGuiManager::RegisterWindow(showGameViewportFunc);
		ImGuiManager::RegisterWindow(showGraphicsViewportFunc);
		ImGuiManager::RegisterWindow(showStylesWindowFunc);

		// Set current render texture
		mCurrentRenderTexture = mFXAATarget->GetTexture();

		// Register docking layouts
	    ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout("Game View", nullptr, ImGui::DockSlotType::Slot_Top, 1.0f));
	    ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout("Graphics", nullptr, ImGui::DockSlotType::Slot_Right, 0.1f));
	    ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout("Styles##options", nullptr, ImGui::DockSlotType::Slot_Bottom, 0.2f));
	
		// Register shader graph templates
		Enjon::ShaderGraph::DeserializeTemplate( Enjon::Engine::GetInstance( )->GetConfig( ).GetEngineResourcePath( ) + "/Shaders/ShaderGraphTemplates/ShaderTemplates.json" );

		// TODO(): I don't like random raw gl calls just lying around...
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		glEnable( GL_DEPTH_CLAMP );
		glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );

		return Result::SUCCESS;
	}

	void DeferredRenderer::BindShader( const Enjon::Shader* shader )
	{
		if ( shader != mActiveShader )
		{
			mActiveShader = const_cast< Shader*> ( shader );
			mActiveShader->Use( );
		}
	}

	//======================================================================================================

	void DeferredRenderer::STBTest( ) 
	{
		glDisable( GL_CULL_FACE );

		Enjon::String rootPath = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( );
		Enjon::String hdrFilePath = "/Textures/HDR/GCanyon_C_YumaPoint_3k.hdr";
		//Enjon::String hdrFilePath = rootPath + "/IsoARPG/Assets/Textures/HDR/03-Ueno-Shrine_3k.hdr";
		//Enjon::String hdrFilePath = "/Textures/HDR/03-Ueno-Shrine_3k.hdr";
		//Enjon::String hdrFilePath = rootPath + "/IsoARPG/Assets/Textures/HDR/Newport_Loft_Ref.hdr";
		//Enjon::String hdrFilePath = rootPath + "/IsoARPG/Assets/Textures/HDR/Factory_Catwalk_2k.hdr";
		//Enjon::String hdrFilePath = rootPath + "/IsoARPG/Assets/Textures/HDR/WinterForest_Ref.hdr";
		//Enjon::String hdrFilePath = rootPath + "/IsoARPG/Assets/Textures/HDR/Alexs_Apt_2k.hdr";
		//Enjon::String hdrFilePath = rootPath + "/IsoARPG/Assets/Textures/HDR/Mono_Lake_B_Ref.hdr";
		//Enjon::String hdrFilePath = rootPath + "/IsoARPG/Assets/Textures/HDR/Mans_Outside_2k.hdr";

		AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( );
		am->AddToDatabase( hdrFilePath ); 
		Enjon::String qualifiedName = Enjon::Utils::ToLower( am->GetName() ) + AssetLoader::GetQualifiedName( hdrFilePath );

		Enjon::AssetHandle< Enjon::Texture > hdrEnv = am->GetAsset< Enjon::Texture >( qualifiedName ); 

		{ 
			// Generate cubemap FBO, RBO
			glGenFramebuffers( 1, &mCaptureFBO );
			glGenFramebuffers( 1, &mCaptureRBO );

			glBindFramebuffer( GL_FRAMEBUFFER, mCaptureFBO );
			glBindRenderbuffer( GL_RENDERBUFFER, mCaptureRBO );
			const u32 envMapSize = 1024;
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 1024, 1024 );
			glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mCaptureRBO );

			// Cube map
			glGenTextures( 1, &mEnvCubemapID );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mEnvCubemapID );
			for ( u32 i = 0; i < 6; ++i )
			{
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, envMapSize, envMapSize, 0, GL_RGB, GL_FLOAT, nullptr );
			}
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
 
			// Capture onto cubemap faces
			Mat4 captureProj = Mat4::Perspective( 90.0f, 1.0f, 0.1f, 10.0f );
			Mat4 captureViews[ ] =  
			{
				Mat4::LookAt( Vec3(0.0f), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 0.0f, -1.0f, 0.0f ) ),
				Mat4::LookAt( Vec3(0.0f), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, -1.0f, 0.0f ) ),
				Mat4::LookAt( Vec3(0.0f), Vec3( 0.0f, 1.0f, 0.0f ), Vec3( 0.0f, 0.0f, 1.0f ) ),
				Mat4::LookAt( Vec3(0.0f), Vec3( 0.0f, -1.0f, 0.0f ), Vec3( 0.0f, 0.0f, -1.0f ) ),
				Mat4::LookAt( Vec3(0.0f), Vec3( 0.0f, 0.0f, 1.0f ), Vec3( 0.0f, -1.0f, 0.0f ) ),
				Mat4::LookAt( Vec3(0.0f), Vec3( 0.0f, 0.0f, -1.0f ), Vec3( 0.0f, -1.0f, 0.0f ) )
			}; 

			glBindFramebuffer( GL_FRAMEBUFFER, mCaptureFBO ); 
			GLSLProgram* equiShader = ShaderManager::Get( "EquiToCube" );
			equiShader->Use( );
			{
				equiShader->BindTexture( "equiMap", hdrEnv.Get()->GetTextureId( ), 0 );
				equiShader->SetUniform( "projection", captureProj );
				glViewport( 0, 0, envMapSize, envMapSize );
				for ( u32 i = 0; i < 6; ++i )
				{
					equiShader->SetUniform( "view", captureViews[ i ] );
					glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mEnvCubemapID, 0 );
					glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

					RenderCube( ); 
				}
				glBindFramebuffer( GL_FRAMEBUFFER, 0 ); 
			}
			equiShader->Unuse( );

			glBindTexture( GL_TEXTURE_CUBE_MAP, mEnvCubemapID );
			glGenerateMipmap( GL_TEXTURE_CUBE_MAP );

			// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
			// --------------------------------------------------------------------------------
			glGenTextures( 1, &mIrradianceMap );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mIrradianceMap );
			for ( unsigned int i = 0; i < 6; ++i )
			{
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr );
			}
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glBindFramebuffer( GL_FRAMEBUFFER, mCaptureFBO );
			glBindRenderbuffer( GL_RENDERBUFFER, mCaptureRBO );
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32 );

			// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
			// -----------------------------------------------------------------------------
			GLSLProgram* irradianceShader = ShaderManager::Get( "IrradianceCapture" );
			irradianceShader->Use( );
			{
				irradianceShader->SetUniform( "projection", captureProj );
				irradianceShader->SetUniform( "envMap", 0 );
				glActiveTexture( GL_TEXTURE0 );
				glBindTexture( GL_TEXTURE_CUBE_MAP, mEnvCubemapID );

				glViewport( 0, 0, 32, 32 ); // don't forget to configure the viewport to the capture dimensions.
				glBindFramebuffer( GL_FRAMEBUFFER, mCaptureFBO );
				for ( unsigned int i = 0; i < 6; ++i )
				{
					irradianceShader->SetUniform( "view", captureViews[ i ] );
					glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mIrradianceMap, 0 );
					glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

					RenderCube( );
				}
				glBindFramebuffer( GL_FRAMEBUFFER, 0 );
			}
			irradianceShader->Unuse( );

			glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

			// pbr: create an prefiltered convolution cubemap.
			// --------------------------------------------------------------------------------
			glGenTextures( 1, &mPrefilteredMap );
		//stbi_set_flip_vertically_on_load( true );
		//s32 width, height, nComps;
		//f32* data = stbi_loadf( hdrFilePath.c_str( ), &width, &height, &nComps, 0 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mPrefilteredMap );
			const u32 textureSize = 256;
			for ( u32 i = 0; i < 6; ++i )
			{
				glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, textureSize, textureSize, 0, GL_RGB, GL_FLOAT, nullptr );
			}
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glGenerateMipmap( GL_TEXTURE_CUBE_MAP );

			// -----------------------------------------------------------------------------
			GLSLProgram* prefilterShader = ShaderManager::Get( "PrefilterConvolution" );
			prefilterShader->Use( );
			{
				prefilterShader->SetUniform( "projection", captureProj );
				prefilterShader->SetUniform( "envMap", 0 );
				glActiveTexture( GL_TEXTURE0 );
				glBindTexture( GL_TEXTURE_CUBE_MAP, mEnvCubemapID );

				glBindFramebuffer( GL_FRAMEBUFFER, mCaptureFBO );

				const u32 maxMipLevels = 5;
				for ( u32 mip = 0; mip < maxMipLevels; ++mip )
				{
					u32 mipWidth = u32( ( f32 )textureSize * std::pow( 0.5, mip ) );
					u32 mipHeight = u32( ( f32 )textureSize * std::pow( 0.5, mip ) );
					f32 roughness = ( f32 )mip / ( f32 )( maxMipLevels - 1 );
					prefilterShader->SetUniform( "roughness", roughness );

					glViewport( 0, 0, mipWidth, mipHeight ); // don't forget to configure the viewport to the capture dimensions.
					glBindRenderbuffer( GL_RENDERBUFFER, mCaptureRBO );
					glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight );
					for ( unsigned int i = 0; i < 6; ++i )
					{
						prefilterShader->SetUniform( "view", captureViews[ i ] );
						glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mPrefilteredMap, mip );
						glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

						RenderCube( );
					} 
				} 
				glBindFramebuffer( GL_FRAMEBUFFER, 0 );
			}
			prefilterShader->Unuse( );

			//=======================================================================================
			// BRDF LUT generation
			glGenTextures( 1, &mBRDFLUT );

			// pre-allocate enough memory for the LUT texture.
			glBindTexture( GL_TEXTURE_2D, mBRDFLUT );
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RGB, GL_FLOAT, 0 );
			// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
			glBindFramebuffer( GL_FRAMEBUFFER, mCaptureFBO );
			glBindRenderbuffer( GL_RENDERBUFFER, mCaptureRBO );
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512 );
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mBRDFLUT, 0 );
 
			glViewport( 0, 0, 512, 512 );
			GLSLProgram* brdfShader = ShaderManager::Get( "BRDFLUT" );
			brdfShader->Use( );
			{
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
				RenderQuad( ); 
				mBatch->Begin( );
				{
					mBatch->Add(
						Vec4( -1, -1, 2, 2 ),
						Vec4( 0, 0, 1, 1 ),
						0
					);
				}
				mBatch->End( );
				mBatch->RenderBatch( );
			}
			brdfShader->Unuse( );

			glBindFramebuffer( GL_FRAMEBUFFER, 0 );

			// Get brdf texture
			Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
			mBRDFHandle = am->GetAsset< Enjon::Texture >( "isoarpg.textures.brdf" ); 
			brdfset = true;
		}
	
		// Shader graph creation
		{
			Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
			Enjon::String shaderPath = am->GetAssetsPath( ) + "/Shaders"; 
			mShaderGraph.Create( shaderPath + "/ShaderGraphs/test.json" ); 

			// Create material
			mMaterial = new Enjon::Material( &mShaderGraph );

			for ( u32 i = 0; i < 10; ++i ) 
			{
				for ( u32 j = 0; j < 10; ++j )
				{
					Enjon::Renderable renderable;
					
					// Set renderable material
					renderable.SetMaterial( mMaterial );
					renderable.SetMesh( am->GetAsset< Enjon::Mesh >( "isoarpg.models.teapot" ) );
					renderable.SetPosition( Enjon::Vec3( j, 0.0f, i ) + Enjon::Vec3( -40, 0, 10 ) );
					renderable.SetRotation( Enjon::Quaternion::AngleAxis( Enjon::ToRadians( 45.0f ), Enjon::Vec3::YAxis( ) ) );

					mRenderables.push_back( renderable );

				}
			}
		}
	}

	//======================================================================================================

	void DeferredRenderer::Update(const f32 dT)
	{ 
		static bool set = false;
		if ( !set )
		{
			Enjon::AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
			if ( am )
			{
				mRenderable.SetMesh( am->GetAsset< Enjon::Mesh >( "isoarpg.models.unreal_shaderball" ) ); 
				mRenderable.SetScale( Vec3( 1.0f ) );
				mRenderable.SetPosition( Vec3( 0, 10, 0 ) );
			}
			set = true;

			STBTest( );
		}

		// Get input
		Enjon::Input* input = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::Input >( );
		if ( input )
		{
			// Recompile test shader
			if ( input->IsKeyPressed( Enjon::KeyCode::R ) )
			{
				std::cout << "Recompiling...\n";
			}
		}

		// Gbuffer pass
		GBufferPass();
		// SSAO pass
		SSAOPass( );
		// Lighting pass
		LightingPass();
		// Luminance Pass
		LuminancePass();
		// Bloom pass
		BloomPass();
		// Composite Pass
		CompositePass(mLightingBuffer);
		// FXAA pass
		FXAAPass(mCompositeTarget);

		// Clear default buffer
		mWindow.Clear( );

		// ImGui pass
		if (true)
		{
			GuiPass();
		}

		else
		{

			glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
			auto program = Enjon::ShaderManager::Get("NoCameraProjection");	
			program->Use();
			{
				mBatch->Begin();
				{
					mBatch->Add(
									Vec4(-1, -1, 2, 2),
									Vec4(0, 0, 1, 1),
									mFXAATarget->GetTexture()
								);
				}
				mBatch->End();
				mBatch->RenderBatch();
			}
			program->Unuse();
		}

		mWindow.SwapBuffer();
	}

	//======================================================================================================

	void DeferredRenderer::GBufferPass()
	{
		glDepthFunc( GL_LESS );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );

		// Bind gbuffer
		mGbuffer->Bind();

		GLSLProgram* shader = Enjon::ShaderManager::Get("GBuffer");

		// Use gbuffer shader
		shader->Use();

		// Clear albedo render target buffer (default)
		glClearBufferfv(GL_COLOR, 0, mBGColor); 

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

		// mWindow.Clear(1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, RGBA16_LightGrey());

		// Get sorted renderables by material
		std::vector<Renderable*> sortedRenderables = mScene.GetRenderables();
		std::set<QuadBatch*>* sortedQuadBatches = mScene.GetQuadBatches();

		if (!sortedRenderables.empty())
		{
			// Set set shared uniform
			shader->SetUniform("u_camera", mSceneCamera.GetViewProjection());

			Material* material = nullptr;
			for (auto& renderable : sortedRenderables)
			{
				// Check for material switch
				Material* curMaterial = renderable->GetMaterial();
				assert(curMaterial != nullptr);
				if (material != curMaterial)
				{
					// Set material
					material = curMaterial;

					// Set material textures
					shader->BindTexture("u_albedoMap", material->GetTexture(Enjon::TextureSlotType::Albedo).Get()->GetTextureId(), 0);
					shader->BindTexture("u_normalMap", material->GetTexture(Enjon::TextureSlotType::Normal).Get()->GetTextureId(), 1);
					shader->BindTexture("u_emissiveMap", material->GetTexture(Enjon::TextureSlotType::Emissive).Get()->GetTextureId(), 2);
					shader->BindTexture("u_metallicMap", material->GetTexture(Enjon::TextureSlotType::Metallic).Get()->GetTextureId(), 3);
					shader->BindTexture("u_roughnessMap", material->GetTexture(Enjon::TextureSlotType::Roughness).Get()->GetTextureId(), 4);
					shader->BindTexture("u_aoMap", material->GetTexture(Enjon::TextureSlotType::AO).Get()->GetTextureId(), 5);
				}

				// Render mesh ( Could make this all within one call to renderable, which submits mesh and material information )
				renderable->Submit( shader ); 
			}
		}

		// Unuse gbuffer shader
		shader->Unuse();

		/////////////////////////////////////////////////
		// SHADER GRAPH TEST ////////////////////////////
		///////////////////////////////////////////////// 

		// Do shader graph test here
		static float wt = 0.0f;
		wt += 0.001f;
		if ( wt >= std::numeric_limits<f32>::max( ) )
		{
			wt = 0.0f;
		}

		Enjon::ShaderGraph* sGraph = const_cast< ShaderGraph* >( mMaterial->GetShaderGraph( ) );
		Enjon::Shader* sgShader = const_cast< Shader*> ( sGraph->GetShader( ShaderPassType::StaticGeom ) );
		sgShader->Use( );
		{
			sgShader->SetUniform( "uViewProjection", mSceneCamera.GetViewProjection( ) );
			sgShader->SetUniform( "uWorldTime", wt ); 
			mMaterial->Bind( sgShader ); 

			for ( auto& r : mRenderables )
			{ 
				r.Submit( sgShader );
			}
		} 
		sgShader->Unuse( ); 
		

		// Quadbatches
		shader = Enjon::ShaderManager::Get("QuadBatch");
		shader->Use();

		if (!sortedQuadBatches->empty())
		{
			// Set shared uniform
			shader->SetUniform("u_camera", mSceneCamera.GetViewProjection());

			Material* material = nullptr;
			for (auto& quadBatch : *sortedQuadBatches)
			{
				Material* curMaterial = quadBatch->GetMaterial();
				assert(curMaterial != nullptr);
				if (material != curMaterial)
				{ 
					// Set material
					material = curMaterial;
					
					// Check whether or not to be rendered two sided
					if ( material->TwoSided( ) )
					{
						glDisable( GL_CULL_FACE );
					}
					else
					{
						glEnable( GL_CULL_FACE );
						glCullFace( GL_BACK );
					}

					// Set material tetxures
					shader->BindTexture("u_albedoMap", material->GetTexture(Enjon::TextureSlotType::Albedo).Get()->GetTextureId(), 0);
					shader->BindTexture("u_normalMap", material->GetTexture(Enjon::TextureSlotType::Normal).Get()->GetTextureId(), 1);
					shader->BindTexture("u_emissiveMap", material->GetTexture(Enjon::TextureSlotType::Emissive).Get()->GetTextureId(), 2);
					shader->BindTexture("u_metallicMap", material->GetTexture(Enjon::TextureSlotType::Metallic).Get()->GetTextureId(), 3);
					shader->BindTexture("u_roughnessMap", material->GetTexture(Enjon::TextureSlotType::Roughness).Get()->GetTextureId(), 4);
					shader->BindTexture("u_aoMap", material->GetTexture(Enjon::TextureSlotType::AO).Get()->GetTextureId(), 5);
				}

				// Render batch
				quadBatch->RenderBatch();
			}
		}

		// Unuse quadbatch shader
		shader->Unuse();

		// Cubemap
		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_LEQUAL );
		//glCullFace( GL_FRONT );
		Enjon::GLSLProgram* skyBoxShader = Enjon::ShaderManager::Get( "SkyBox" );
		skyBoxShader->Use( );
		{
			skyBoxShader->SetUniform( "view", mSceneCamera.GetView( ) );
			skyBoxShader->SetUniform( "projection", mSceneCamera.GetProjection( ) );
			skyBoxShader->BindTexture( "environmentMap", mEnvCubemapID, 0 );

			// TODO: When setting BindTexture on shader, have to set what the texture type is ( Texture2D, SamplerCube, etc. )
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mEnvCubemapID );

			RenderCube( );
		}
		skyBoxShader->Unuse( );

		// Unbind gbuffer
		mGbuffer->Unbind();
	}

	//======================================================================================================

	void DeferredRenderer::SSAOPass( )
	{
		Enjon::iVec2 screenRes = GetViewport( ); 

		// SSAO pass
		mSSAOTarget->Bind( );
		{
			glClear( GL_COLOR_BUFFER_BIT );

			GLSLProgram* shader = ShaderManager::Get( "SSAO" );
			shader->Use( );
			{ 
				// Upload kernel uniform
				glUniform3fv( glGetUniformLocation( shader->GetProgramID( ), "samples" ), 16 * 3, ( f32* )&mSSAOKernel[ 0 ] );
				shader->SetUniform( "projection", mSceneCamera.GetProjection( ) );
				shader->SetUniform( "view", mSceneCamera.GetView( ) );
				shader->SetUniform( "uScreenResolution", Vec2( screenRes.x, screenRes.y ) );
				shader->SetUniform( "radius", mSSAORadius );
				shader->SetUniform( "bias", mSSAOBias );
				shader->SetUniform( "uIntensity", mSSAOIntensity );
				shader->SetUniform( "near", mSceneCamera.GetNear() );
				shader->SetUniform( "far", mSceneCamera.GetFar() );
				shader->BindTexture( "gPosition", mGbuffer->GetTexture( GBufferTextureType::POSITION ), 0 );
				shader->BindTexture( "gNormal", mGbuffer->GetTexture( GBufferTextureType::NORMAL ), 1 );
				shader->BindTexture( "texNoise", mSSAONoiseTexture, 2 ); 
				shader->BindTexture( "uDepthMap", mGbuffer->GetDepth( ), 3 ); 
				RenderQuad( );
			}
			shader->Unuse( ); 
		}
		mSSAOTarget->Unbind( );

		// Blur SSAO to remove noise
		mSSAOBlurTarget->Bind( );
		{
			glClear( GL_COLOR_BUFFER_BIT );

			GLSLProgram* shader = ShaderManager::Get( "SSAOBlur" );
			shader->Use( );
			{
				shader->BindTexture( "ssaoInput", mSSAOTarget->GetTexture( ), 0 );
				RenderQuad( ); 
			}
			shader->Unuse( ); 
		}
		mSSAOBlurTarget->Unbind( );
	}

	//======================================================================================================

	void DeferredRenderer::LightingPass()
	{
		mLightingBuffer->Bind();
		// mFullScreenQuad->Bind();
		
		GLSLProgram* ambientShader 		= Enjon::ShaderManager::Get("AmbientLight");
		GLSLProgram* directionalShader 	= Enjon::ShaderManager::Get("PBRDirectionalLight");	
		GLSLProgram* pointShader 		= Enjon::ShaderManager::Get("PBRPointLight");	
		GLSLProgram* spotShader 		= Enjon::ShaderManager::Get("SpotLight");	

		std::set<DirectionalLight*>* directionalLights 	= mScene.GetDirectionalLights();	
		std::set<SpotLight*>* spotLights 				= mScene.GetSpotLights();	
		std::set<PointLight*>* pointLights 				= mScene.GetPointLights();

		AmbientSettings* aS = mScene.GetAmbientSettings();

		mWindow.Clear();

		// TODO(): Abstract these away 
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE); 

		// Ambient pass
		ambientShader->Use();
		{ 
			ambientShader->SetUniform( "uIrradianceMap", 0 );
			ambientShader->SetUniform( "uPrefilterMap", 1 );
			ambientShader->SetUniform( "uBRDFLUT", 2 );
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mIrradianceMap );
			glActiveTexture( GL_TEXTURE1 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mPrefilteredMap );

			if ( useOther )
			{
				glActiveTexture( GL_TEXTURE2 );
				glBindTexture( GL_TEXTURE_2D, mBRDFLUT ); 
			}
			else
			{ 
				ambientShader->BindTexture( "uBRDFLUT", mBRDFHandle.Get( )->GetTextureId( ), 2 );
			}
			ambientShader->BindTexture("uAlbedoMap", mGbuffer->GetTexture(GBufferTextureType::ALBEDO), 3);
			ambientShader->BindTexture("uNormalMap", mGbuffer->GetTexture(GBufferTextureType::NORMAL), 4);
			ambientShader->BindTexture("uPositionMap", mGbuffer->GetTexture(GBufferTextureType::POSITION), 5);
			ambientShader->BindTexture("uEmissiveMap", mGbuffer->GetTexture(GBufferTextureType::EMISSIVE), 6);
			ambientShader->BindTexture("uMaterialMap", mGbuffer->GetTexture(GBufferTextureType::MAT_PROPS), 7);
			ambientShader->BindTexture("uSSAOMap", mSSAOBlurTarget->GetTexture(), 8);
			ambientShader->BindTexture("uDepthMap", mSSAOBlurTarget->GetTexture(), 9);
			ambientShader->SetUniform("uResolution", mGbuffer->GetResolution());
			ambientShader->SetUniform( "uCamPos", mSceneCamera.GetPosition() );

			// Render
				mBatch->Begin();
					mBatch->Add(
									Vec4(-1, -1, 2, 2),
									Vec4(0, 0, 1, 1),
									mGbuffer->GetTexture(GBufferTextureType::ALBEDO)
								);
				mBatch->End();
				mBatch->RenderBatch();
		}
		ambientShader->Unuse();

		directionalShader->Use();
		{
			directionalShader->SetUniform( "u_camPos", mSceneCamera.GetPosition( ) );
			for (auto& l : *directionalLights)
			{
				ColorRGBA16 color = l->GetColor();

				directionalShader->BindTexture("u_albedoMap", 	mGbuffer->GetTexture(GBufferTextureType::ALBEDO), 0);
				directionalShader->BindTexture("u_normalMap", 	mGbuffer->GetTexture(GBufferTextureType::NORMAL), 1);
				directionalShader->BindTexture("u_positionMap", mGbuffer->GetTexture(GBufferTextureType::POSITION), 2);
				directionalShader->BindTexture("u_matProps", 	mGbuffer->GetTexture(GBufferTextureType::MAT_PROPS), 3);
				directionalShader->BindTexture("u_ssao", 		mSSAOBlurTarget->GetTexture(), 4);
				// directionalShader->BindTexture("u_shadowMap", 		mShadowDepth->GetDepth(), 4);
				directionalShader->SetUniform("u_resolution", 		mGbuffer->GetResolution());
				// directionalShader->SetUniform("u_lightSpaceMatrix", mShadowCamera->GetViewProjectionMatrix());
				// directionalShader->SetUniform("u_shadowBias", 		EM::Vec2(0.005f, ShadowBiasMax));
				directionalShader->SetUniform("u_lightDirection", 	l->GetDirection());															
				directionalShader->SetUniform("u_lightColor", 		Vec3(color.r, color.g, color.b));
				directionalShader->SetUniform("u_lightIntensity", 	l->GetIntensity());

				// Render	
				// TODO(): Fix full screen quad - super janky
				// mFullScreenQuad->Submit();
				mBatch->Begin();
					mBatch->Add(
									Vec4(-1, -1, 2, 2),
									Vec4(0, 0, 1, 1),
									mGbuffer->GetTexture(GBufferTextureType::ALBEDO)
								);
				mBatch->End();
				mBatch->RenderBatch();
			}
		}
		directionalShader->Unuse();

		pointShader->Use();
		{
			pointShader->BindTexture( "u_albedoMap", mGbuffer->GetTexture( GBufferTextureType::ALBEDO ), 0 );
			pointShader->BindTexture( "u_normalMap", mGbuffer->GetTexture( GBufferTextureType::NORMAL ), 1 );
			pointShader->BindTexture( "u_positionMap", mGbuffer->GetTexture( GBufferTextureType::POSITION ), 2 );
			pointShader->BindTexture( "u_matProps", mGbuffer->GetTexture( GBufferTextureType::MAT_PROPS ), 3 );
			pointShader->BindTexture( "u_ssao", mSSAOBlurTarget->GetTexture( ), 4 );
			pointShader->SetUniform( "u_resolution", mGbuffer->GetResolution( ) );
			pointShader->SetUniform( "u_camPos", mSceneCamera.GetPosition( ) );

			for (auto& l : *pointLights)
			{
				ColorRGBA16& color = l->GetColor();
				Vec3& position = l->GetPosition();

				pointShader->SetUniform("u_lightPos", position);
				pointShader->SetUniform("u_lightColor", Vec3(color.r, color.g, color.b));
				pointShader->SetUniform("u_lightIntensity", l->GetIntensity());
				pointShader->SetUniform("u_attenuationRate", l->GetAttenuationRate());
				pointShader->SetUniform("u_radius", l->GetRadius());

				// Render Light to screen
				mBatch->Begin();
					mBatch->Add(
									Vec4(-1, -1, 2, 2),
									Vec4(0, 0, 1, 1),
									mGbuffer->GetTexture(GBufferTextureType::ALBEDO)
								);
				mBatch->End();
				mBatch->RenderBatch();
			}
		}
		pointShader->Unuse();

		spotShader->Use();
		{
			for (auto& l : *spotLights)
			{
				spotShader->BindTexture("u_albedoMap", mGbuffer->GetTexture(GBufferTextureType::ALBEDO), 0);
				spotShader->BindTexture("u_normalMap", mGbuffer->GetTexture(GBufferTextureType::NORMAL), 1);
				spotShader->BindTexture("u_positionMap", mGbuffer->GetTexture(GBufferTextureType::POSITION), 2);
				// spotShader->BindTexture("u_matProps", mGbuffer->GetTexture(GBufferTextureType::MAT_PROPS), 3);
				spotShader->SetUniform("u_resolution", mGbuffer->GetResolution());
				spotShader->SetUniform("u_camPos", mSceneCamera.GetPosition());			

				for (auto& l : *spotLights)
				{
					ColorRGBA16& color = l->GetColor();
					SLParams& params = l->GetParams();
					Vec3& position = l->GetPosition();

					spotShader->SetUniform("u_lightPos", position);
					spotShader->SetUniform("u_lightColor", Vec3(color.r, color.g, color.b));
					spotShader->SetUniform("u_falloff", params.mFalloff);
					spotShader->SetUniform("u_lightIntensity", l->GetIntensity());
					spotShader->SetUniform("u_lightDirection", params.mDirection);
					spotShader->SetUniform("u_innerCutoff", params.mInnerCutoff);
					spotShader->SetUniform("u_outerCutoff", params.mOuterCutoff);

					// Render Light to screen
					mBatch->Begin();
						mBatch->Add(
										Vec4(-1, -1, 2, 2),
										Vec4(0, 0, 1, 1),
										mGbuffer->GetTexture(GBufferTextureType::ALBEDO)
									);
					mBatch->End();
					mBatch->RenderBatch();
				}
			}
		}
		spotShader->Unuse();

		// mFullScreenQuad->Unbind();
		mLightingBuffer->Unbind();	

		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	//======================================================================================================

	void DeferredRenderer::LuminancePass()
	{
		GLSLProgram* luminanceProgram = Enjon::ShaderManager::Get("Bright");
		mLuminanceTarget->Bind();
		{
			mWindow.Clear(1.0f, GL_COLOR_BUFFER_BIT, RGBA16_Black());
			luminanceProgram->Use();
			{
				luminanceProgram->BindTexture("u_emissiveMap", mGbuffer->GetTexture(GBufferTextureType::EMISSIVE), 1);
				luminanceProgram->SetUniform("u_threshold", mToneMapSettings.mThreshold);
				mBatch->Begin();
				{
					mBatch->Add(
										Vec4(-1, -1, 2, 2),
										Vec4(0, 0, 1, 1),
										mLightingBuffer->GetTexture()
									);
				}
				mBatch->End();
				mBatch->RenderBatch();
			}
			luminanceProgram->Unuse();
		}
		mLuminanceTarget->Unbind();
	}

	//======================================================================================================

	void DeferredRenderer::BloomPass()
	{
		GLSLProgram* horizontalBlurProgram = Enjon::ShaderManager::Get("HorizontalBlur");
		GLSLProgram* verticalBlurProgram = Enjon::ShaderManager::Get("VerticalBlur");

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE);

		// Small blur
    	for (u32 i = 0; i < (u32)mBloomSettings.mIterations.x * 2; ++i)
    	{
    		bool isEven = (i % 2 == 0);
    		RenderTarget* target = isEven ? mSmallBlurHorizontal : mSmallBlurVertical;
    		GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind(RenderTarget::BindType::WRITE);
			{
				program->Use();
				{
					for (uint32_t j = 0; j < 16; j++)
					{
						std::string uniform = "u_blurWeights[" + std::to_string(j) + "]";
						program->SetUniform(uniform, mBloomSettings.mSmallGaussianCurve[j]);
					}

					program->SetUniform("u_weight", mBloomSettings.mWeights.x);
					program->SetUniform("u_blurRadius", mBloomSettings.mRadius.x);
					GLuint texID = i == 0 ? mLuminanceTarget->GetTexture() : isEven ? mSmallBlurVertical->GetTexture() : mSmallBlurHorizontal->GetTexture();
					mBatch->Begin();
					{
			    		mBatch->Add(
									Vec4(-1, -1, 2, 2),
									Vec4(0, 0, 1, 1), 
									texID
								);
					}
					mBatch->End();
					mBatch->RenderBatch();
				}
				program->Unuse();
			}	
			target->Unbind();
    	}

		// Medium blur
    	for (u32 i = 0; i < (u32)mBloomSettings.mIterations.y * 2; ++i)
    	{
    		bool isEven = (i % 2 == 0);
    		RenderTarget* target = isEven ? mMediumBlurHorizontal : mMediumBlurVertical;
    		GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind(RenderTarget::BindType::WRITE);
			{
				program->Use();
				{
					for (uint32_t j = 0; j < 16; j++)
					{
						std::string uniform = "u_blurWeights[" + std::to_string(j) + "]";
						program->SetUniform(uniform, mBloomSettings.mMediumGaussianCurve[j]);
					}

					program->SetUniform("u_weight", mBloomSettings.mWeights.y);
					program->SetUniform("u_blurRadius", mBloomSettings.mRadius.y);
					GLuint texID = i == 0 ? mSmallBlurVertical->GetTexture() : isEven ? mMediumBlurVertical->GetTexture() : mMediumBlurHorizontal->GetTexture();
					//GLuint texID = i == 0 ? mLuminanceTarget->GetTexture() : isEven ? mMediumBlurVertical->GetTexture() : mMediumBlurHorizontal->GetTexture();
					mBatch->Begin();
					{
			    		mBatch->Add(
									Vec4(-1, -1, 2, 2),
									Vec4(0, 0, 1, 1), 
									texID
								);
					}
					mBatch->End();
					mBatch->RenderBatch();
				}
				program->Unuse();
			}	
			target->Unbind();
    	}

		// Large blur
    	for (u32 i = 0; i < (u32)mBloomSettings.mIterations.z * 2; ++i)
    	{
    		bool isEven = (i % 2 == 0);
    		RenderTarget* target = isEven ? mLargeBlurHorizontal : mLargeBlurVertical;
    		GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind(RenderTarget::BindType::WRITE);
			{
				program->Use();
				{
					for (uint32_t j = 0; j < 16; j++)
					{
						std::string uniform = "u_blurWeights[" + std::to_string(j) + "]";
						program->SetUniform(uniform, mBloomSettings.mLargeGaussianCurve[j]);
					}

					program->SetUniform("u_weight", mBloomSettings.mWeights.z);
					program->SetUniform("u_blurRadius", mBloomSettings.mRadius.z);
					GLuint texID = i == 0 ? mMediumBlurVertical->GetTexture() : isEven ? mLargeBlurVertical->GetTexture() : mLargeBlurHorizontal->GetTexture();
					//GLuint texID = i == 0 ? mLuminanceTarget->GetTexture() : isEven ? mLargeBlurVertical->GetTexture() : mLargeBlurHorizontal->GetTexture();
					mBatch->Begin();
					{
			    		mBatch->Add(
									Vec4(-1, -1, 2, 2),
									Vec4(0, 0, 1, 1), 
									texID
								);
					}
					mBatch->End();
					mBatch->RenderBatch();
				}
				program->Unuse();
			}	
			target->Unbind();
    	}

		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	//======================================================================================================

	void DeferredRenderer::BloomPass2( )
	{
		GLSLProgram* horizontalBlurProgram = Enjon::ShaderManager::Get( "HorizontalBlur" );
		GLSLProgram* verticalBlurProgram = Enjon::ShaderManager::Get( "VerticalBlur" );

		glEnable( GL_BLEND );
		glDisable( GL_DEPTH_TEST );
		glBlendFunc( GL_ONE, GL_ONE ); 
	}

	//======================================================================================================

	void DeferredRenderer::FXAAPass(RenderTarget* input)
	{
		GLSLProgram* fxaaProgram = Enjon::ShaderManager::Get("FXAA");
		mFXAATarget->Bind();
		{
			mWindow.Clear();
			fxaaProgram->Use();
			{
				auto viewPort = GetViewport();
				fxaaProgram->SetUniform("u_resolution", Vec2(viewPort.x, viewPort.y));
				fxaaProgram->SetUniform("u_FXAASettings", Vec3(mFXAASettings.mSpanMax, mFXAASettings.mReduceMul, mFXAASettings.mReduceMin));
				mBatch->Begin();
				{
					mBatch->Add(
										Vec4(-1, -1, 2, 2),
										Vec4(0, 0, 1, 1),
										input->GetTexture()
									);
				}
				mBatch->End();
				mBatch->RenderBatch();
			}
			fxaaProgram->Unuse();
		}
		mFXAATarget->Unbind();
	}

	//======================================================================================================

	void DeferredRenderer::CompositePass(RenderTarget* input)
	{
		GLSLProgram* compositeProgram = Enjon::ShaderManager::Get("Composite");
		mCompositeTarget->Bind();
		{
			mWindow.Clear();
			compositeProgram->Use();
			{
				compositeProgram->BindTexture("u_blurTexSmall", mSmallBlurVertical->GetTexture(), 1);
				compositeProgram->BindTexture("u_blurTexMedium", mMediumBlurVertical->GetTexture(), 2);
				compositeProgram->BindTexture("u_blurTexLarge", mLargeBlurVertical->GetTexture(), 3);
				compositeProgram->SetUniform("u_exposure", mToneMapSettings.mExposure);
				compositeProgram->SetUniform("u_gamma", mToneMapSettings.mGamma);
				compositeProgram->SetUniform("u_bloomScalar", mToneMapSettings.mBloomScalar);
				compositeProgram->SetUniform("u_saturation", mToneMapSettings.mSaturation);
				mBatch->Begin();
				{
					mBatch->Add(
										Vec4(-1, -1, 2, 2),
										Vec4(0, 0, 1, 1),
										input->GetTexture()
									);
				}
				mBatch->End();
				mBatch->RenderBatch();
			}
			compositeProgram->Unuse();
		}
		mCompositeTarget->Unbind();
	}

	//======================================================================================================

	void DeferredRenderer::GuiPass()
	{
		static bool show_test_window = false;
		static bool show_frame_rate = false;
		static bool show_graphics_window = true;
		static bool show_app_layout = false;
		static bool show_game_viewport = true;

        // Queue up gui
        ImGuiManager::Render(mWindow.GetSDLWindow());
		 //ImGuiManager::RenderGameUI(&mWindow, mSceneCamera.GetView().elements, mSceneCamera.GetProjection().elements);

        // Flush
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        ImGui::Render(); 
	}

	//======================================================================================================

	void DeferredRenderer::SetViewport(iVec2& dimensions)
	{
		mWindow.SetViewport(dimensions);
	}

	//======================================================================================================

	iVec2 DeferredRenderer::GetViewport()
	{
		return mWindow.GetViewport();
	}

	//======================================================================================================

	f64 NormalPDF(const f64 x, const f64 s, f64 m = 0.0)
	{
		static const f64 inv_sqrt_2pi = 0.3989422804014327;
		f64 a = (x - m) / s;

		return inv_sqrt_2pi / s * std::exp(-0.5 * a * a);
	}

	//======================================================================================================

	void DeferredRenderer::InitializeFrameBuffers()
	{
		auto viewport = mWindow.GetViewport();
		Enjon::u32 width = (Enjon::u32)viewport.x;
		Enjon::u32 height = (Enjon::u32)viewport.y;

		mGbuffer 					= new GBuffer(width, height);
		mDebugTarget 				= new RenderTarget(width, height);
		mSmallBlurHorizontal 		= new RenderTarget(width / 4, height / 4);
		mSmallBlurVertical 			= new RenderTarget(width / 4, height / 4);
		mMediumBlurHorizontal 		= new RenderTarget(width  / 8, height  / 8);
		mMediumBlurVertical 		= new RenderTarget(width  / 8, height  / 8);
		mLargeBlurHorizontal 		= new RenderTarget(width / 16, height / 16);
		mLargeBlurVertical 			= new RenderTarget(width / 16, height / 16);
		mCompositeTarget 			= new RenderTarget(width, height);
		mLightingBuffer 			= new RenderTarget(width, height);
		mLuminanceTarget 			= new RenderTarget(width / 2, height / 2);
		mFXAATarget 				= new RenderTarget(width, height);
		mShadowDepth 				= new RenderTarget(2048, 2048);
		mFinalTarget 				= new RenderTarget(width, height);
		mSSAOTarget					= new RenderTarget( width, height );
		mSSAOBlurTarget				= new RenderTarget( width / 2, height / 2 );

		mBatch 						= new SpriteBatch();
		mBatch->Init();

		mFullScreenQuad 			= new FullScreenQuad();

		// Generate sample kernel
		std::uniform_real_distribution< f32 > randomFloats( 0.0f, 1.0f );
		std::default_random_engine generator;
		for ( u32 i = 0; i < 16; ++i )
		{
			Enjon::Vec3 sample( randomFloats( generator ) * 2.0f - 1.0f, randomFloats( generator ) * 2.0f - 1.0f, randomFloats( generator ) );
			sample *= randomFloats( generator );
			f32 scale = f32( i ) / 64.0;

			// scale samples s.t. they're more aligned to center of kernel
			scale = Enjon::Lerp( 0.1f, 1.0f, scale * scale );
			sample *= scale;
			mSSAOKernel.push_back( sample );
		}

		// Generate noise texture
		std::vector< Enjon::Vec3 > ssaoNoise;
		for ( unsigned int i = 0; i < 256 * 256; i++ )
		{
			Enjon::Vec3 noise( randomFloats( generator ) * 2.0 - 1.0, randomFloats( generator ) * 2.0 - 1.0, 0.0f ); // rotate around z-axis (in tangent space)
			ssaoNoise.push_back( noise );
		}

		glGenTextures( 1, &mSSAONoiseTexture );
		glBindTexture( GL_TEXTURE_2D, mSSAONoiseTexture );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB32F, 256, 256, 0, GL_RGB, GL_FLOAT, &ssaoNoise[ 0 ] );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ); 
	}

	//======================================================================================================

	void DeferredRenderer::CalculateBlurWeights()
	{
		f64 weight;
		f64 start = -3.0;
		f64 end = 3.0;
		f64 denom = 2.0 * end + 1.0;
		f64 num_samples = 15.0;
		f64 range = end * 2.0;
		f64 step = range / num_samples;
		u32 i = 0;

		weight = 1.74;
		for (f64 x = start; x <= end; x += step)
		{
			f64 pdf = NormalPDF(x, 0.23);
			mBloomSettings.mSmallGaussianCurve[i++] = pdf;
		}

		i = 0;
		weight = 3.9f;
		for (f64 x = start; x <= end; x += step)
		{
			f64 pdf = NormalPDF(x, 0.775);
			mBloomSettings.mMediumGaussianCurve[i++]= pdf;
		}

		i = 0;
		weight = 2.53f;
		for (f64 x = start; x <= end; x += step)
		{
			f64 pdf = NormalPDF(x, 1.0);
			mBloomSettings.mLargeGaussianCurve[i++] = pdf;
		}
	}

	//======================================================================================================

	void DeferredRenderer::RegisterCVars()
	{
		Enjon::CVarsSystem::Register("exposure", &mToneMapSettings.mExposure, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("gamma", &mToneMapSettings.mGamma, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("bloomScale", &mToneMapSettings.mBloomScalar, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("saturation", &mToneMapSettings.mSaturation, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_weight_small", &mBloomSettings.mWeights.x, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_weight_medium", &mBloomSettings.mWeights.y, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_weight_large", &mBloomSettings.mWeights.z, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_iter_small", &mBloomSettings.mIterations.x, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_iter_medium", &mBloomSettings.mIterations.y, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_iter_large", &mBloomSettings.mIterations.z, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("threshold", &mToneMapSettings.mThreshold, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_radius_small", &mBloomSettings.mRadius.x, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_radius_medium", &mBloomSettings.mRadius.y, Enjon::CVarType::TYPE_FLOAT);
		Enjon::CVarsSystem::Register("blur_radius_large", &mBloomSettings.mRadius.z, Enjon::CVarType::TYPE_FLOAT);
	}

	//======================================================================================================

	void DeferredRenderer::ShowGraphicsWindow(bool* p_open)
	{
	    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);                                 // Right align, keep 140 pixels for labels

        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(1.0, 0.6f, 0.0f, 1.0f));
        ImGui::Text("Graphics Options");
        ImGui::PopStyleColor(1);
        ImGui::Separator();

		ImGui::SliderFloat( "X", &uvs.x, 0.0f, 1.0f );
		ImGui::SliderFloat( "Y", &uvs.y, 0.0f, 1.0f );
		ImGui::SliderFloat( "Z", &uvs.z, 0.0f, 1.0f );
		ImGui::SliderFloat( "W", &uvs.w, 0.0f, 1.0f );

	    if (ImGui::TreeNode("ToneMapping"))
	    {
		    if (ImGui::TreeNode("Bloom"))
		    {
		    	if (ImGui::TreeNode("Blur"))
		    	{
				    if (ImGui::TreeNode("Small##bloom"))
				    {
				    	// Iterations
				    	s32 smallIter = (s32)mBloomSettings.mIterations.x;
					    ImGui::SliderInt("Iterations##small", &smallIter, 0, 30);
					    mBloomSettings.mIterations.x = smallIter;

					    // Radius
					    f32 radius = (f32)mBloomSettings.mRadius.x;
					    ImGui::SliderFloat("Radius##bloomsmall", &radius, 0.001f, 0.1f, "%.3f");
					    mBloomSettings.mRadius.x = radius;

					    // Weight
					    f32 weight = (f32)mBloomSettings.mWeights.x;
					    ImGui::SliderFloat("Weight##bloomsmall", &weight, 0.001, 0.5f, "%.3f");
					    mBloomSettings.mWeights.x = weight;

					    ImGui::TreePop();
				    }

				    if (ImGui::TreeNode("Medium##bloom"))
				    {
				    	// Iterations
				    	s32 medIter = (s32)mBloomSettings.mIterations.y;
					    ImGui::SliderInt("Iterations##medium", &medIter, 0, 30);
					    mBloomSettings.mIterations.y = medIter;

					    // Radius
					    f32 radius = (f32)mBloomSettings.mRadius.y;
					    ImGui::SliderFloat("Radius##bloomedium", &radius, 0.001f, 0.1f, "%.3f");
					    mBloomSettings.mRadius.y = radius;

					    // Weight
					    f32 weight = (f32)mBloomSettings.mWeights.y;
					    ImGui::SliderFloat("Weight##bloomedium", &weight, 0.001, 0.5f, "%.3f");
					    mBloomSettings.mWeights.y = weight;

					    ImGui::TreePop();
				    }

				    if (ImGui::TreeNode("Large##bloom"))
				    {
				    	// Iterations
				    	s32 largeIter = (s32)mBloomSettings.mIterations.z;
					    ImGui::SliderInt("Iterations##large", &largeIter, 0, 30);
					    mBloomSettings.mIterations.z = largeIter;

					    // Radius
					    f32 radius = (f32)mBloomSettings.mRadius.z;
					    ImGui::SliderFloat("Radius##bloomlarge", &radius, 0.001f, 0.1f, "%.3f");
					    mBloomSettings.mRadius.z = radius;

					    // Weight
					    f32 weight = (f32)mBloomSettings.mWeights.z;
					    ImGui::SliderFloat("Weight##bloomlarge", &weight, 0.001, 0.5f, "%.3f");
					    mBloomSettings.mWeights.z = weight;

					    ImGui::TreePop();
				    }

				    ImGui::TreePop();
		    	}

			    ImGui::SliderFloat("Scale##bloom", &mToneMapSettings.mBloomScalar, 0.01f, 100.0f, "%.2f");
			    ImGui::SliderFloat("Threshold##bloom", &mToneMapSettings.mThreshold, 0.00f, 100.0f, "%.2f");

			    ImGui::TreePop();
		    }

		    if (ImGui::TreeNode("Saturation"))
		    {
			    // Saturation
			    ImGui::SliderFloat("Saturation##tonemap", &mToneMapSettings.mSaturation, 0.0, 2.0f, "%.1f");

			    ImGui::TreePop();
		    }

		    if (ImGui::TreeNode("Gamma"))
		    {
			    // Gamma
			    ImGui::SliderFloat("Gamma##tonemap", &mToneMapSettings.mGamma, 0.01, 2.5f, "%.2f");

			    ImGui::TreePop();
		    }

		    if (ImGui::TreeNode("Exposure"))
		    {
			    // Exposure
			    ImGui::SliderFloat("Exposure##tonemap", &mToneMapSettings.mExposure, 0.01, 1.0f, "%.2f");

			    ImGui::TreePop();
		    }

		    ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("FXAA"))
	    {
		    ImGui::SliderFloat("SpanMax##fxaa", &mFXAASettings.mSpanMax, 0.001, 100.0f, "%.3f");
		    ImGui::SliderFloat("ReduceMul##fxaa", &mFXAASettings.mReduceMul, 0.000001, 0.001f, "%.6f");
		    ImGui::SliderFloat("ReduceMin##fxaa", &mFXAASettings.mReduceMin, 0.000001, 0.001f, "%.6f");

		    bool enabled = (bool)mFXAASettings.mEnabled;
            ImGui::Checkbox("Enabled##fxaa", &enabled);
            mFXAASettings.mEnabled = (u32)enabled;

		    ImGui::TreePop();
	    }
	    
		if (ImGui::TreeNode("SSAO"))
	    {
		    ImGui::SliderFloat("Radius##ssao", &mSSAORadius, 0.01, 1.0f, "%.3f");
		    ImGui::SliderFloat("Bias##ssao", &mSSAOBias, 1.0, 0.01f, "%.3f"); 
		    ImGui::SliderFloat("Intensity##ssao", &mSSAOIntensity, 0.0f, 5.0f, "%.2f"); 

		    ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("FrameBuffers"))
	    {
	    	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	    	ImGui::PushFont(atlas->Fonts[1]);
	        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0.2, 0.6f, 0.6f, 1.0f));
	    	for (u32 i = 0; i < (u32)GBufferTextureType::GBUFFER_TEXTURE_COUNT; ++i)
	    	{
	    		const char* string_name = mGbuffer->FrameBufferToString(i);
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mGbuffer->GetTexture(i);

                if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
                {
			        mCurrentRenderTexture = mGbuffer->GetTexture(i); 
                }
	    	}

	    	{
	    		const char* string_name = "SmallBloom";
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mSmallBlurVertical->GetTexture();
	            if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
	            {
			        mCurrentRenderTexture = mSmallBlurVertical->GetTexture(); 
	            }
	    	}

	    	{
	    		const char* string_name = "MediumBloom";
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mMediumBlurVertical->GetTexture();
	            if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
	            {
			        mCurrentRenderTexture = mMediumBlurVertical->GetTexture(); 
	            }
	    	}
	    	{
	    		const char* string_name = "LargeBloom";
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mLargeBlurVertical->GetTexture();
	            if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
	            {
			        mCurrentRenderTexture = mLargeBlurVertical->GetTexture(); 
	            }
	    	}
	    	{
	    		const char* string_name = "Bright";
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mLuminanceTarget->GetTexture();
	            if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
	            {
			        mCurrentRenderTexture = mLuminanceTarget->GetTexture(); 
	            }
	    	}
	    	{
	    		const char* string_name = "Light";
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mLightingBuffer->GetTexture();
	            if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
	            {
			        mCurrentRenderTexture = mLightingBuffer->GetTexture(); 
	            }
	    	}

			{
				const char* string_name = "BRDFLUT";
				ImGui::Text( string_name );
				ImTextureID img = ( ImTextureID )mBRDFLUT;
				if ( ImGui::ImageButton( img, ImVec2( 64, 64 ), ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1, ImVec4( 0, 0, 0, 0 ), ImColor( 255, 255, 255, 255 ) ) )
				{
					mCurrentRenderTexture = mBRDFLUT;
				}
			}
			
			{
				const char* string_name = "SSAO";
				ImGui::Text( string_name );
				ImTextureID img = ( ImTextureID )mSSAOTarget->GetTexture( );
				if ( ImGui::ImageButton( img, ImVec2( 64, 64 ), ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1, ImVec4( 0, 0, 0, 0 ), ImColor( 255, 255, 255, 255 ) ) )
				{
					mCurrentRenderTexture = mSSAOTarget->GetTexture( );
				}
			}
			
			{
				const char* string_name = "SSAOBlur";
				ImGui::Text( string_name );
				ImTextureID img = ( ImTextureID )mSSAOBlurTarget->GetTexture();
				if ( ImGui::ImageButton( img, ImVec2( 64, 64 ), ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1, ImVec4( 0, 0, 0, 0 ), ImColor( 255, 255, 255, 255 ) ) )
				{
					mCurrentRenderTexture = mSSAOBlurTarget->GetTexture();
				}
			}
			
			{
				const char* string_name = "SSAONoise";
				ImGui::Text( string_name );
				ImTextureID img = ( ImTextureID )mSSAONoiseTexture;
				if ( ImGui::ImageButton( img, ImVec2( 64, 64 ), ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1, ImVec4( 0, 0, 0, 0 ), ImColor( 255, 255, 255, 255 ) ) )
				{
					mCurrentRenderTexture = mSSAONoiseTexture;
				}
			}

	    	{
	    		const char* string_name = "Final";
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mFXAATarget->GetTexture();
	            if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
	            {
			        mCurrentRenderTexture = mFXAATarget->GetTexture(); 
	            }
	    	}

	    	ImGui::PopStyleColor(1);
	    	ImGui::PopFont();
	    	ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("Sunlight"))
	    {
	    	static const char* labels[] = {"X", "Y", "Z"};
	    	Vec3 direction = mScene.GetSun()->GetDirection();
            float vec4f[4] = { direction.x, direction.y, direction.z, 1.0f };
	    	ImGui::Text("Direction");
            ImGui::DragFloat3Labels("##sundir", labels, vec4f, 0.001f, -1.0f, 1.0f);
	    	mScene.GetSun()->SetDirection(Vec3(vec4f[0], vec4f[1], vec4f[2]));

	    	float intensity = mScene.GetSun()->GetIntensity();
	    	ImGui::DragFloat("Intensity", &intensity, 0.1f, 0.0f, 50.0f);
	    	mScene.GetSun()->SetIntensity(intensity);

	    	ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("Camera"))
	    {
	    	ImGui::DragFloat("FOV", &mSceneCamera.FOV, 0.1f, 0.0f, 100.0f);
	    	ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("Background"))
	    {
	    	static const char* labels[] = {"R", "G", "B"};
	    	ImGui::Text("Color");
            ImGui::DragFloat3Labels("##bgcolor", labels, mBGColor, 0.1f, 0.0f, 30.0f);
	    	ImGui::TreePop();
	    } 

		if ( ImGui::CollapsingHeader( "Uniforms" ) )
		{
			if ( mMaterial )
			{
				for ( auto& u : *mShaderGraph.GetUniforms( ) )
				{
					Enjon::String uniformName = u.second->GetName( );
					Enjon::UniformType type = u.second->GetType( );
					Enjon::ShaderUniform* uniform = u.second;

					if ( mMaterial->HasOverride( uniformName ) )
					{
						uniform = const_cast< ShaderUniform* > ( mMaterial->GetOverride( uniformName ) );
					}

					switch ( type )
					{
						case UniformType::Float:
						{
							UniformPrimitive< f32 >* uf = uniform->Cast< UniformPrimitive< f32 > >( );
							f32 val = uf->GetValue( );
							if ( ImGui::SliderFloat( uniformName.c_str( ), &val, 0.0f, 3.0f ) )
							{
								mMaterial->SetUniform( uniformName, val );
							}
						} break;

						case UniformType::Vec2:
						{
							UniformPrimitive< Vec2 >* uf = uniform->Cast< UniformPrimitive< Vec2 > >( );
							Enjon::Vec2 val = uf->GetValue( );
							f32 vals[ 2 ];
							vals[ 0 ] = val.x;
							vals[ 1 ] = val.y;
							if ( ImGui::SliderFloat2( uniformName.c_str( ), (f32*)&vals, 0.0f, 3.0f ) )
							{ 
								mMaterial->SetUniform( uniformName, Enjon::Vec2( vals[ 0 ], vals[ 1 ] ) );
							}
						} break;

						case UniformType::Vec3:
						{
							UniformPrimitive< Vec3 >* uf = uniform->Cast< UniformPrimitive< Vec3 > >( );
							Enjon::Vec3 val = uf->GetValue( );
							f32 vals[ 3 ];
							vals[ 0 ] = val.x;
							vals[ 1 ] = val.y;
							vals[ 2 ] = val.z;
							if ( ImGui::SliderFloat3( uniformName.c_str( ), (f32*)&vals, 0.0f, 3.0f ) )
							{ 
								mMaterial->SetUniform( uniformName, Enjon::Vec3( vals[ 0 ], vals[ 1 ], vals[ 2 ] ) );
							}
						} break;

						case UniformType::Vec4:
						{
							UniformPrimitive< Vec4 >* uf = uniform->Cast< UniformPrimitive< Vec4 > >( );
							Enjon::Vec4 val = uf->GetValue( );
							f32 vals[ 4 ];
							vals[ 0 ] = val.x;
							vals[ 1 ] = val.y;
							vals[ 2 ] = val.z;
							vals[ 3 ] = val.w;
							if ( ImGui::SliderFloat4( uniformName.c_str( ), (f32*)&vals, 0.0f, 3.0f ) )
							{ 
								mMaterial->SetUniform( uniformName, Enjon::Vec4( vals[ 0 ], vals[ 1 ], vals[ 2 ], vals[ 3 ] ) );
							}
						} break;

						case UniformType::TextureSampler2D:
						{
							UniformTexture* uf = uniform->Cast< UniformTexture >( );
							if ( ImGui::CollapsingHeader( uf->GetName( ).c_str( ) ) )
							{
								Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
								ImGui::ListBoxHeader( ( "##textures" + uniformName ).c_str( ) );
								{
									for ( auto& a : *am->GetAssets< Enjon::Texture >( ) )
									{
										Enjon::String texName = a.second->Cast< Enjon::Texture >( )->GetName( );
										if ( ImGui::Button( texName.c_str( ) ) )
										{
											Enjon::AssetHandle< Enjon::Texture > newTex = am->GetAsset< Enjon::Texture >( texName );
											mMaterial->SetUniform( uniformName, newTex );
										} 
									} 
								}
								ImGui::ListBoxFooter( );
							}
						} break;
					}

				} 
			}
		}

		if ( ImGui::Button( "Recompile Shader" ) )
		{
			Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
			Enjon::String shaderPath = am->GetAssetsPath( ) + "/Shaders"; 
			mShaderGraph.Create( shaderPath + "/ShaderGraphs/test.json" ); 
		}

		ImGui::Image( ImTextureID( mHDRTextureID ), ImVec2( 128, 128 ) );
		if ( brdfset )
		{
			ImGui::Image( ImTextureID( mBRDFHandle.Get()->GetTextureId() ), ImVec2( 128, 128 ) ); 
		}

		if ( ImGui::Button( "UseOther" ) )
		{
			useOther ^= 1;
		}
	}

	//=======================================================================================================

	void DeferredRenderer::ShowGameViewport(bool* open)
	{
	    // Render game in window
		ImVec2 cursorPos = ImGui::GetCursorScreenPos( );

	    ImTextureID img = (ImTextureID)mCurrentRenderTexture;
	    ImGui::Image(img, ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), 
	    				ImVec2(0,1), ImVec2(1,0), ImColor(255,255,255,255), ImColor(255,255,255,0));
		
		ImVec2 min = ImVec2 ( cursorPos.x + ImGui::GetContentRegionAvailWidth() - 100.0f, cursorPos.y + 10.0f );
		ImVec2 max = ImVec2( min.x + 50.0f, min.y + 10.0f );

		ImGui::SetCursorScreenPos( min );
		auto drawlist = ImGui::GetWindowDrawList( ); 
		//drawlist->AddRect( min, max, ImColor( 255, 255, 255, 255 ) );
		f32 fps = ImGui::GetIO( ).Framerate;
		drawlist->AddText( min, ImColor( 255, 255, 255, 255 ), fmt::sprintf( "Frame: %.3f", fps ).c_str() );

		// Update camera aspect ratio
		mSceneCamera.SetAspectRatio( ImGui::GetWindowWidth( ) / ImGui::GetWindowHeight( ) );
	} 

	// renderQuad() renders a 1x1 XY quad in NDC
	// -----------------------------------------
	unsigned int quadVAO = 0;
	unsigned int quadVBO = 0;
	void DeferredRenderer::RenderQuad( )
	{
		if ( quadVAO == 0 )
		{
			float quadVertices[ ] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays( 1, &quadVAO );
			glGenBuffers( 1, &quadVBO );
			glBindVertexArray( quadVAO );
			glBindBuffer( GL_ARRAY_BUFFER, quadVBO );
			glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), &quadVertices, GL_STATIC_DRAW );
			glEnableVertexAttribArray( 0 );
			glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* )0 );
			glEnableVertexAttribArray( 1 );
			glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof( float ), ( void* )( 3 * sizeof( float ) ) );
		}
		glBindVertexArray( quadVAO );
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		glBindVertexArray( 0 );
	}

	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;
	void DeferredRenderer::RenderCube( )
	{
		// initialize (if necessary)
		if ( cubeVAO == 0 )
		{
			float vertices[ ] = {
				// back face
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
				1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
				-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
				-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			  // front face
			  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			  -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		  // right face
		  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		   // bottom face
		   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		   1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		   -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 // top face
		 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
			};
			glGenVertexArrays( 1, &cubeVAO );
			glGenBuffers( 1, &cubeVBO );
			// fill buffer
			glBindBuffer( GL_ARRAY_BUFFER, cubeVBO );
			glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );
			// link vertex attributes
			glBindVertexArray( cubeVAO );
			glEnableVertexAttribArray( 0 );
			glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* )0 );
			glEnableVertexAttribArray( 1 );
			glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* )( 3 * sizeof( float ) ) );
			glEnableVertexAttribArray( 2 );
			glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof( float ), ( void* )( 6 * sizeof( float ) ) );
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
			glBindVertexArray( 0 );
		}
		// render Cube
		glBindVertexArray( cubeVAO );
		glDrawArrays( GL_TRIANGLES, 0, 36 );
		glBindVertexArray( 0 );
	}

}


















