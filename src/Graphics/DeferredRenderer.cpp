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
#include "Console.h"
#include "CVarsSystem.h"
#include "ImGui/imgui_impl_sdl_gl3.h"

#include <cassert>

namespace Enjon { namespace Graphics {

	//------------------------------------------------------------------------------
	DeferredRenderer::DeferredRenderer()
	{
	}

	//------------------------------------------------------------------------------
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
		delete(mCompositeTarget);
		delete(mLightingBuffer);
		delete(mLuminanceTarget);
		delete(mFXAATarget);
		delete(mShadowDepth);

		ImGui_ImplSdlGL3_Shutdown();
	}

	//------------------------------------------------------------------------------
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
		// Calcualte blur weights
		CalculateBlurWeights();
		// Register cvars
		RegisterCVars();

		EG::GLSLProgram* shader = EG::ShaderManager::Get("GBuffer");
		shader->Use();
			shader->SetUniform("u_albedoMap", 0);
			shader->SetUniform("u_normalMap", 1);
		shader->Unuse();

		// Set up ImGui
		ImGui_ImplSdlGL3_Init(mWindow.GetSDLWindow());

		// Set up ImGui styles
		ImGuiStyles();

		// TODO(): I don't like random raw gl calls just lying around...
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
	}

	//------------------------------------------------------------------------------
	void DeferredRenderer::Update(float dt)
	{
		// Gbuffer pass
		GBufferPass();
		// Lighting pass
		LightingPass();
		// Luminance Pass
		LuminancePass();
		// Bloom pass
		BloomPass();
		// FXAA pass
		FXAAPass(mLightingBuffer);
		// Composite Pass
		CompositePass(mFXAATarget);

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
							mCompositeTarget->GetTexture()
						);
			}
			mBatch->End();
			mBatch->RenderBatch();
		}
		shader->Unuse();

		// ImGui pass
		if (Enjon::Console::Visible())
		{
			GuiPass();
		}

		mWindow.SwapBuffer();
	}

	//------------------------------------------------------------------------------
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
					shader->SetUniform("u_albedoColor", material->GetColor(EG::TextureSlotType::ALBEDO));
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

	//------------------------------------------------------------------------------
	void DeferredRenderer::LightingPass()
	{
		mLightingBuffer->Bind();
		// mFullScreenQuad->Bind();
		
		EG::GLSLProgram* ambientShader 		= EG::ShaderManager::Get("AmbientLight");
		EG::GLSLProgram* directionalShader 	= EG::ShaderManager::Get("PBRDirectionalLight");	
		EG::GLSLProgram* pointShader 		= EG::ShaderManager::Get("PBRPointLight");	
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
			directionalShader->SetUniform("u_camPos", mSceneCamera.GetPosition() + mSceneCamera.Backward());
			for (auto& l : *directionalLights)
			{
				EG::ColorRGBA16* color = l->GetColor();

				directionalShader->BindTexture("u_albedoMap", 		mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO), 0);
				directionalShader->BindTexture("u_normalMap", 		mGbuffer->GetTexture(EG::GBufferTextureType::NORMAL), 1);
				directionalShader->BindTexture("u_positionMap", 	mGbuffer->GetTexture(EG::GBufferTextureType::POSITION), 2);
				directionalShader->BindTexture("u_matProps", 		mGbuffer->GetTexture(EG::GBufferTextureType::MAT_PROPS), 3);
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
			pointShader->BindTexture("u_matProps", mGbuffer->GetTexture(EG::GBufferTextureType::MAT_PROPS), 3);
			pointShader->SetUniform("u_resolution", mGbuffer->GetResolution());
			pointShader->SetUniform("u_camPos", mSceneCamera.GetPosition() + mSceneCamera.Backward());			

			for (auto& l : *pointLights)
			{
				EG::ColorRGBA16& color = l->GetColor();
				EM::Vec3& position = l->GetPosition();

				pointShader->SetUniform("u_lightPos", position);
				pointShader->SetUniform("u_lightColor", EM::Vec3(color.r, color.g, color.b));
				pointShader->SetUniform("u_lightIntensity", l->GetIntensity());
				pointShader->SetUniform("u_attenuationRate", l->GetAttenuationRate());
				pointShader->SetUniform("u_radius", l->GetRadius());

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
				// spotShader->BindTexture("u_matProps", mGbuffer->GetTexture(EG::GBufferTextureType::MAT_PROPS), 3);
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

		// mFullScreenQuad->Unbind();
		mLightingBuffer->Unbind();	

		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	}

	//------------------------------------------------------------------------------
	void DeferredRenderer::LuminancePass()
	{
		GLSLProgram* luminanceProgram = EG::ShaderManager::Get("Bright");
		mLuminanceTarget->Bind();
		{
			mWindow.Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16_Black());
			luminanceProgram->Use();
			{
				luminanceProgram->SetUniform("u_threshold", mToneMapSettings.mThreshold);
				mBatch->Begin();
				{
					mBatch->Add(
										EM::Vec4(-1, -1, 2, 2),
										EM::Vec4(0, 0, 1, 1),
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

	//------------------------------------------------------------------------------
	void DeferredRenderer::BloomPass()
	{
		GLSLProgram* horizontalBlurProgram = EG::ShaderManager::Get("HorizontalBlur");
		GLSLProgram* verticalBlurProgram = EG::ShaderManager::Get("VerticalBlur");

		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_ONE, GL_ONE);

		// Small blur
    	for (u32 i = 0; i < (u32)mBloomSettings.mIterations.x * 2; ++i)
    	{
    		bool isEven = (i % 2 == 0);
    		EG::RenderTarget* target = isEven ? mSmallBlurHorizontal : mSmallBlurVertical;
    		EG::GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind();
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
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1), 
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
    		EG::RenderTarget* target = isEven ? mMediumBlurHorizontal : mMediumBlurVertical;
    		EG::GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind();
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
					GLuint texID = i == 0 ? mLuminanceTarget->GetTexture() : isEven ? mMediumBlurVertical->GetTexture() : mMediumBlurHorizontal->GetTexture();
					mBatch->Begin();
					{
			    		mBatch->Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1), 
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
    		EG::RenderTarget* target = isEven ? mLargeBlurHorizontal : mLargeBlurVertical;
    		EG::GLSLProgram* program = isEven ? horizontalBlurProgram : verticalBlurProgram;

			target->Bind();
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
					GLuint texID = i == 0 ? mLuminanceTarget->GetTexture() : isEven ? mLargeBlurVertical->GetTexture() : mLargeBlurHorizontal->GetTexture();
					mBatch->Begin();
					{
			    		mBatch->Add(
									EM::Vec4(-1, -1, 2, 2),
									EM::Vec4(0, 0, 1, 1), 
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

	//------------------------------------------------------------------------------
	void DeferredRenderer::FXAAPass(EG::RenderTarget* input)
	{
		GLSLProgram* fxaaProgram = EG::ShaderManager::Get("FXAA");
		mFXAATarget->Bind();
		{
			mWindow.Clear();
			fxaaProgram->Use();
			{
				auto viewPort = GetViewport();
				fxaaProgram->SetUniform("u_resolution", EM::Vec2(viewPort.x, viewPort.y));
				fxaaProgram->SetUniform("u_FXAASettings", EM::Vec3(mFXAASettings.mSpanMax, mFXAASettings.mReduceMul, mFXAASettings.mReduceMin));
				mBatch->Begin();
				{
					mBatch->Add(
										EM::Vec4(-1, -1, 2, 2),
										EM::Vec4(0, 0, 1, 1),
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

	//------------------------------------------------------------------------------
	void DeferredRenderer::CompositePass(EG::RenderTarget* input)
	{
		GLSLProgram* compositeProgram = EG::ShaderManager::Get("Composite");
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
										EM::Vec4(-1, -1, 2, 2),
										EM::Vec4(0, 0, 1, 1),
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

	//------------------------------------------------------------------------------
	void DeferredRenderer::GuiPass()
	{
		static bool show_test_window = true;
	    static bool show_another_window = false;

	    // Make a new window
		ImGui_ImplSdlGL3_NewFrame(mWindow.GetSDLWindow());

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            if (ImGui::Button("Test Window")) show_test_window ^= 1;
            if (ImGui::Button("Another Window")) show_another_window ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (show_another_window)
        {
            ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        ImGui::Render();
	}

	//------------------------------------------------------------------------------
	void DeferredRenderer::SetViewport(EM::iVec2& dimensions)
	{
		mWindow.SetViewport(dimensions);
	}

	//------------------------------------------------------------------------------
	EM::iVec2 DeferredRenderer::GetViewport()
	{
		return mWindow.GetViewport();
	}

	//------------------------------------------------------------------------------
	double NormalPDF(double x, double s, double m = 0.0)
	{
		static const double inv_sqrt_2pi = 0.3989422804014327;
		double a = (x - m) / s;

		return inv_sqrt_2pi / s * std::exp(-0.5 * a * a);
	}

	//------------------------------------------------------------------------------
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
		mCompositeTarget 			= new EG::RenderTarget(width, height);
		mLightingBuffer 			= new EG::RenderTarget(width, height);
		mLuminanceTarget 			= new EG::RenderTarget(width / 4, height / 4);
		mFXAATarget 				= new EG::RenderTarget(width, height);
		mShadowDepth 				= new EG::RenderTarget(2048, 2048);

		mBatch 						= new EG::SpriteBatch();
		mBatch->Init();

		mFullScreenQuad 			= new EG::FullScreenQuad();
	}

	//------------------------------------------------------------------------------
	void DeferredRenderer::CalculateBlurWeights()
	{
		double weight;
		double start = -3.0;
		double end = 3.0;
		double denom = 2.0 * end + 1.0;
		double num_samples = 15.0;
		double range = end * 2.0;
		double step = range / num_samples;
		u32 i = 0;

		weight = 1.74;
		for (double x = start; x <= end; x += step)
		{
			double pdf = NormalPDF(x, 0.23);
			mBloomSettings.mSmallGaussianCurve[i++] = pdf;
		}

		i = 0;
		weight = 3.9f;
		for (double x = start; x <= end; x += step)
		{
			double pdf = NormalPDF(x, 0.775);
			mBloomSettings.mMediumGaussianCurve[i++]= pdf;
		}

		i = 0;
		weight = 2.53f;
		for (double x = start; x <= end; x += step)
		{
			double pdf = NormalPDF(x, 1.0);
			mBloomSettings.mLargeGaussianCurve[i++] = pdf;
		}
	}

	//------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------
	void DeferredRenderer::ImGuiStyles()
	{
		// Grab reference to style
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
        style.FrameRounding = 3.0f;
        style.Colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
        style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_Border]                = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
        style.Colors[ImGuiCol_BorderShadow]          = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        style.Colors[ImGuiCol_FrameBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
        style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
        style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
        style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
        style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
        style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
        style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
        style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
        style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

        // Dark style 
        for (int i = 0; i <= ImGuiCol_COUNT; i++)
        {
            ImVec4& col = style.Colors[i];
            float H, S, V;
            ImGui::ColorConvertRGBtoHSV( col.x, col.y, col.z, H, S, V );

            if( S < 0.1f )
            {
               V = 1.0f - V;
            }
            ImGui::ColorConvertHSVtoRGB( H, S, V, col.x, col.y, col.z );
            if( col.w < 1.00f )
            {
                col.w *= 1.0f;
            }
        }
	}

}}


















