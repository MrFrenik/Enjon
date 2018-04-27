#include "Graphics/GraphicsSubsystem.h"
#include "Graphics/FrameBuffer.h"
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
#include "Asset/AssetManager.h"
#include "Asset/MeshAssetLoader.h"
#include "CVarsSystem.h"
#include "ImGui/ImGuiManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Skeleton.h"
#include "Graphics/ShaderGraph.h"
#include "Engine.h"
#include "SubsystemCatalog.h"
#include "Defines.h"

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

#define SSAO_KERNEL_SIZE 16

Enjon::Renderable mRenderable; 
std::vector < Enjon::Renderable > mRenderables;
Enjon::AssetHandle< Enjon::Texture > mBRDFHandle;
Enjon::AssetHandle< Enjon::ShaderGraph > mTestShaderGraph;
Enjon::Material* mMaterial = nullptr;
bool brdfset = false; 
bool useOther = false; 

namespace Enjon 
{ 
	//======================================================================================================

	Enjon::Result GraphicsSubsystem::Initialize()
	{
		// TODO(John): Need to have a way to have an .ini that's read or grab these values from a static
		// engine config file
		// mWindow.Init("Game", 1920, 1080, WindowFlagsMask((u32)WindowFlags::FULLSCREEN)); 
		mWindow.Init( "Game", 1440, 900, WindowFlags::RESIZABLE ); 
		mWindows.push_back( &mWindow ); 

		// Set current window
		mCurrentWindow = &mWindow;
		mWindow.MakeCurrent( );

		// Initialize window sdl cursors
		Window::InitSDLCursors( );

		// Initialize shader manager
		Enjon::ShaderManager::Init();

		// Initialize font manager
		Enjon::FontManager::Init();

		// Initialize sprite batch ( not really needed, I don't think...)
		mBatch 						= new SpriteBatch();
		mBatch->Init(); 

		// Initialize full screen quad for rendering rendertargets
		mFullScreenQuad 			= new FullScreenQuad();

		// Initialize frame buffers
		InitializeFrameBuffers();
		// Initialize random noise texture
		InitializeNoiseTexture( );
		// Calcualte blur weights
		CalculateBlurWeights();
		// Register cvars
		RegisterCVars();

		GLSLProgram* shader = Enjon::ShaderManager::Get("GBuffer");
		shader->Use();
			shader->SetUniform("u_albedoMap", 0);
			shader->SetUniform("u_normalMap", 1);
		shader->Unuse();

		mShowGraphicsOptionsWindow = false;
		auto graphicsMenuOption = [&]()
		{
        	ImGui::MenuItem("Graphics##options", NULL, &mShowGraphicsOptionsWindow);
		};

		mShowStyles = false;
		auto stylesMenuOption = [&]()
		{
        	ImGui::MenuItem("Styles##options", NULL, &mShowStyles);
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

	 	auto showStylesWindowFunc = [&]()
	 	{
			if (ImGui::BeginDock("Styles##options", &mShowStyles))
			{
				ImGui::ShowStyleEditor();	
			}
			ImGui::EndDock();
	 	}; 

		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		// ImGuiManager::Register(graphicsMenuOption);
		// TODO(John): I HATE the way this looks
		igm->RegisterMenuOption("View", "Graphics##Options", graphicsMenuOption);
		igm->RegisterWindow("Graphics", showGraphicsViewportFunc);
		igm->RegisterMenuOption("View", "Styles##Options", stylesMenuOption);
		igm->RegisterWindow("Styles", showStylesWindowFunc);

		// Set current render texture
		mCurrentRenderTexture = mFXAATarget->GetTexture();

		// Register docking layouts
	    //ImGuiManager::RegisterDockingLayout(ImGui::DockingLayout("Graphics", nullptr, ImGui::DockSlotType::Slot_Right, 0.2f));
	
		// Register shader graph templates
		Enjon::ShaderGraph::DeserializeTemplate( Enjon::Engine::GetInstance( )->GetConfig( ).GetEngineResourcePath( ) + "/Shaders/ShaderGraphTemplates/ShaderTemplates.json" );

		// Init ui sprite batch
		mUIBatch.Init( );

		// TODO(): I don't like random raw gl calls just lying around...
		glEnable( GL_DEPTH_TEST );
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		glEnable( GL_DEPTH_CLAMP );
		glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		// Initialize debug drawing
		InitDebugDrawing( );

		return Result::SUCCESS;
	}

	void GraphicsSubsystem::BindShader( const Enjon::Shader* shader )
	{
		if ( shader != mActiveShader )
		{
			mActiveShader = const_cast< Shader*> ( shader );
			mActiveShader->Use( );
		}
	} 

	//======================================================================================================

	Enjon::Result GraphicsSubsystem::Shutdown()
	{ 
		// Delete auxillary items
		delete( mBatch );
		delete( mFullScreenQuad );
		delete[] mModelMatricies; 
		delete mInstancedRenderable;

		mInstancedRenderable = nullptr;
		mFullScreenQuad = nullptr;
		mBatch = nullptr;
		mModelMatricies = nullptr;

		// Free all memory for render targets / frame buffers
		FreeAllFrameBuffers( );

		// Shutdown shader manager
		ShaderManager::DeleteShaders( );

		// Shutdown font manager
		FontManager::DeleteFonts( );

		// Clear noise kernel
		mSSAOKernel.clear( );
 
		return Result::SUCCESS; 
	}

	//======================================================================================================

	void GraphicsSubsystem::STBTest( ) 
	{
		glDisable( GL_CULL_FACE );

		Enjon::String rootPath = Enjon::Engine::GetInstance( )->GetConfig( ).GetRoot( );
		//Enjon::String hdrFilePath = "Textures/HDR/GCanyon_C_YumaPoint_3k.hdr";
		//Enjon::String hdrFilePath = "Textures/HDR/WinterForest_Ref.hdr";
		Enjon::String hdrFilePath = "Textures/HDR/03-Ueno-Shrine_3k.hdr";
		//Enjon::String hdrFilePath = "Textures/HDR/Newport_Loft_Ref.hdr";
		//Enjon::String hdrFilePath = rootPath + "IsoARPG/Assets/Textures/HDR/Factory_Catwalk_2k.hdr";
		//Enjon::String hdrFilePath = rootPath + "IsoARPG/Assets/Textures/HDR/WinterForest_Ref.hdr";
		//Enjon::String hdrFilePath = "Textures/HDR/Alexs_Apt_2k.hdr";
		//Enjon::String hdrFilePath = "Textures/HDR/Mono_Lake_B_Ref.hdr";
		//Enjon::String hdrFilePath = rootPath + "IsoARPG/Assets/Textures/HDR/Mans_Outside_2k.hdr";

		AssetManager* am = Engine::GetInstance( )->GetSubsystemCatalog( )->Get< AssetManager >( )->ConstCast< AssetManager >();
		//am->AddToDatabase( hdrFilePath ); 
		Enjon::String qualifiedName = AssetLoader::GetQualifiedName( hdrFilePath ); 
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
			Mat4x4 captureProj = Mat4x4::Perspective( 90.0f, 1.0f, 0.1f, 10.0f );
			Mat4x4 captureViews[ ] =  
			{
				Mat4x4::LookAt( Vec3(0.0f), Vec3( 1.0f, 0.0f, 0.0f ), Vec3( 0.0f, -1.0f, 0.0f ) ),
				Mat4x4::LookAt( Vec3(0.0f), Vec3( -1.0f, 0.0f, 0.0f ), Vec3( 0.0f, -1.0f, 0.0f ) ),
				Mat4x4::LookAt( Vec3(0.0f), Vec3( 0.0f, 1.0f, 0.0f ), Vec3( 0.0f, 0.0f, 1.0f ) ),
				Mat4x4::LookAt( Vec3(0.0f), Vec3( 0.0f, -1.0f, 0.0f ), Vec3( 0.0f, 0.0f, -1.0f ) ),
				Mat4x4::LookAt( Vec3(0.0f), Vec3( 0.0f, 0.0f, 1.0f ), Vec3( 0.0f, -1.0f, 0.0f ) ),
				Mat4x4::LookAt( Vec3(0.0f), Vec3( 0.0f, 0.0f, -1.0f ), Vec3( 0.0f, -1.0f, 0.0f ) )
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
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, 0 );
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
				mFullScreenQuad->Submit( );
			}
			brdfShader->Unuse( );

			glBindFramebuffer( GL_FRAMEBUFFER, 0 ); 
		}
	
