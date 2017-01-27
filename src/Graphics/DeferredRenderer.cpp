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
		delete(mGbuffer);
		delete(mDebugTarget);
		delete(mSmallBlurHorizontal);
		delete(mSmallBlurVertical);
		delete(mMediumBlurHorizontal);
		delete(mMediumBlurVertical);
		delete(mLargeBlurHorizontal);
		delete(mLargeBlurVertical);
		delete(mComposite);
		delete(mLightingBuffer);
		delete(mLuminanceBuffer);
		delete(mFXAATarget);
		delete(mShadowDepth);
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
		mSceneCamera.SetProjection(EG::ProjectionType::PERSPECTIVE);
		mSceneCamera.SetPosition(EM::Vec3(0, 5, 10));
		mSceneCamera.LookAt(EM::Vec3(0, 0, 0));

		// Initialize frame buffers
		InitializeFrameBuffers();

		EG::GLSLProgram* shader = EG::ShaderManager::Get("GBuffer");
		shader->Use();
			shader->SetUniform("u_albedoMap", 0);
			shader->SetUniform("u_normalMap", 1);
		shader->Unuse();

		// TODO(): I don't like random raw gl calls just lying around...
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	void DeferredRenderer::Update(float dt)
	{
		// Just do this as simply as possible at first
		GBufferPass();
		LightingPass();

		// Draw diffuse texture just for testing
		GLSLProgram* shader = EG::ShaderManager::Get("NoCameraProjection");		
		shader->Use();
		{
			mWindow.Clear();

			mBatch.Begin();
			{
				mBatch.Add(
							EM::Vec4(-1, -1, 2, 2),
							EM::Vec4(0, 0, 1, 1),
							mGbuffer->GetTexture(EG::GBufferTextureType::NORMAL)
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
		mGbuffer->Bind();

		EG::GLSLProgram* shader = EG::ShaderManager::Get("GBuffer");

		// Use gbuffer shader
		shader->Use();

		// Clear buffer (default)
		mWindow.Clear();

		// Get sorted renderables by material
		std::vector<EG::Renderable*> sortedRenderables = mScene.GetRenderables();

		if (!sortedRenderables.empty())
		{
			EG::Material* material = nullptr;
			for (auto& renderable : sortedRenderables)
			{
				// Check for material switch
				EG::Material* curMaterial = renderable->GetMaterial();
				if (material != curMaterial)
				{
					// Set material
					material = curMaterial;

					// Set material textures
					shader->BindTexture("u_albedoMap", material->GetTexture(EG::TextureSlotType::ALBEDO), 0);
					shader->BindTexture("u_normalMap", material->GetTexture(EG::TextureSlotType::NORMAL), 1);
					shader->BindTexture("u_emissiveMap", material->GetTexture(EG::TextureSlotType::EMISSIVE), 2);
					shader->BindTexture("u_metallicMap", material->GetTexture(EG::TextureSlotType::METALLIC), 3);
					shader->BindTexture("u_roughnessMap", material->GetTexture(EG::TextureSlotType::ROUGHNESS), 4);
					shader->BindTexture("u_aoMap", material->GetTexture(EG::TextureSlotType::AO), 5);
					shader->SetUniform("camera", mSceneCamera.GetViewProjection());
				}

				// Now need to render
				EG::Mesh* mesh = renderable->GetMesh();
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
		}

		// Unuse shader
		shader->Unuse();

		// Unbind gbuffer
		mGbuffer->Unbind();
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

	void DeferredRenderer::LightingPass()
	{
		mLightingBuffer->Bind();
		
		EG::GLSLProgram* directionalShader 	= EG::ShaderManager::Get("DirectionalLight");	
		EG::GLSLProgram* pointShader 		= EG::ShaderManager::Get("PointLight");	
		EG::GLSLProgram* spotShader 		= EG::ShaderManager::Get("SpotLight");	

		std::set<EG::DirectionalLight*>* directionalLights 	= mScene.GetDirectionalLights();	
		std::set<EG::SpotLight*>* spotLights 				= mScene.GetSpotLights();	
		std::set<EG::PointLight*>* pointLights 				= mScene.GetPointLights();

		mLightingBuffer->Unbind();	
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

		mGbuffer 					= new EG::GBuffer(width, height);
		mDebugTarget 				= new EG::RenderTarget(width, height);
		mSmallBlurHorizontal 		= new EG::RenderTarget(width / 2, height / 2);
		mSmallBlurVertical 			= new EG::RenderTarget(width / 2, height / 2);
		mMediumBlurHorizontal 		= new EG::RenderTarget(width  / 4, height  / 4);
		mMediumBlurVertical 		= new EG::RenderTarget(width  / 4, height  / 4);
		mLargeBlurHorizontal 		= new EG::RenderTarget(width / 64, height / 64);
		mLargeBlurVertical 			= new EG::RenderTarget(width / 64, height / 64);
		mComposite 					= new EG::RenderTarget(width, height);
		mLightingBuffer 			= new EG::RenderTarget(width, height);
		mLuminanceBuffer 			= new EG::RenderTarget(width, height);
		mFXAATarget 				= new EG::RenderTarget(width, height);
		mShadowDepth 				= new EG::RenderTarget(2048, 2048);

		mBatch.Init();
	}
}}


















