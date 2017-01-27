#include "Graphics/DeferredRenderer.h"
#include "Graphics/FontManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Renderable.h"
#include "IO/ResourceManager.h"

namespace Enjon { namespace Graphics {

	DeferredRenderer::DeferredRenderer()
	{
	}

	DeferredRenderer::~DeferredRenderer()
	{
	}

	void DeferredRenderer::Init()
	{
		// TODO(John): Need to have a way to have an ini that's read or grab these values from a static
		// engine config file
		mWindow.Init("Game", 1440, 900); 

		// Initialize shader manager
		EG::ShaderManager::Init();

		// Initialize font manager
		EG::FontManager::Init();

		// Initialize scene camera
		mSceneCamera = EG::Camera(mWindow.GetViewport());

		// Initialize frame buffers
		InitializeFrameBuffers();
	}

	void DeferredRenderer::Update(float dt)
	{
		mWindow.Clear();

		// Just do this as simply as possible at first
		//GBufferPass();

		// Draw diffuse texture just for testing
		GLSLProgram* shader = EG::ShaderManager::Get("NoCameraProjection");		
		shader->Use();
		{
			mBatch.Begin();
			{
				// mWindow.Clear(1.0f, GL_COLOR_BUFFER_BIT);

				mBatch.Add(
							EM::Vec4(-1, -1, 2, 2),
							EM::Vec4(0, 0, 1, 1),
							mGbuffer.GetTexture(EG::GBufferTextureType::ALBEDO)
							// EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/bb8_diffuse.png").id
						);
			}
			mBatch.End();
			mBatch.RenderBatch();
		}
		shader->Unuse();

		mWindow.SwapBuffer();
	}

	void DeferredRenderer::GBufferPass()
	{
		// Bind gbuffer
		mGbuffer.Bind();

		// Clear buffer (default)
		mWindow.Clear();

		// Get sorted renderables by material
		std::vector<EG::Renderable*> sortedRenderables = mScene.GetRenderables(RenderableSortType::MATERIAL);

		/*
		if (!sortedRenderables.empty())
		{
			EG::Material* material = nullptr;
			EG::GLSLProgram* shader = nullptr;
			for (auto& renderable : sortedRenderables)
			{
				// Check for material switch
				EG::Material* curMaterial = renderable->GetMaterial();
				if (material != curMaterial)
				{
					// Set material
					material = curMaterial;

					// Unbind previous shader
					if (shader) shader->Unuse();	

					// Set shader
					shader = material->GetShader();

					// Bind shader
					shader->Use();

					// Set uniforms
					// material->SetUniforms();

					shader->BindTexture("u_diffuseMap", material->GetTexture(EG::TextureSlotType::ALBEDO), 0);
					shader->BindTexture("u_normalMap", material->GetTexture(EG::TextureSlotType::NORMAL), 1);
					shader->BindTexture("u_emissiveMap", material->GetTexture(EG::TextureSlotType::EMISSIVE), 2);
					shader->BindTexture("u_metallicMap", material->GetTexture(EG::TextureSlotType::METALLIC), 3);
					shader->BindTexture("u_roughnessMap", material->GetTexture(EG::TextureSlotType::ROUGHNESS), 4);
					shader->BindTexture("u_aoMap", material->GetTexture(EG::TextureSlotType::AO), 5);
				}

				// Now need to render
				SubmitRenderable(renderable);
			}
		}
		*/

		// Unbind gbuffer
		mGbuffer.Unbind();
	}

	void DeferredRenderer::SubmitRenderable(EG::Renderable* renderable)
	{
		EG::Mesh* mesh = renderable->GetMesh();
		GLSLProgram* shader = renderable->GetMaterial()->GetShader();
		mesh->Bind();
		{
			EM::Mat4 Model;
			Model *= EM::Mat4::Translate(renderable->GetPosition());
			Model *= EM::QuaternionToMat4(renderable->GetOrientation());
			Model *= EM::Mat4::Scale(renderable->GetScale());
			shader->SetUniform("model", Model);
			mesh->Submit();
		}
		mesh->Unbind();
	}

	void DeferredRenderer::SetViewport(EM::iVec2& dimensions)
	{
		mWindow.SetViewport(dimensions);
	}

	EM::iVec2 DeferredRenderer::GetViewport()
	{
		return mWindow.GetViewport();
	}

	void DeferredRenderer::InitializeFrameBuffers()
	{
		auto viewport = mWindow.GetViewport();
		Enjon::u32 width = (Enjon::u32)viewport.x;
		Enjon::u32 height = (Enjon::u32)viewport.y;

		mGbuffer 					= EG::GBuffer(width, height);
		mDebugTarget 				= EG::RenderTarget(width, height);
		mSmallBlurHorizontal 		= EG::RenderTarget(width / 2, height / 2);
		mSmallBlurVertical 			= EG::RenderTarget(width / 2, height / 2);
		mMediumBlurHorizontal 		= EG::RenderTarget(width  / 4, height  / 4);
		mMediumBlurVertical 		= EG::RenderTarget(width  / 4, height  / 4);
		mLargeBlurHorizontal 		= EG::RenderTarget(width / 64, height / 64);
		mLargeBlurVertical 			= EG::RenderTarget(width / 64, height / 64);
		mComposite 					= EG::RenderTarget(width, height);
		mLightingBuffer 			= EG::RenderTarget(width, height);
		mLuminanceBuffer 			= EG::RenderTarget(width, height);
		mFXAATarget 				= EG::RenderTarget(width, height);
		mShadowDepth 				= EG::RenderTarget(2048, 2048);

		mBatch.Init();
	}
}}


