		// Shader graph creation
		{ 
			mTestShaderGraph = am->GetAsset< Enjon::ShaderGraph >( "shaders.shadergraphs.testgraph" );

			mMaterial = am->GetAsset< Material >( "NewMaterial" ).Get()->ConstCast< Material >();

			for ( u32 i = 0; i < 0; ++i ) 
			{
				for ( u32 j = 0; j < 0; ++j )
				{
					Enjon::Renderable renderable;
					
					// Set renderable material
					renderable.SetMaterial( mMaterial, 0 );
					renderable.SetMesh( am->GetAsset< Enjon::Mesh >( "models.monkey" ) );
					renderable.SetPosition( Enjon::Vec3( j, 1.0f, i ) + Enjon::Vec3( -25, 0, 5 ) );

					mRenderables.push_back( renderable ); 
				}
			}
		}

		InstancingTest( );
	}

	//======================================================================================================

	void GraphicsSubsystem::InstancingTest( )
	{
		// generate a large list of semi-random model transformation matrices
		// ------------------------------------------------------------------
		mModelMatricies = new Enjon::Mat4x4[ mInstancedAmount ];
		f32 radius = 120.0;
		f32 offset = 40.0f;
		for ( u32 i = 0; i < mInstancedAmount; i++ )
		{
			Enjon::Mat4x4 model;
			 //1. translation: displace along circle with 'radius' in range [-offset, offset]
			f32 angle = ( f32 )i / ( f32 )mInstancedAmount * 360.0f;
			f32 displacement = ( rand( ) % ( s32 )( 2 * offset * 100 ) ) / 100.0f - offset;
			f32 x = sin( angle ) * radius + displacement;
			displacement = ( rand( ) % ( s32 )( 2 * offset * 100 ) ) / 100.0f - offset;
			f32 y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
			displacement = ( rand( ) % ( s32 )( 2 * offset * 100 ) ) / 100.0f - offset;
			f32 z = cos( angle ) * radius + displacement;
			model *= Enjon::Mat4x4::Translate( Enjon::Vec3( x, y, z ) );

			// 2. scale: Scale between 0.05 and 0.25f
			f32 scale = ( rand( ) % 40 ) / 100.0f + 0.05;
			model *= Enjon::Mat4x4::Scale( Enjon::Vec3( scale ) );

			// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
			f32 rotAngle = ( rand( ) % 360 );
			model *= Enjon::Mat4x4::Rotate( rotAngle, Enjon::Vec3( 0.4f, 0.6f, 0.8f ) ); 

			// 4. now add to list of matrices
			mModelMatricies[ i ] = model;
		} 

		// Get a mesh and make it instanced... or something
		auto db = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );
		Enjon::AssetHandle< Enjon::Mesh > mesh = db->GetAsset< Enjon::Mesh >( "models.unit_cube" );
		//if ( mesh )
		//{
		//	// Set bunny mesh for later use
		//	mInstancedRenderable = new Enjon::Renderable( );
		//	mInstancedRenderable->SetMesh( mesh );
		//	mInstancedRenderable->SetMaterial( db->GetDefaultAsset<Material>( ) );
		//	//Enjon::Material* instancedMat = new Enjon::Material( );
		//	//instancedMat->TwoSided( true );
		//	//instancedMat->SetTexture(Enjon::TextureSlotType::Albedo, db->GetAsset<Enjon::Texture>("materials.copperrock.albedo"));
		//	//instancedMat->SetTexture(Enjon::TextureSlotType::Normal, db->GetAsset<Enjon::Texture>("materials.copperrock.normal"));
		//	//instancedMat->SetTexture(Enjon::TextureSlotType::Metallic, db->GetAsset<Enjon::Texture>("materials.copperrock.roughness"));
		//	//instancedMat->SetTexture(Enjon::TextureSlotType::Roughness, db->GetAsset<Enjon::Texture>("materials.copperrock.metallic"));
		//	//instancedMat->SetTexture(Enjon::TextureSlotType::Emissive, db->GetAsset<Enjon::Texture>("textures.black"));
		//	//instancedMat->SetTexture(Enjon::TextureSlotType::AO, db->GetAsset<Enjon::Texture>("materials.copperrock.ao"));
		//	//mInstancedRenderable->SetMaterial( instancedMat );

		//	glGenBuffers( 1, &mInstancedVBO );
		//	glBindBuffer( GL_ARRAY_BUFFER, mInstancedVBO );
		//	glBufferData( GL_ARRAY_BUFFER, mInstancedAmount * sizeof( Enjon::Mat4x4 ), &mModelMatricies[ 0 ], GL_DYNAMIC_DRAW );

		//	// Vertex attributes for instanced model matrix
		//	glBindVertexArray( mesh.Get( )->GetSubmeshes(0)() );
		//	// Set attrib pointers for matrix
		//	glEnableVertexAttribArray( 4 );
		//	glVertexAttribPointer( 4, 4, GL_FLOAT, GL_FALSE, sizeof( Enjon::Mat4x4 ), ( void* )0 );

		//	glEnableVertexAttribArray( 5 );
		//	glVertexAttribPointer( 5, 4, GL_FLOAT, GL_FALSE, sizeof( Enjon::Mat4x4 ), ( void* )sizeof(Enjon::Vec4) );

		//	glEnableVertexAttribArray( 6 );
		//	glVertexAttribPointer( 6, 4, GL_FLOAT, GL_FALSE, sizeof( Enjon::Mat4x4 ), ( void* )( 2 * sizeof(Enjon::Vec4) ) );

		//	glEnableVertexAttribArray( 7 );
		//	glVertexAttribPointer( 7, 4, GL_FLOAT, GL_FALSE, sizeof( Enjon::Mat4x4 ), ( void* )( 3 * sizeof(Enjon::Vec4) ) );

		//	glVertexAttribDivisor( 4, 1 );
		//	glVertexAttribDivisor( 5, 1 );
		//	glVertexAttribDivisor( 6, 1 );
		//	glVertexAttribDivisor( 7, 1 );

		//	glBindVertexArray( 0 );
		//} 
	}

	//======================================================================================================

	void GraphicsSubsystem::Update(const f32 dT)
	{ 
		for ( auto& w : mWindows )
		{ 
			// Set current window
			mCurrentWindow = w;
			w->MakeCurrent( );

			static bool set = false;
			if ( !set )
			{ 
				STBTest( );
				set = true;
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
			CompositePass( mLightingBuffer );
			// Motion Blur Pass
			MotionBlurPass( mCompositeTarget );
			// FXAA pass
			 FXAAPass( mMotionBlurTarget );
			// Do UI pass
			UIPass( mFXAATarget ); 
	 
			// Clear default buffer
			mCurrentWindow->Clear( 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, RGBA32_Black() ); 

			// TODO(): Hate this : Compile it out
			if ( Engine::GetInstance( )->GetConfig( ).IsStandAloneApplication( ) )
			{
				PresentBackBuffer( );
			} 
			else
			{
				// Otherwise Enjon Editor views
				ImGuiPass(); 
			}

			mCurrentWindow->SwapBuffer(); 
		} 

		// Reset current window back to base window
		// TODO(): This will be handled elsewhere
		mCurrentWindow = &mWindow;
		mCurrentWindow->MakeCurrent( ); 
	}

	//======================================================================================================

	/*

		// Possible implementation of future system
		GraphicsSubsystem::Update()
		{
			// Grab all windows to be submitted
			WindowManager* wm = EngineSubsystem( WindowManager );

			for ( auto& window : wm->GetWindows() )
			{ 
				window->Submit();
			}
		}

		Window::Submit()
		{
			// Make this window current
			EngineSubsystem( WindowManager )->MakeCurrent( this );

			// Submit all render passes
			for ( auto& rt : mRenderTargets() )
			{
				rt->Submit();
			}
		}

		GBufferPass::Submit()  // RenderPass implementation
		{
			// Do Gbuffer stuff and write to render targets	
			// Need access to scene - not sure where this needs to be pulled from...
		} 

		SSAOPass::Submit()
		{
			// REQUIRES THAT A GBUFFER IS ATTACHED ( Not sure that I like having this dependency... )
		}

		-> 

		// Constructor requires a GBufferPass* that it will use ?
		// Or can it get a pass of type GBufferPass from GetFrameBuffer

		ex. SSAOPass::Submit()
		{
			// Get GBuffer from window
			GBuffer* gbuffer = mWindow->GetFrameBuffer< GBufferPass >();

			// Can only continue with valid gbuffer
			if ( gbuffer )
			{

			}
			else
			{
				assert(false);
			}
		}

		So Window will hold HashMap[u32, FrameBuffer*] mFrameBuffers;
	*/

	void GraphicsSubsystem::PresentBackBuffer( )
	{
		glViewport( 0, 0, ( s32 )GetViewport().x, ( s32 )GetViewport().y );
		auto program = Enjon::ShaderManager::Get( "NoCameraProjection" );
		program->Use( );
		{
			program->BindTexture( "tex", mCurrentRenderTexture, 0 );
			mFullScreenQuad->Submit( );
		}
		program->Unuse( );
	}

	iVec2 GraphicsSubsystem::GetImGuiViewport( ) const
	{
		auto disp = ImGui::GetIO( ).DisplaySize;
		return iVec2( disp.x, disp.y );
	}

	//======================================================================================================

	PickResult GraphicsSubsystem::GetPickedObjectResult( const Vec2& screenPosition )
	{
		return this->GetPickedObjectResult( iVec2( screenPosition.x, screenPosition.y ) );
	}

	//======================================================================================================

	PickResult GraphicsSubsystem::GetPickedObjectResult( const iVec2& screenPosition )
	{
		// Set pixel alignment for unpacking
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		mGbuffer->Bind( BindType::READ );
		glReadBuffer( GL_COLOR_ATTACHMENT0 + ( u32 )GBufferTextureType::OBJECT_ID );

		// Read at center of screen and convert to color
		u8 data[ 4 ];
		glReadPixels( screenPosition.x, GetViewport().y - screenPosition.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data );
		ColorRGBA32 color( ( f32 )data[ 0 ] / 255.0f, ( f32 )data[ 1 ] / 255.0f, ( f32 )data[ 2 ] / 255.0f, ( f32 )data[ 3 ] / 255.0f );

		// Get id from color
		u32 id = Renderable::ColorToID( color );

		// Get the entity that this id corresponds to
		EntityManager* em = EngineSubsystem( EntityManager );
		EntityHandle handle = em->GetRawEntity( id );

		// Unbind buffer
		mGbuffer->Unbind( );

		return PickResult { handle, id };
	}

	//======================================================================================================
	
	void GraphicsSubsystem::GBufferPass()
	{
		static float wt = 0.0f;
		wt += 0.001f;
		if ( wt >= std::numeric_limits<f32>::max( ) )
		{
			wt = 0.0f;
		}

		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
		glEnable(GL_DEPTH_TEST);
		glDepthFunc( GL_LESS );
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		// Bind gbuffer
		mGbuffer->Bind(); 

		// Clear albedo render target buffer (default)
		glClearBufferfv(GL_COLOR, (u32)GBufferTextureType::ALBEDO, mBGColor); 
		// Clear object id render target buffer
		const GLfloat whiteColor[ ] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glClearBufferfv(GL_COLOR, (u32)GBufferTextureType::OBJECT_ID, whiteColor); 
		// Clear normal render target
		const GLfloat blackColor[ ] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glClearBufferfv(GL_COLOR, (u32)GBufferTextureType::NORMAL, blackColor); 
 
		// Get sorted renderables by material
		const Vector<Renderable*>& sortedRenderables = mGraphicsScene.GetRenderables();
		const HashSet<QuadBatch*>& sortedQuadBatches = mGraphicsScene.GetQuadBatches(); 

		Camera* camera = mGraphicsScene.GetActiveCamera( );
		Mat4x4 viewMtx = camera->GetView( );
		Mat4x4 projMtx = camera->GetProjection( );
		Mat4x4 viewProjMtx = camera->GetViewProjection( );

		// Shader graph to be used
		Enjon::AssetHandle< Enjon::ShaderGraph > sg; 
		GLSLProgram* gbufferShader = Enjon::ShaderManager::Get("GBuffer");

		if (!sortedRenderables.empty())
		{ 
			const Material* material = nullptr;
			for (auto& renderable : sortedRenderables)
			{ 
				renderable->Bind( );
				{
					// For each submesh
					const Vector< SubMesh* >& subMeshes = renderable->GetMesh( ).Get( )->GetSubmeshes( );
					for ( u32 i = 0; i < subMeshes.size(); ++i )
					{
						const Material* curMaterial = renderable->GetMaterial( i ).Get( );
						sg = curMaterial->GetShaderGraph( );
						assert( curMaterial != nullptr );

						if ( sg )
						{
							Enjon::Shader* sgShader = const_cast< Shader * >( sg->GetShader( ShaderPassType::Deferred_StaticGeom ) );
							if ( material != curMaterial )
							{
								// Set material
								material = curMaterial;

								// Bind uniforms
								sgShader->Use( );
								sgShader->SetUniform( "uViewProjection", camera->GetViewProjection( ) );
								sgShader->SetUniform( "uWorldTime", wt );
								sgShader->SetUniform( "uViewPositionWorldSpace", camera->GetPosition( ) );
								sgShader->SetUniform( "uPreviousViewProjection", mPreviousViewProjectionMatrix );
								material->Bind( sgShader );
							}

							sgShader->SetUniform( "uObjectID", Renderable::IdToColor( renderable->GetRenderableID( ) ) ); 
							renderable->Submit( sg->GetShader( ShaderPassType::Deferred_StaticGeom ), subMeshes.at( i ) );
						}
					} 
				}
				renderable->Unbind( );
			}
		}

		// Attempt to do the skinned mesh things
		static Renderable skinnedRenderable;
		GLSLProgram* skinnedMeshProgram = ShaderManager::Get( "SkinnedMesh" );
		if ( skinnedMeshProgram )
		{
			skinnedMeshProgram->Use( );
			{
				// Grab mesh asset loader
				MeshAssetLoader* ml = EngineSubsystem( AssetManager )->GetLoader( Object::GetClass< MeshAssetLoader >( ) )->ConstCast< MeshAssetLoader >( );
				Vector<Skeleton*> skeletons = ml->GetSkeletons( );
				if ( !skeletons.empty( ) )
				{
					Skeleton* sk = skeletons.at( 0 );
					// Grab mesh to use for this skeleton ( "We'll just assume sk_mannequin, I think" )
					AssetHandle< Mesh > mesh = EngineSubsystem( AssetManager )->GetAsset< Mesh >( "meshes.maw" );
					AssetHandle< Material > mat = EngineSubsystem( AssetManager )->GetDefaultAsset< Material >( );
					
					skinnedRenderable.SetMesh( mesh );
					skinnedRenderable.SetMaterial( mat );

					skinnedRenderable.Bind( );
					{
						if ( mesh && sk )
						{
							auto transforms = sk->GetTransforms( );

							// Set transforms uniforms in shader
							for ( u32 i = 0; i < transforms.size(); ++i )
							{
								skinnedMeshProgram->SetUniform( "uJointTransforms[" + std::to_string( i ) + "]", transforms.at( i ) );
							}

							skinnedMeshProgram->SetUniform( "uViewProjection", camera->GetViewProjection( ) );
							skinnedMeshProgram->SetUniform( "uPreviousViewProjection", mPreviousViewProjectionMatrix ); 

							skinnedMeshProgram->SetUniform( "uObjectID", Renderable::IdToColor( skinnedRenderable.GetRenderableID( ) ) ); 

							for ( u32 i = 0; i < mesh->GetSubMeshCount( ); ++i )
							{
								skinnedRenderable.Submit( skinnedMeshProgram, mesh->GetSubmeshes().at( i ) ); 
							}
						} 
					}
					skinnedRenderable.Unbind( );
				}
			}
			skinnedMeshProgram->Unuse( );
		}

		// Quadbatches
		Enjon::GLSLProgram* shader = Enjon::ShaderManager::Get("QuadBatch");
		shader->Use();

		if (!sortedQuadBatches.empty())
		{
			// Set shared uniform
			shader->SetUniform("u_camera", camera->GetViewProjection());

			const Material* material = nullptr;
			for (auto& quadBatch : sortedQuadBatches)
			{ 
				// Check for material switch 
				const Material* curMaterial = quadBatch->GetMaterial().Get();
				sg = curMaterial->GetShaderGraph( );
				assert(curMaterial != nullptr); 

				// Grab shader from graph
				Enjon::Shader* sgShader = const_cast< Enjon::Shader* >( sg->GetShader( ShaderPassType::Deferred_StaticGeom ) );

				if ( sg )
				{
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
						
						sgShader->Use( );
						sgShader->SetUniform( "uViewProjection", camera->GetViewProjection( ) );
						sgShader->SetUniform( "uWorldTime", wt );
						sgShader->SetUniform( "uViewPositionWorldSpace", camera->GetPosition( ) );
						sgShader->SetUniform( "uPreviousViewProjection", mPreviousViewProjectionMatrix );
						material->Bind( sgShader ); 
					}

					// Need to set up renderable ids for quadbatches
					//sgShader->SetUniform( "uObjectID", Renderable::IdToColor( quadBatch->GetRenderableID( ) ) );
					// Render batch
					quadBatch->RenderBatch(); 
				} 
			}
		}
		shader->Unuse();
 
		static f32 rotT = 0.0f;
		rotT += 0.01f; 

		// Instancing test
		shader = Enjon::ShaderManager::Get( "Instanced" ); 
		shader->Use( );
		{
			// Set set shared uniform
			shader->SetUniform( "uProjection", camera->GetProjection( ) );
			shader->SetUniform( "uView", camera->GetView( ) );

			// Get material
			//const Material* material = mInstancedRenderable->GetMaterial( ).Get();

			// Set material textures
			/*shader->BindTexture( "uAlbedoMap", material->GetTexture( Enjon::TextureSlotType::Albedo ).Get( )->GetTextureId( ), 0 );
			shader->BindTexture( "uNormalMap", material->GetTexture( Enjon::TextureSlotType::Normal ).Get( )->GetTextureId( ), 1 );
			shader->BindTexture( "uEmissiveMap", material->GetTexture( Enjon::TextureSlotType::Emissive ).Get( )->GetTextureId( ), 2 );
			shader->BindTexture( "uMetallicMap", material->GetTexture( Enjon::TextureSlotType::Metallic ).Get( )->GetTextureId( ), 3 );
			shader->BindTexture( "uRoughnessMap", material->GetTexture( Enjon::TextureSlotType::Roughness ).Get( )->GetTextureId( ), 4 );
			shader->BindTexture( "uAoMap", material->GetTexture( Enjon::TextureSlotType::AO ).Get( )->GetTextureId( ), 5 );*/

			// Render instanced mesh
			//mInstancedRenderable->GetMesh( ).Get( )->Bind( );
		
			//glBindBuffer( GL_ARRAY_BUFFER, mInstancedVBO );
			//for ( u32 i = 0; i < mInstancedAmount; ++i )
			//{
			//	mModelMatricies[ i ] *= Enjon::Mat4x4::Rotate( rotT, Enjon::Vec3( 0, 1, 0 ) );
			//}
			//glBufferSubData( GL_ARRAY_BUFFER, 0, mInstancedAmount * sizeof( Enjon::Mat4x4 ), &mModelMatricies[ 0 ] );

			//glDrawArraysInstanced( GL_TRIANGLES, 0, mInstancedRenderable->GetMesh( ).Get( )->GetDrawCount(), mInstancedAmount );

			//mInstancedRenderable->GetMesh( ).Get( )->Unbind( ); 
		} 
		shader->Unuse( ); 

		// Cubemap
		//glCullFace( GL_FRONT );
		//Enjon::GLSLProgram* skyBoxShader = Enjon::ShaderManager::Get( "SkyBox" );
		//skyBoxShader->Use( );
		//{
		//	skyBoxShader->SetUniform( "view", mGraphicsScene.GetActiveCamera()->GetView( ) );
		//	skyBoxShader->SetUniform( "projection", mGraphicsScene.GetActiveCamera()->GetProjection( ) );
		//	skyBoxShader->BindTexture( "environmentMap", mEnvCubemapID, 0 );

		//	// TODO: When setting BindTexture on shader, have to set what the texture type is ( Texture2D, SamplerCube, etc. )
		//	glActiveTexture( GL_TEXTURE0 );
		//	glBindTexture( GL_TEXTURE_CUBE_MAP, mEnvCubemapID );

		//	RenderCube( );
		//}
		//skyBoxShader->Unuse( );

		// Unbind gbuffer
		mGbuffer->Unbind();

		// Store the previous view projection matrix
		mPreviousViewProjectionMatrix = camera->GetViewProjection( );

		glEnable( GL_DEPTH_TEST );
		glDepthFunc( GL_LEQUAL );
		glCullFace( GL_BACK );
	}

	//======================================================================================================

	void GraphicsSubsystem::SSAOPass( )
	{
		Enjon::iVec2 screenRes = GetViewport( ); 

		Camera* camera = mGraphicsScene.GetActiveCamera( );

		// SSAO pass
		mSSAOBlurTarget->Bind( );
		{
			glClear( GL_COLOR_BUFFER_BIT );

			GLSLProgram* shader = ShaderManager::Get( "SSAO" );
			shader->Use( );
			{ 
				// Upload kernel uniform
				shader->SetUniform( "projection", camera->GetProjection( ) );
				shader->SetUniform( "uProjMatrixInv", Mat4x4::Inverse( camera->GetProjection( ) ) );
				shader->SetUniform( "uViewMatrixInv", Mat4x4::Inverse( camera->GetView( ) ) );
				shader->SetUniform( "view", camera->GetView( ) );
				shader->SetUniform( "uScreenResolution", Vec2( screenRes.x, screenRes.y ) );
				shader->SetUniform( "radius", mSSAORadius );
				shader->SetUniform( "bias", mSSAOBias );
				shader->SetUniform( "uIntensity", mSSAOIntensity );
				shader->SetUniform( "uScale", mSSAOScale );
				shader->SetUniform( "uNear", camera->GetNear() );
				shader->SetUniform( "uFar", camera->GetFar() );
				shader->BindTexture( "gNormal", mGbuffer->GetTexture( GBufferTextureType::NORMAL ), 0 );
				shader->BindTexture( "texNoise", mSSAONoiseTexture, 1 ); 
				shader->BindTexture( "uDepthMap", mGbuffer->GetDepth( ), 2 ); 
				mFullScreenQuad->Submit( );
			}
			shader->Unuse( ); 
		}
		mSSAOBlurTarget->Unbind( );

		// Blur SSAO to remove noise
		//mSSAOBlurTarget->Bind( );
		//{
		//	glClear( GL_COLOR_BUFFER_BIT );

		//	GLSLProgram* shader = ShaderManager::Get( "SSAOBlur" );
		//	shader->Use( );
		//	{
		//		shader->BindTexture( "ssaoInput", mSSAOTarget->GetTexture( ), 0 );
		//		mFullScreenQuad->Submit( );
		//	}
		//	shader->Unuse( ); 
		//}
		//mSSAOBlurTarget->Unbind( );
	}

	//======================================================================================================

	void GraphicsSubsystem::LightingPass()
	{
		Camera* camera = mGraphicsScene.GetActiveCamera( );

		
		GLSLProgram* ambientShader 		= Enjon::ShaderManager::Get("AmbientLight");
		GLSLProgram* directionalShader 	= Enjon::ShaderManager::Get("PBRDirectionalLight");	
		GLSLProgram* pointShader 		= Enjon::ShaderManager::Get("PBRPointLight");	
		GLSLProgram* spotShader 		= Enjon::ShaderManager::Get("SpotLight");	

		const HashSet<DirectionalLight*>& directionalLights 	= mGraphicsScene.GetDirectionalLights();	
		const HashSet<SpotLight*>& spotLights 					= mGraphicsScene.GetSpotLights();	
		const HashSet<PointLight*>& pointLights 				= mGraphicsScene.GetPointLights();

		AmbientSettings* aS = mGraphicsScene.GetAmbientSettings();

		Mat4x4 projInverse = Mat4x4::Inverse( camera->GetProjection( ) );
		Mat4x4 viewInverse = Mat4x4::Inverse( camera->GetView( ) );

		mCurrentWindow->Clear(); 

		mGbuffer->Bind( BindType::READ, false ); 
		mLightingBuffer->Bind();
		glBlitFramebuffer( 0, 0, mGbuffer->GetResolution( ).x, mGbuffer->GetResolution( ).y, 0, 0, mLightingBuffer->GetResolution( ).x, mLightingBuffer->GetResolution( ).y, GL_DEPTH_BUFFER_BIT, GL_NEAREST );

		// Cubemap
		SubmitSkybox( ); 

		mGbuffer->Unbind( );

		mLightingBuffer->Bind( BindType::WRITE, false );

		// TODO(): Abstract these away 
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE); 

		// Ambient pass
		ambientShader->Use();
		{ 
			ambientShader->SetUniform( "uIrradianceMap", 0 );
			ambientShader->SetUniform( "uPrefilterMap", 1 );
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mIrradianceMap );
			glActiveTexture( GL_TEXTURE1 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mPrefilteredMap ); 

			// Bind textures
			ambientShader->BindTexture( "uBRDFLUT", mBRDFLUT, 2 );
			ambientShader->BindTexture("uAlbedoMap", mGbuffer->GetTexture(GBufferTextureType::ALBEDO), 3);
			ambientShader->BindTexture("uNormalMap", mGbuffer->GetTexture(GBufferTextureType::NORMAL), 4);
			ambientShader->BindTexture("uDepthMap", mGbuffer->GetDepth(), 5);
			ambientShader->BindTexture("uEmissiveMap", mGbuffer->GetTexture(GBufferTextureType::EMISSIVE), 6);
			ambientShader->BindTexture("uMaterialMap", mGbuffer->GetTexture(GBufferTextureType::MAT_PROPS), 7);
			ambientShader->BindTexture("uSSAOMap", mSSAOBlurTarget->GetTexture(), 8);

			// Bind uniforms
			ambientShader->SetUniform("uResolution", mGbuffer->GetResolution());
			ambientShader->SetUniform( "uCamPos", camera->GetPosition() );
			ambientShader->SetUniform( "uProjMatrixInv", projInverse );
			ambientShader->SetUniform( "uViewMatrixInv", viewInverse );
			// Render
			mFullScreenQuad->Submit( );
		}
		ambientShader->Unuse();

		directionalShader->Use();
		{
			directionalShader->SetUniform( "u_camPos", camera->GetPosition( ) );
			for (auto& l : directionalLights)
			{
				ColorRGBA32 color = l->GetColor();

				directionalShader->BindTexture("u_albedoMap", 	mGbuffer->GetTexture(GBufferTextureType::ALBEDO), 0);
				directionalShader->BindTexture("u_normalMap", 	mGbuffer->GetTexture(GBufferTextureType::NORMAL), 1);
				directionalShader->BindTexture("u_depthMap",	mGbuffer->GetDepth(), 2);
				directionalShader->BindTexture("u_matProps", 	mGbuffer->GetTexture(GBufferTextureType::MAT_PROPS), 3);
				directionalShader->BindTexture("u_ssao", 		mSSAOBlurTarget->GetTexture(), 4);
				// directionalShader->BindTexture("u_shadowMap", 		mShadowDepth->GetDepth(), 4);
				directionalShader->SetUniform("u_resolution", 		mGbuffer->GetResolution());
				directionalShader->SetUniform( "uProjMatrixInv", projInverse );
				directionalShader->SetUniform( "uViewMatrixInv", viewInverse );
				// directionalShader->SetUniform("u_lightSpaceMatrix", mShadowCamera->GetViewProjectionMatrix());
				// directionalShader->SetUniform("u_shadowBias", 		EM::Vec2(0.005f, ShadowBiasMax));
				directionalShader->SetUniform("u_lightDirection", 	l->GetDirection());															
				directionalShader->SetUniform("u_lightColor", 		Vec3(color.r, color.g, color.b));
				directionalShader->SetUniform("u_lightIntensity", 	l->GetIntensity());

				// Render	
				 mFullScreenQuad->Submit();
			}
		}
		directionalShader->Unuse();

		pointShader->Use();
		{
			pointShader->BindTexture( "u_albedoMap", mGbuffer->GetTexture( GBufferTextureType::ALBEDO ), 0 );
			pointShader->BindTexture( "u_normalMap", mGbuffer->GetTexture( GBufferTextureType::NORMAL ), 1 );
			pointShader->BindTexture( "u_depthMap", mGbuffer->GetDepth( ), 2 );
			pointShader->BindTexture( "u_matProps", mGbuffer->GetTexture( GBufferTextureType::MAT_PROPS ), 3 );
			pointShader->BindTexture( "u_ssao", mSSAOBlurTarget->GetTexture( ), 4 );
			pointShader->SetUniform( "u_resolution", mGbuffer->GetResolution( ) );
			pointShader->SetUniform( "u_camPos", camera->GetPosition( ) );
			pointShader->SetUniform( "uProjMatrixInv", projInverse );
			pointShader->SetUniform( "uViewMatrixInv", viewInverse );

			for (auto& l : pointLights)
			{
				ColorRGBA32& color = l->GetColor();
				Vec3& position = l->GetPosition();

				pointShader->SetUniform("u_lightPos", position);
				pointShader->SetUniform("u_lightColor", Vec3(color.r, color.g, color.b));
				pointShader->SetUniform("u_lightIntensity", l->GetIntensity());
				pointShader->SetUniform("u_attenuationRate", l->GetAttenuationRate());
				pointShader->SetUniform("u_radius", l->GetRadius());

				// Render Light to screen
				mFullScreenQuad->Submit( );
			}
		}
		pointShader->Unuse();

		spotShader->Use();
		{
			spotShader->BindTexture("u_albedoMap", mGbuffer->GetTexture(GBufferTextureType::ALBEDO), 0);
			spotShader->BindTexture("u_normalMap", mGbuffer->GetTexture(GBufferTextureType::NORMAL), 1);
			// spotShader->BindTexture("u_matProps", mGbuffer->GetTexture(GBufferTextureType::MAT_PROPS), 3);
			spotShader->SetUniform("u_resolution", mGbuffer->GetResolution());
			spotShader->SetUniform("u_camPos", camera->GetPosition());			

			for (auto& l : spotLights)
			{
				ColorRGBA32& color = l->GetColor();
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
				mFullScreenQuad->Submit( );
			}
		}
		spotShader->Unuse();

		// mFullScreenQuad->Unbind();
		mLightingBuffer->Unbind();	

		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	//======================================================================================================

	void GraphicsSubsystem::SubmitSkybox( )
	{
		// Cubemap
		glCullFace( GL_FRONT );
		Enjon::GLSLProgram* skyBoxShader = Enjon::ShaderManager::Get( "SkyBox" );
		skyBoxShader->Use( );
		{
			skyBoxShader->SetUniform( "view", mGraphicsScene.GetActiveCamera()->GetView( ) );
			skyBoxShader->SetUniform( "projection", mGraphicsScene.GetActiveCamera()->GetProjection( ) );
			skyBoxShader->BindTexture( "environmentMap", mIrradianceMap, 0 );

			// TODO: When setting BindTexture on shader, have to set what the texture type is ( Texture2D, SamplerCube, etc. )
			glActiveTexture( GL_TEXTURE0 );
			glBindTexture( GL_TEXTURE_CUBE_MAP, mIrradianceMap );

			RenderCube( );
		}
		skyBoxShader->Unuse( );
		glCullFace( GL_BACK );
	}

	//======================================================================================================

	void GraphicsSubsystem::LuminancePass()
	{
		GLSLProgram* luminanceProgram = Enjon::ShaderManager::Get("Bright");
		mLuminanceTarget->Bind();
		{
			mCurrentWindow->Clear(1.0f, GL_COLOR_BUFFER_BIT, RGBA32_Black());
			luminanceProgram->Use();
			{
				luminanceProgram->BindTexture( "tex", mLightingBuffer->GetTexture( ), 0 );
				luminanceProgram->BindTexture("u_emissiveMap", mGbuffer->GetTexture(GBufferTextureType::EMISSIVE), 1);
				luminanceProgram->SetUniform("u_threshold", mToneMapSettings.mThreshold);

				// Render
				mFullScreenQuad->Submit( ); 
			}
			luminanceProgram->Unuse();
		}
		mLuminanceTarget->Unbind();
	}

	//======================================================================================================

	void GraphicsSubsystem::BloomPass()
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
    		FrameBuffer* target = isEven ? mSmallBlurHorizontal : mSmallBlurVertical;
    		GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind(BindType::WRITE);
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
					program->BindTexture( "tex", texID, 0 );

					// Render
					mFullScreenQuad->Submit( );
				}
				program->Unuse();
			}	
			target->Unbind();
    	}

		// Medium blur
    	for (u32 i = 0; i < (u32)mBloomSettings.mIterations.y * 2; ++i)
    	{
    		bool isEven = (i % 2 == 0);
    		FrameBuffer* target = isEven ? mMediumBlurHorizontal : mMediumBlurVertical;
    		GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind(BindType::WRITE);
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
					program->BindTexture( "tex", texID, 0 );

					// Render
					mFullScreenQuad->Submit( );
				}
				program->Unuse();
			}	
			target->Unbind();
    	}

		// Large blur
    	for (u32 i = 0; i < (u32)mBloomSettings.mIterations.z * 2; ++i)
    	{
    		bool isEven = (i % 2 == 0);
    		FrameBuffer* target = isEven ? mLargeBlurHorizontal : mLargeBlurVertical;
    		GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind(BindType::WRITE);
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
					program->BindTexture( "tex", texID, 0 );

					// Render
					mFullScreenQuad->Submit( ); 
				}
				program->Unuse();
			}	
			target->Unbind();
    	}

		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	//======================================================================================================

	void GraphicsSubsystem::BloomPass2( )
	{
		GLSLProgram* horizontalBlurProgram = Enjon::ShaderManager::Get( "HorizontalBlur" );
		GLSLProgram* verticalBlurProgram = Enjon::ShaderManager::Get( "VerticalBlur" );

		glEnable( GL_BLEND );
		glDisable( GL_DEPTH_TEST );
		glBlendFunc( GL_ONE, GL_ONE ); 
	}

	//======================================================================================================

	void GraphicsSubsystem::FXAAPass(FrameBuffer* input)
	{
		GLSLProgram* fxaaProgram = Enjon::ShaderManager::Get("FXAA");
		Vector<Renderable*> nonDepthTestedRenderables = mGraphicsScene.GetNonDepthTestedRenderables( );
		mFXAATarget->Bind();
		{
			mCurrentWindow->Clear();
			fxaaProgram->Use();
			{
				auto viewPort = GetViewport();
				fxaaProgram->BindTexture( "tex", input->GetTexture( ), 0 );
				fxaaProgram->SetUniform("u_resolution", Vec2(viewPort.x, viewPort.y));
				fxaaProgram->SetUniform("u_FXAASettings", Vec3(mFXAASettings.mSpanMax, mFXAASettings.mReduceMul, mFXAASettings.mReduceMin));

				// Render
				mFullScreenQuad->Submit( );
			}
			fxaaProgram->Unuse();
		}
		mFXAATarget->Unbind();
	}

	//======================================================================================================

	void GraphicsSubsystem::MotionBlurPass( FrameBuffer* inputTarget )
	{
		Camera* camera = mGraphicsScene.GetActiveCamera( );
		Vector<Renderable*> nonDepthTestedRenderables = mGraphicsScene.GetNonDepthTestedRenderables( );
		GLSLProgram* motionBlurProgram = ShaderManager::Get( "MotionBlur" );
		mMotionBlurTarget->Bind( );
		{
			mCurrentWindow->Clear( );
			motionBlurProgram->Use( );
			{
				auto viewPort = GetViewport( );
				motionBlurProgram->BindTexture( "uInputTextureMap", inputTarget->GetTexture( ), 0 );
				motionBlurProgram->BindTexture( "uVelocityMap", mGbuffer->GetTexture( GBufferTextureType::VELOCITY ), 1 );

				//f32 velocityScale = Engine::GetInstance( )->GetWorldTime( ).GetFPS( ) / 60.0f;
				motionBlurProgram->SetUniform( "uVelocityScale", mMotionBlurVelocityScale );
				motionBlurProgram->SetUniform( "uEnabled", (bool)mMotionBlurEnabled );

				// Render
				mFullScreenQuad->Submit( );
			}
			motionBlurProgram->Unuse( );
		}

		// Render non depth tested renderables
		glClear( GL_DEPTH_BUFFER_BIT );

		// None depth tested renderables
		if ( !nonDepthTestedRenderables.empty( ) )
		{
			const Material* material = nullptr;
			for ( auto& renderable : nonDepthTestedRenderables )
			{
				// Check for material switch 
				const Material* curMaterial = renderable->GetMaterial( 0  ).Get( );
				AssetHandle< ShaderGraph > sg = curMaterial->GetShaderGraph( );
				assert( curMaterial != nullptr );

				// Grab shader from graph
				if ( sg )
				{
					Enjon::Shader* sgShader = const_cast<Enjon::Shader*>( sg->GetShader( ShaderPassType::Deferred_StaticGeom ) );

					if ( material != curMaterial )
					{
						// Set material
						material = curMaterial;

						sgShader->Use( );
						sgShader->SetUniform( "uViewProjection", camera->GetViewProjection( ) );
						sgShader->SetUniform( "uWorldTime", Engine::GetInstance( )->GetWorldTime( ).mTotalTime );
						sgShader->SetUniform( "uViewPositionWorldSpace", camera->GetPosition( ) );
						sgShader->SetUniform( "uPreviousViewProjection", camera->GetViewProjection( ) );
						material->Bind( sgShader );
					}

					// Render object
					sgShader->SetUniform( "uObjectID", Renderable::IdToColor( renderable->GetRenderableID( ) ) );
					renderable->Submit( sg->GetShader( ShaderPassType::Deferred_StaticGeom ) );
				}
			}
		}

		// Draw debug lines
		{
			// Blit the gbuffer depth first
			mGbuffer->Bind( BindType::READ, false ); 
			glBlitFramebuffer( 0, 0, mGbuffer->GetResolution( ).x, mGbuffer->GetResolution( ).y, 0, 0, mMotionBlurTarget->GetResolution( ).x, mMotionBlurTarget->GetResolution( ).y, GL_DEPTH_BUFFER_BIT, GL_NEAREST ); 

			// Draw the lines
			DebugDrawSubmit( );

			// Unbind the gbuffer from reading
			mGbuffer->Unbind( ); 
		}

		// Unbind the motion blur target
		mMotionBlurTarget->Unbind( );

		// Simply to write object ID
		mGbuffer->Bind( BindType::WRITE, false );
		{
			glClear( GL_DEPTH_BUFFER_BIT );

			// None depth tested renderables
			if ( !nonDepthTestedRenderables.empty( ) )
			{
				const Material* material = nullptr;
				for ( auto& renderable : nonDepthTestedRenderables )
				{
					// Check for material switch 
					const Material* curMaterial = renderable->GetMaterial( 0 ).Get( );
					AssetHandle< ShaderGraph > sg = curMaterial->GetShaderGraph( );
					assert( curMaterial != nullptr );

					if ( sg )
					{
						// Grab shader from graph
						Enjon::Shader* sgShader = const_cast<Enjon::Shader*>( sg->GetShader( ShaderPassType::Deferred_StaticGeom ) );

						if ( material != curMaterial )
						{
							// Set material
							material = curMaterial;

							sgShader->Use( );
							sgShader->SetUniform( "uViewProjection", camera->GetViewProjection( ) );
							sgShader->SetUniform( "uWorldTime", Engine::GetInstance( )->GetWorldTime( ).mTotalTime );
							sgShader->SetUniform( "uViewPositionWorldSpace", camera->GetPosition( ) );
							sgShader->SetUniform( "uPreviousViewProjection", camera->GetViewProjection( ) );
							material->Bind( sgShader );
						}

						// Render object
						sgShader->SetUniform( "uObjectID", Renderable::IdToColor( renderable->GetRenderableID( ) ) );
						renderable->Submit( sg->GetShader( ShaderPassType::Deferred_StaticGeom ) );
					}
				}
			}
		}
		mGbuffer->Unbind( ); 
	}

	//======================================================================================================

	void GraphicsSubsystem::CompositePass(FrameBuffer* input)
	{
		GLSLProgram* compositeProgram = Enjon::ShaderManager::Get("Composite"); 
		mCompositeTarget->Bind();
		{
			mCurrentWindow->Clear();
			compositeProgram->Use();
			{
				compositeProgram->BindTexture( "tex", input->GetTexture( ), 0 );
				compositeProgram->BindTexture( "u_blurTexSmall", mSmallBlurVertical->GetTexture( ), 1 );
				compositeProgram->BindTexture( "u_blurTexMedium", mMediumBlurVertical->GetTexture( ), 2 );
				compositeProgram->BindTexture( "u_blurTexLarge", mLargeBlurVertical->GetTexture( ), 3 );
				compositeProgram->SetUniform( "u_exposure", mToneMapSettings.mExposure );
				compositeProgram->SetUniform( "u_gamma", mToneMapSettings.mGamma );
				compositeProgram->SetUniform( "u_bloomScalar", mToneMapSettings.mBloomScalar );
				compositeProgram->SetUniform( "u_saturation", mToneMapSettings.mSaturation );

				// Render
				mFullScreenQuad->Submit( );
			}
			compositeProgram->Unuse();
		} 

		mCompositeTarget->Unbind();
	}

	//======================================================================================================

	void GraphicsSubsystem::ImGuiPass()
	{
		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		static bool show_test_window = false;
		static bool show_frame_rate = false;
		static bool show_graphics_window = true;
		static bool show_app_layout = false;
		static bool show_game_viewport = true;

        // Queue up gui
        igm->Render(mCurrentWindow->GetSDLWindow());

        // Flush
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        ImGui::Render(); 
		ImGui_ImplSdlGL3_RenderDrawData( ImGui::GetDrawData( ) );
	}

	//======================================================================================================

	void GraphicsSubsystem::UIPass( FrameBuffer* inputTarget )
	{
		bool isStandalone = Engine::GetInstance( )->GetConfig( ).IsStandAloneApplication( );

		Camera* camera = mGraphicsScene.GetActiveCamera( );

		ImGuiManager* igm = EngineSubsystem( ImGuiManager );

		if ( isStandalone )
		{
			// Queue up gui
			igm->Render(mCurrentWindow->GetSDLWindow()); 
			// Flush
			glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
			ImGui::Render(); 
		}

		inputTarget->Bind( BindType::WRITE, false );
		{
			auto shader = ShaderManager::GetShader( "Text" ); 
			shader->Use( );
			{
				auto dispX = ( s32 )ImGui::GetIO( ).DisplaySize.x; 
				auto dispY = ( s32 )ImGui::GetIO( ).DisplaySize.y;
				Mat4x4 ortho = Mat4x4::Orthographic(0.0f, (f32)dispX, 0.0f, (f32)dispY, -1, 1);
				shader->SetUniform( "projection", ortho );
				mUIBatch.Begin( );
				{
					// Print out frame time 
					if ( isStandalone )
					{
						auto wt = Engine::GetInstance( )->GetWorldTime( ).mTotalTime;
						auto uiFont = FontManager::GetFont("WeblySleek_16"); 
						f32 frameTime = 1000.0f / ( f32 )ImGui::GetIO( ).Framerate;

						auto vp = GetViewport( );
						Enjon::PrintText( 10.0f, vp.y - 20.0f, 1.0f, std::to_string( frameTime ) + " ms", uiFont, mUIBatch ); 
						Enjon::PrintText( 10.0f, vp.y - 40.0f, 1.0f, isStandalone ? "Standalone" : "Editor", uiFont, mUIBatch ); 
					}
				}
				mUIBatch.End( );
				mUIBatch.RenderBatch( ); 
			}
			shader->Unuse( ); 
		}

		inputTarget->Unbind( ); 
	}

	//======================================================================================================

	void GraphicsSubsystem::SetViewport(iVec2& dimensions)
	{
		mCurrentWindow->SetViewport(dimensions);
	}

	//======================================================================================================

	iVec2 GraphicsSubsystem::GetViewport() const
	{
		return mCurrentWindow->GetViewport();
	}

	//======================================================================================================

	f64 NormalPDF(const f64 x, const f64 s, f64 m = 0.0)
	{
		static const f64 inv_sqrt_2pi = 0.3989422804014327;
		f64 a = (x - m) / s;

		return inv_sqrt_2pi / s * std::exp(-0.5 * a * a);
	}

	//======================================================================================================

	void GraphicsSubsystem::InitializeNoiseTexture( )
	{
		// Generate sample kernel
		std::uniform_real_distribution< f32 > randomFloats( 0.0f, 1.0f );
		std::default_random_engine generator;
		for ( u32 i = 0; i < SSAO_KERNEL_SIZE; ++i )
		{
			Enjon::Vec3 sample( randomFloats( generator ) * 2.0f - 1.0f, randomFloats( generator ) * 2.0f - 1.0f, randomFloats( generator ) );
			sample *= randomFloats( generator );
			f32 scale = f32( i ) / (f32)SSAO_KERNEL_SIZE;

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

	void GraphicsSubsystem::FreeAllFrameBuffers( )
	{
		// Free all framebuffers and rendertargets
		delete( mGbuffer );
		delete( mDebugTarget );
		delete( mSmallBlurHorizontal );
		delete( mSmallBlurVertical );
		delete( mMediumBlurHorizontal );
		delete( mMediumBlurVertical );
		delete( mLargeBlurHorizontal );
		delete( mLargeBlurVertical );
		delete( mCompositeTarget );
		delete( mLightingBuffer );
		delete( mLuminanceTarget );
		delete( mFXAATarget );
		delete( mShadowDepth );
		delete( mFinalTarget );
		delete( mSSAOTarget );
		delete( mSSAOBlurTarget );
		delete( mMotionBlurTarget );

		mGbuffer = nullptr;
		mDebugTarget = nullptr;
		mSmallBlurHorizontal = nullptr;
		mSmallBlurVertical = nullptr;
		mMediumBlurHorizontal = nullptr;
		mMediumBlurVertical = nullptr;
		mLargeBlurHorizontal = nullptr;
		mLargeBlurVertical = nullptr;
		mCompositeTarget = nullptr;
		mLightingBuffer = nullptr;
		mLuminanceTarget = nullptr;
		mFXAATarget = nullptr;
		mShadowDepth = nullptr;
		mFinalTarget = nullptr;
		mSSAOTarget = nullptr;
		mSSAOBlurTarget = nullptr;
		mMotionBlurTarget = nullptr; 
	}

	void GraphicsSubsystem::ReinitializeFrameBuffers( )
	{
		// Free all previous memory for rendertargets / framebuffers
		FreeAllFrameBuffers( );

		// Reinitialize frame buffers
		InitializeFrameBuffers( );

		// Reset current rendertarget 
		mCurrentRenderTexture = mFXAATarget->GetTexture( );
	}

	//======================================================================================================

	void GraphicsSubsystem::InitializeFrameBuffers()
	{
		auto viewport = mCurrentWindow->GetViewport();
		Enjon::u32 width = (Enjon::u32)viewport.x;
		Enjon::u32 height = (Enjon::u32)viewport.y;

		mGbuffer 					= new GBuffer(width, height);
		mDebugTarget 				= new FrameBuffer(width, height);
		mSmallBlurHorizontal 		= new FrameBuffer(width / 4, height / 4);
		mSmallBlurVertical 			= new FrameBuffer(width / 4, height / 4);
		mMediumBlurHorizontal 		= new FrameBuffer(width  / 8, height  / 8);
		mMediumBlurVertical 		= new FrameBuffer(width  / 8, height  / 8);
		mLargeBlurHorizontal 		= new FrameBuffer(width / 16, height / 16);
		mLargeBlurVertical 			= new FrameBuffer(width / 16, height / 16);
		mCompositeTarget 			= new FrameBuffer(width, height);
		mLightingBuffer 			= new FrameBuffer(width, height);
		mLuminanceTarget 			= new FrameBuffer(width / 2, height / 2);
		mFXAATarget 				= new FrameBuffer(width, height);
		mShadowDepth 				= new FrameBuffer(2048, 2048);
		mFinalTarget				= new FrameBuffer( width, height );
		mSSAOTarget					= new FrameBuffer( width, height );
		mSSAOBlurTarget				= new FrameBuffer( width, height ); 
		mMotionBlurTarget			= new FrameBuffer( width, height ); 
	}

	//======================================================================================================

	void GraphicsSubsystem::CalculateBlurWeights()
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

	u32 GraphicsSubsystem::GetCurrentRenderTextureId( ) const
	{
		return (u32)mCurrentRenderTexture;
	}

	void GraphicsSubsystem::RegisterCVars()
	{
		// NOTE(): These aren't being cleared properly; will have them commented out until then.
		//Enjon::CVarsSystem::Register("exposure", &mToneMapSettings.mExposure, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("gamma", &mToneMapSettings.mGamma, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("bloomScale", &mToneMapSettings.mBloomScalar, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("saturation", &mToneMapSettings.mSaturation, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_weight_small", &mBloomSettings.mWeights.x, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_weight_medium", &mBloomSettings.mWeights.y, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_weight_large", &mBloomSettings.mWeights.z, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_iter_small", &mBloomSettings.mIterations.x, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_iter_medium", &mBloomSettings.mIterations.y, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_iter_large", &mBloomSettings.mIterations.z, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("threshold", &mToneMapSettings.mThreshold, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_radius_small", &mBloomSettings.mRadius.x, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_radius_medium", &mBloomSettings.mRadius.y, Enjon::CVarType::TYPE_FLOAT);
		//Enjon::CVarsSystem::Register("blur_radius_large", &mBloomSettings.mRadius.z, Enjon::CVarType::TYPE_FLOAT);
	}

	//======================================================================================================

	void GraphicsSubsystem::ShowGraphicsWindow(bool* p_open)
	{
	    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);                                 // Right align, keep 140 pixels for labels

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0, 0.6f, 0.0f, 1.0f));
        ImGui::Text("Graphics Options");
        ImGui::PopStyleColor(1);
        ImGui::Separator(); 

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
		    ImGui::SliderFloat("Radius##ssao", &mSSAORadius, 0.01, 0.1f, "%.3f");
		    ImGui::SliderFloat("Bias##ssao", &mSSAOBias, 0.01, 1.0f, "%.3f"); 
		    ImGui::SliderFloat("Intensity##ssao", &mSSAOIntensity, 0.0f, 15.0f, "%.2f"); 
		    ImGui::SliderFloat("Scale##ssao", &mSSAOScale, 0.0f, 5.0f, "%.2f"); 

		    ImGui::TreePop();
	    }

		if (ImGui::TreeNode("Motion Blur"))
	    {
		    ImGui::SliderFloat("Velocity Scale##motionblur", &mMotionBlurVelocityScale, 0.01, 5.0f, "%.3f");
		    ImGui::Checkbox("Enabled##motionblur", (bool*)&mMotionBlurEnabled ); 
		    ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("FrameBuffers"))
	    {
	    	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	    	ImGui::PushFont(atlas->Fonts[1]);
	        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2, 0.6f, 0.6f, 1.0f)); 
 
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
				if ( ImGui::ImageButton( img, ImVec2( 64, 64 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ), 1, ImVec4( 0, 0, 0, 0 ), ImColor( 255, 255, 255, 255 ) ) )
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
				const char* string_name = "Motion Blur";
				ImGui::Text( string_name );
				ImTextureID img = (ImTextureID)mMotionBlurTarget->GetTexture( );
				if ( ImGui::ImageButton( img, ImVec2( 64, 64 ), ImVec2( 0, 1 ), ImVec2( 1, 0 ), 1, ImVec4( 0, 0, 0, 0 ), ImColor( 255, 255, 255, 255 ) ) )
				{
					mCurrentRenderTexture = mMotionBlurTarget->GetTexture( );
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
	}

	//======================================================================================================= 

	unsigned int cubeVAO = 0;
	unsigned int cubeVBO = 0;
	void GraphicsSubsystem::RenderCube( )
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


	ShaderUniform* GraphicsSubsystem::NewShaderUniform( const MetaClass* uniformClass ) const
	{
		if ( uniformClass )
		{
			ShaderUniform* newUniform = (ShaderUniform*)uniformClass->Construct( );

			return newUniform;
		}

		return nullptr;
	}

	//====================================================================================================================

	void GraphicsSubsystem::DrawDebugLine( const Vec3& start, const Vec3& end, const Vec3& color )
	{
		// Construct new debug line
		DebugLine line = { start, color, end, color };

		// Push back into lines
		mDebugLines.push_back( line );
	}

	//====================================================================================================================

	void GraphicsSubsystem::DrawDebugCircle( const Vec3& center, const f32& radius, const Vec3& normal, const u32& numSegments, const Vec3& color )
	{
		u32 numSegs = numSegments < 4 ? 4 : numSegments;
		f32 dt = 360.0f / ( f32 )numSegs;

		// Compute rotation for circle
		Quaternion rot = Vec3::ZAxis( ).GetRotationTowards( normal );

		for ( f32 theta = dt; theta <= 360.0f; theta += dt )
		{
			f32 thetaRad = ToRadians( theta );
			f32 prevThetaRad = ToRadians( theta - dt );
			Vec3 startPos = rot * ( center + Vec3( std::cosf( thetaRad ), std::sinf( thetaRad ), 0.0f ) * radius );
			Vec3 endPos = rot * ( center + Vec3( std::cosf( prevThetaRad ), std::sinf( prevThetaRad ), 0.0f ) * radius );

			// Draw Line
			DrawDebugLine( startPos, endPos, color );
		} 

		// Draw end line
		f32 thetaRad = ToRadians( 0.0f );
		f32 prevThetaRad = ToRadians( 360.0f - dt );
		Vec3 startPos = rot * ( center + Vec3( std::cosf( thetaRad ), std::sinf( thetaRad ), 0.0f ) * radius );
		Vec3 endPos = rot * ( center + Vec3( std::cosf( prevThetaRad ), std::sinf( prevThetaRad ), 0.0f ) * radius );

		// Draw Line
		DrawDebugLine( startPos, endPos, color );

	}

	//====================================================================================================================

	void GraphicsSubsystem::DrawDebugSphere( const Vec3& center, const f32& radius, const u32& numSlices, const u32& numSegments, const Vec3& color )
	{
		u32 slices = numSlices < 5 ? 5 : numSlices; 
		f32 radSquared = radius * radius; 
		f32 dh = radius / (f32)numSlices;
		for ( f32 h = -radius; h <= radius; h += dh )
		{
			// Compute radius at this slice
			f32 a = std::sqrtf( radSquared - ( h * h ) );
			// Draw circle slice
			DrawDebugCircle( center + Vec3( 0.0f, 0.0f, h ), a, Vec3::YAxis( ), numSegments, color );
		} 

		// Draw encapsulating circles
		f32 dt = 360.0f / (f32)numSlices;
		for ( f32 theta = 0.0f; theta <= 360.0f; theta += dt )
		{
			Vec3 normal = Quaternion::AngleAxis( ToRadians( theta ), Vec3::YAxis( ) ) * Vec3::ZAxis( );
			DrawDebugCircle( center, radius, normal, numSegments, color );
		}
	} 

	//====================================================================================================================

	void GraphicsSubsystem::DrawDebugAABB( const Vec3& min, const Vec3& max, const Vec3& color )
	{
		// Calculate dimensions
		f32 width = max.x - min.x;
		f32 height = max.y - min.y;
		f32 depth = max.z - min.z; 

		// 0
		DrawDebugLine( min, min + Vec3(width, 0.0f, 0.0f ), color );

		// 1
		DrawDebugLine( min, min + Vec3( 0.0f, height, 0.0f ), color );

		// 2
		DrawDebugLine( min + Vec3(width, 0.0f, 0.0f), min + Vec3( width, height, 0.0f ), color );

		// 3
		DrawDebugLine( min + Vec3(0.0f, height, 0.0f), min + Vec3( width, height, 0.0f ), color );

		// 4
		DrawDebugLine( min + Vec3(width, 0.0f, 0.0f), min + Vec3( width, 0.0f, depth ), color );

		// 5
		DrawDebugLine( min, min + Vec3( 0.0f, 0.0f, depth ), color ); 

		// 6
		DrawDebugLine( min + Vec3( 0.0f, height, 0.0f ), min + Vec3( 0.0f, height, depth ), color ); 

		// 7
		DrawDebugLine( min + Vec3( 0.0f, 0.0f, depth ), min + Vec3( 0.0f, height, depth ), color ); 

		// 8
		DrawDebugLine( min + Vec3( width, height, 0.0f ), min + Vec3( width, height, depth ), color ); 

		// 9
		DrawDebugLine( min + Vec3( 0.0f, 0.0f, depth ), min + Vec3( width, 0.0f, depth ), color ); 

		// 10
		DrawDebugLine( min + Vec3( 0.0f, height, depth ), min + Vec3( width, height, depth ), color ); 

		// 11
		DrawDebugLine( min + Vec3( width, 0.0f, depth ), min + Vec3( width, height, depth ), color ); 
	}

	void GraphicsSubsystem::DebugDrawSubmit( )
	{
		// Bind the buffer
		glBindBuffer( GL_ARRAY_BUFFER, mDebugLineVBO );

		// Upload the data
		u32 count = 0;
		for ( auto& l : mDebugLines )
		{
			float sd[ 12 ];
			sd[ 0 ] = l.mStart.x;
			sd[ 1 ] = l.mStart.y;
			sd[ 2 ] = l.mStart.z;
			sd[ 3 ] = l.mStartColor.x;
			sd[ 4 ] = l.mStartColor.y;
			sd[ 5 ] = l.mStartColor.z;

			sd[ 6 ] = l.mEnd.x;
			sd[ 7 ] = l.mEnd.y;
			sd[ 8 ] = l.mEnd.z;
			sd[ 9 ] = l.mEndColor.x;
			sd[ 10 ] = l.mEndColor.y;
			sd[ 11 ] = l.mEndColor.z;

			GLintptr os = sizeof( sd ) * count;
			GLsizei sz = sizeof( sd );
			glBufferSubData(GL_ARRAY_BUFFER, os, sz, sd ); 
			count++;
		}

		// Unbind buffer
		glBindBuffer( GL_ARRAY_BUFFER, 0 ); 

		// Get the shader
		GLSLProgram* shader = ShaderManager::Get( "DebugLine" );
		shader->Use( );
		{
			// Set uniforms
			shader->SetUniform( "uView", mGraphicsScene.GetActiveCamera( )->GetView( ) );
			shader->SetUniform( "uProjection", mGraphicsScene.GetActiveCamera( )->GetProjection( ) );

			// Submit data
			glBindVertexArray( mDebugLineVAO ); 
			{
				glDrawArrays( GL_LINES, 0, 2 * count );
			} 
			glBindVertexArray( 0 ); 
		}
		shader->Unuse( ); 

		// Clear debug lines
		mDebugLines.clear( );
	}

	void GraphicsSubsystem::InitDebugDrawing( )
	{
			// Generate the VAO if it isn't already generated 
			if (mDebugLineVAO == 0) 
			{
				glGenVertexArrays(1, &mDebugLineVAO);
			}

			// Bind the VAO. All subsequent opengl calls will modify its state.
			glBindVertexArray(mDebugLineVAO);

			// Generate the VBO if it isn't already generated 
			if (mDebugLineVBO == 0) 
			{
				glGenBuffers(1, &mDebugLineVBO);
			}
			glBindBuffer(GL_ARRAY_BUFFER, mDebugLineVBO);

			// Data size
			glBufferData( GL_ARRAY_BUFFER, sizeof( DebugLine ) * MAX_DEBUG_LINES, NULL, GL_DYNAMIC_DRAW );

			// Tell opengl what attribute arrays we need 
			glEnableVertexAttribArray(0);	// Point
			glEnableVertexAttribArray(1);	// Color

			// Calculate stride of overall data
			GLsizei stride = ( GLsizei )( ( f32 )( sizeof( DebugLine ) ) / 2.0f );
			// Compute offset for color within stride
			GLintptr offs = 4 * 3;

			// This is the position attribute pointer 
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
			// This is the color attribute pointer 
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offs );

			// Unbind the VAO 
			glBindVertexArray(0);
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}

}


















