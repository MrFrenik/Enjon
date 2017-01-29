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
#include "Graphics/Color.h"
#include "Graphics/DirectionalLight.h"
#include "Graphics/PointLight.h"
#include "Graphics/SpotLight.h"
#include "IO/ResourceManager.h"

#include <cassert>

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
		// Gbuffer pass
		GBufferPass();
		// Lighting pass
		LightingPass();

		// Draw diffuse texture just for testing
		GLSLProgram* shader = EG::ShaderManager::Get("NoCameraProjection");		
		shader->Use();
		{
			mWindow.Clear();

			mBatch->Begin();
			{
				mBatch->Add(
							EM::Vec4(-1, -1, 2, 2),
							EM::Vec4(0, 0, 1, 1),
							mLightingBuffer->GetTexture()
							// mGbuffer->GetTexture(EG::GBufferTextureType::NORMAL)
						);
			}
			mBatch->End();
			mBatch->RenderBatch();
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
		std::set<EG::QuadBatch*>* sortedQuadBatches = mScene.GetQuadBatches();

		if (!sortedRenderables.empty())
		{
			// Set set shared uniform
			shader->SetUniform("u_camera", mSceneCamera.GetViewProjection());

			EG::Material* material = nullptr;
			for (auto& renderable : sortedRenderables)
			{
				// Check for material switch
				EG::Material* curMaterial = renderable->GetMaterial();
				assert(curMaterial != nullptr);
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
				}

				// Now need to render
				EG::Mesh* mesh = renderable->GetMesh();
				mesh->Bind();
				{
					EM::Mat4 Model;
					Model *= EM::Mat4::Translate(renderable->GetPosition());
					Model *= EM::QuaternionToMat4(renderable->GetOrientation());
					Model *= EM::Mat4::Scale(renderable->GetScale());
					shader->SetUniform("u_model", Model);
					mesh->Submit();
				}
				mesh->Unbind();
			}
		}

		// Unuse gbuffer shader
		shader->Unuse();

		// Quadbatches
		shader = EG::ShaderManager::Get("QuadBatch");
		shader->Use();

		if (!sortedQuadBatches->empty())
		{
			// Set shared uniform
			shader->SetUniform("u_camera", mSceneCamera.GetViewProjection());

			EG::Material* material = nullptr;
			for (auto& quadBatch : *sortedQuadBatches)
			{
				EG::Material* curMaterial = quadBatch->GetMaterial();
				assert(curMaterial != nullptr);
				if (material != curMaterial)
				{
					// Set material
					material = curMaterial;

					// Set material tetxures
					shader->BindTexture("u_albedoMap", material->GetTexture(EG::TextureSlotType::ALBEDO), 0);
					shader->BindTexture("u_albedoMap", material->GetTexture(EG::TextureSlotType::ALBEDO), 0);
					shader->BindTexture("u_normalMap", material->GetTexture(EG::TextureSlotType::NORMAL), 1);
					shader->BindTexture("u_emissiveMap", material->GetTexture(EG::TextureSlotType::EMISSIVE), 2);
					shader->BindTexture("u_metallicMap", material->GetTexture(EG::TextureSlotType::METALLIC), 3);
					shader->BindTexture("u_roughnessMap", material->GetTexture(EG::TextureSlotType::ROUGHNESS), 4);
					shader->BindTexture("u_aoMap", material->GetTexture(EG::TextureSlotType::AO), 5);
				}

				// Render batch
				quadBatch->RenderBatch();
			}
		}

		// Unuse quadbatch shader
		shader->Unuse();

		// Unbind gbuffer
		mGbuffer->Unbind();
	}

	void DeferredRenderer::LightingPass()
	{
		mLightingBuffer->Bind();
		mFullScreenQuad->Bind();
		
		EG::GLSLProgram* ambientShader 		= EG::ShaderManager::Get("AmbientLight");
		EG::GLSLProgram* directionalShader 	= EG::ShaderManager::Get("DirectionalLight");	
		EG::GLSLProgram* pointShader 		= EG::ShaderManager::Get("PointLight");	
		EG::GLSLProgram* spotShader 		= EG::ShaderManager::Get("SpotLight");	

		std::set<EG::DirectionalLight*>* directionalLights 	= mScene.GetDirectionalLights();	
		std::set<EG::SpotLight*>* spotLights 				= mScene.GetSpotLights();	
		std::set<EG::PointLight*>* pointLights 				= mScene.GetPointLights();

		EG::AmbientSettings* aS = mScene.GetAmbientSettings();

		mWindow.Clear();

		// TODO(): Abstract these away 
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE);

		// Ambient pass
		ambientShader->Use();
		{
			ambientShader->BindTexture("u_albedoMap", mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO), 0);
			ambientShader->BindTexture("u_emissiveMap", mGbuffer->GetTexture(EG::GBufferTextureType::EMISSIVE), 1);
			ambientShader->SetUniform("u_ambientColor", EM::Vec3(aS->mColor.r, aS->mColor.g, aS->mColor.b));
			ambientShader->SetUniform("u_ambientIntensity", aS->mIntensity);
			ambientShader->SetUniform("u_resolution", mGbuffer->GetResolution());

			// Render
				mBatch->Begin();
					mBatch->Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1),
									mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO)
								);
				mBatch->End();
				mBatch->RenderBatch();
		}
		ambientShader->Unuse();

		directionalShader->Use();
		{
			for (auto& l : *directionalLights)
			{
				EG::ColorRGBA16* color = l->GetColor();

				directionalShader->BindTexture("u_albedoMap", 		mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO), 0);
				directionalShader->BindTexture("u_normalMap", 		mGbuffer->GetTexture(EG::GBufferTextureType::NORMAL), 1);
				directionalShader->BindTexture("u_positionMap", 	mGbuffer->GetTexture(EG::GBufferTextureType::POSITION), 2);
				// directionalShader->BindTexture("u_shadowMap", 		mShadowDepth->GetDepth(), 4);
				directionalShader->SetUniform("u_resolution", 		mGbuffer->GetResolution());
				// directionalShader->SetUniform("u_lightSpaceMatrix", mShadowCamera->GetViewProjectionMatrix());
				// directionalShader->SetUniform("u_shadowBias", 		EM::Vec2(0.005f, ShadowBiasMax));
				directionalShader->SetUniform("u_lightDirection", 	*l->GetDirection());															
				directionalShader->SetUniform("u_lightColor", 		EM::Vec3(color->r, color->g, color->b));
				directionalShader->SetUniform("u_lightIntensity", 	l->GetIntensity());

				// Render	
				// TODO(): Fix full screen quad - super janky
				// mFullScreenQuad->Submit();
				mBatch->Begin();
					mBatch->Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1),
									mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO)
								);
				mBatch->End();
				mBatch->RenderBatch();
			}
		}
		directionalShader->Unuse();

		pointShader->Use();
		{
			pointShader->BindTexture("u_albedoMap", mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO), 0);
			pointShader->BindTexture("u_normalMap", mGbuffer->GetTexture(EG::GBufferTextureType::NORMAL), 1);
			pointShader->BindTexture("u_positionMap", mGbuffer->GetTexture(EG::GBufferTextureType::POSITION), 2);
			// pointShader->BindTexture("MaterialProperties", mGbuffer->GetTexture(EG::GBufferTextureType::MAT_PROPS), 3);
			pointShader->SetUniform("u_resolution", mGbuffer->GetResolution());
			pointShader->SetUniform("u_camPos", mSceneCamera.GetPosition());			

			for (auto& l : *pointLights)
			{
				EG::ColorRGBA16& color = l->GetColor();
				EG::PLParams& params = l->GetParams();
				EM::Vec3& position = l->GetPosition();

				pointShader->SetUniform("u_lightPos", position);
				pointShader->SetUniform("u_lightColor", EM::Vec3(color.r, color.g, color.b));
				pointShader->SetUniform("u_falloff", params.mFalloff);
				pointShader->SetUniform("u_lightIntensity", l->GetIntensity());

				// Render Light to screen
				mBatch->Begin();
					mBatch->Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1),
									mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO)
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
				spotShader->BindTexture("u_albedoMap", mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO), 0);
				spotShader->BindTexture("u_normalMap", mGbuffer->GetTexture(EG::GBufferTextureType::NORMAL), 1);
				spotShader->BindTexture("u_positionMap", mGbuffer->GetTexture(EG::GBufferTextureType::POSITION), 2);
				// spotShader->BindTexture("MaterialProperties", mGbuffer->GetTexture(EG::GBufferTextureType::MAT_PROPS), 3);
				spotShader->SetUniform("u_resolution", mGbuffer->GetResolution());
				spotShader->SetUniform("u_camPos", mSceneCamera.GetPosition());			

				for (auto& l : *spotLights)
				{
					EG::ColorRGBA16& color = l->GetColor();
					EG::SLParams& params = l->GetParams();
					EM::Vec3& position = l->GetPosition();

					spotShader->SetUniform("u_lightPos", position);
					spotShader->SetUniform("u_lightColor", EM::Vec3(color.r, color.g, color.b));
					spotShader->SetUniform("u_falloff", params.mFalloff);
					spotShader->SetUniform("u_lightIntensity", l->GetIntensity());
					spotShader->SetUniform("u_lightDirection", params.mDirection);
					spotShader->SetUniform("u_innerCutoff", params.mInnerCutoff);
					spotShader->SetUniform("u_outerCutoff", params.mOuterCutoff);

					// Render Light to screen
					mBatch->Begin();
						mBatch->Add(
										EM::Vec4(-1, -1, 2, 2),
										EM::Vec4(0, 0, 1, 1),
										mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO)
									);
					mBatch->End();
					mBatch->RenderBatch();
				}
			}
		}
		spotShader->Unuse();

		mFullScreenQuad->Unbind();
		mLightingBuffer->Unbind();	

		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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

		mBatch 						= new EG::SpriteBatch();
		mBatch->Init();

		mFullScreenQuad 			= new EG::FullScreenQuad();
	}
}}


















