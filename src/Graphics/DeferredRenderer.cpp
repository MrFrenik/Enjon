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
#include "ImGui/ImGuiManager.h"

#include "ImGui/ImGuiDock.h"

#include <cassert>

namespace Enjon { namespace Graphics {

static void ShowExampleAppLayout(bool* p_open);

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

		// Final target
		mFinalTarget->Bind();
		{
			mWindow.Clear();

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
		}
		mFinalTarget->Unbind();

		// Clear default buffer
		mWindow.Clear();

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
		static bool show_test_window = false;
		static bool show_frame_rate = false;
		static bool show_graphics_window = true;
		static bool show_app_layout = false;
		static bool show_game_viewport = true;

	    // Make a new window
		ImGui_ImplSdlGL3_NewFrame(mWindow.GetSDLWindow());

	    // Menu
	    ImGui::BeginMainMenuBar();
	    {
	        if (ImGui::BeginMenu("File"))
	        {
	            ImGui::MenuItem("Frame Rate", NULL, &show_frame_rate);
	            ImGui::EndMenu();
	        }
	        if (ImGui::BeginMenu("Edit"))
	        {
	            ImGui::EndMenu();
	        }
	        if (ImGui::BeginMenu("Scene"))
	        {
	            ImGui::EndMenu();
	        }
	        if (ImGui::BeginMenu("View"))
	        {
	        	ImGui::MenuItem("Graphics", NULL, &show_graphics_window);
	        	ImGui::MenuItem("Viewport##game", NULL, &show_game_viewport);
	            ImGui::EndMenu();
	        }
	        if (ImGui::BeginMenu("Help"))
	        {
	        	ImGui::MenuItem("TestWindow", NULL, &show_test_window);
	        	ImGui::MenuItem("App Layout", NULL, &show_app_layout);
	            ImGui::EndMenu();
	        }

        	ImGuiManager::Render();

	       	// Frame time
            ImGui::SameLine(ImGui::GetWindowWidth() - 90.0f);
            ImGui::Text("FPS: ");

            f32 fps = ImGui::GetIO().Framerate;
            ImColor color;

            if (fps >= 60.0f) color = ImColor(0.1f, 1.0f, 0.0f, 1.0f);
            else if (fps > 30.0f) color = ImColor(1.00f, 0.41f, 0.00f, 1.00f);
            else color = ImColor(1.0f, 0.0f, 0.0f, 1.0f);

            ImGui::SameLine();
	       	ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("%.1f", ImGui::GetIO().Framerate);
            ImGui::PopStyleColor(1);

	        ImGui::EndMainMenuBar();
	    }

	    // Frame rate
        if (show_frame_rate)
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // Render game
        if (show_game_viewport)
        {
	        ShowGameViewport(&show_game_viewport, !show_graphics_window);
        }

        if (show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(450, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        // Show graphics options
        if (show_graphics_window)
        {
            ShowGraphicsWindow(&show_graphics_window);
        }

        if (show_app_layout)
        {
        	ShowExampleAppLayout(&show_app_layout);
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
		mFinalTarget 				= new EG::RenderTarget(width, height);

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
		ImGuiIO& io = ImGui::GetIO();

		io.Fonts->Clear();
		io.Fonts->AddFontFromFileTTF("../Assets/Fonts/WeblySleek/weblysleekuisb.ttf", 16);
		io.Fonts->AddFontFromFileTTF("../Assets/Fonts/WeblySleek/weblysleekuisb.ttf", 14);
		io.Fonts->Build();

		// Grab reference to style
		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowPadding            = ImVec2(3, 7);
		style.WindowRounding           = 5.0f;
		style.FramePadding             = ImVec2(2, 0);
		style.FrameRounding            = 2.0f;
		style.ItemSpacing              = ImVec2(8, 4);
		style.ItemInnerSpacing         = ImVec2(2, 2);
		style.IndentSpacing            = 21.0f;
		style.ScrollbarSize            = 11.0f;
		style.ScrollbarRounding        = 9.0f;
		style.GrabMinSize              = 4.0f;
		style.GrabRounding             = 3.0f;
		style.WindowTitleAlign 		   = ImVec2(0.5f, 0.41f);
		style.ButtonTextAlign 		   = ImVec2(0.5f, 0.5f);
		style.Alpha = 1.0f;
        style.FrameRounding = 3.0f;

       	style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_ChildWindowBg]         = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.04f, 0.04f, 0.04f, 0.94f);
		style.Colors[ImGuiCol_Border]                = ImVec4(1.00f, 1.00f, 1.00f, 0.18f);
		style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.10f);
		style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		style.Colors[ImGuiCol_ComboBg]               = ImVec4(0.14f, 0.14f, 0.14f, 0.99f);
		style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_Button]                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.85f);
		style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.06f, 0.53f, 0.98f, 0.63f);
		style.Colors[ImGuiCol_Header]                = ImVec4(0.26f, 0.59f, 0.98f, 0.32f);
		style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.37f);
		style.Colors[ImGuiCol_Column]                = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
		style.Colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
		style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		style.Colors[ImGuiCol_CloseButton]           = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
		style.Colors[ImGuiCol_CloseButtonHovered]    = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_CloseButtonActive]     = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
		style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		style.Colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	void DeferredRenderer::ShowGraphicsWindow(bool* p_open)
	{
	    static bool no_titlebar = true;
	    static bool no_border = false;
	    static bool no_resize = true;
	    static bool no_move = true;
	    static bool no_scrollbar = false;
	    static bool no_collapse = true;
	    static bool no_menu = true;

	    // Demonstrate the various window flags. Typically you would just use the default.
	    ImGuiWindowFlags window_flags = 0;
	    if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
	    if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
	    if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
	    if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
	    if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
	    if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
	    if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;

	    ImGui::SetNextWindowSize(ImVec2(407, 886));
	    ImGui::SetNextWindowPos(ImVec2(1032, 17));
	    if (!ImGui::Begin("Graphics Options", p_open, window_flags))
	    {
	        // Early out if the window is collapsed, as an optimization.
	        ImGui::End();
	        return;
	    }

	    /*
	    auto pos = ImGui::GetWindowPos();
	    printf("W/H: %d %d\n", (u32)ImGui::GetWindowWidth(), (u32)ImGui::GetWindowHeight());
	    printf("X/Y: %d %d\n", (u32)pos.x, (u32)pos.y);
	    */

	    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);                                 // Right align, keep 140 pixels for labels

        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(1.0, 0.6f, 0.0f, 1.0f));
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
			    ImGui::SliderFloat("Threshold##bloom", &mToneMapSettings.mThreshold, 0.00f, 4.0f, "%.2f");

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
		    ImGui::SliderFloat("SpanMax##fxaa", &mFXAASettings.mSpanMax, 0.001, 10.0f, "%.3f");
		    ImGui::SliderFloat("ReduceMul##fxaa", &mFXAASettings.mReduceMul, 0.001, 10.0f, "%.3f");
		    ImGui::SliderFloat("ReduceMin##fxaa", &mFXAASettings.mReduceMin, 0.001, 10.0f, "%.3f");

		    ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("FrameBuffers"))
	    {
	    	ImFontAtlas* atlas = ImGui::GetIO().Fonts;
	    	ImGui::PushFont(atlas->Fonts[1]);
	        ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0.2, 0.6f, 0.6f, 1.0f));
	    	for (u32 i = 0; i < (u32)EG::GBufferTextureType::GBUFFER_TEXTURE_COUNT; ++i)
	    	{
	    		const char* string_name = mGbuffer->FrameBufferToString(i);
	    		// if (ImGui::TreeNode(string_name))
	    		// {
				   //  ImTextureID img = (ImTextureID)mGbuffer->GetTexture(i);
				   //  ImGui::Image(img, ImVec2(200, 200), ImVec2(0,1), ImVec2(1,0), ImColor(255,255,255,255), ImColor(255,255,255,128));
				   //  ImGui::TreePop();
	    		// }

	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mGbuffer->GetTexture(i);
			    ImGui::Image(img, ImVec2(200, 200), ImVec2(0,1), ImVec2(1,0), ImColor(255,255,255,255), ImColor(255,255,255,128));
	    	}
	    	ImGui::PopStyleColor(1);
	    	ImGui::PopFont();
	    	ImGui::TreePop();
	    }


	    ImGui::End();
	}

	//-----------------------------------------------------------------------------------------------------
	void DeferredRenderer::ShowGameViewport(bool* open, bool fullscreen)
	{
		static bool no_titlebar = true;
	    static bool no_border = false;
	    static bool no_resize = false;
	    static bool no_move = true;
	    static bool no_scrollbar = true;
	    static bool no_collapse = true;
	    static bool no_menu = true;

	    // Demonstrate the various window flags. Typically you would just use the default.
	    ImGuiWindowFlags window_flags = 0;
	    if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
	    if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
	    if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
	    if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
	    if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
	    if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
	    if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;

	    if (!fullscreen) ImGui::SetNextWindowSize(ImVec2(1028, 883));
	    else ImGui::SetNextWindowSize(ImVec2(1440, 900));
	    ImGui::SetNextWindowPos(ImVec2(1, 18));
	    if (!ImGui::Begin("Viewport", open, window_flags))
	    {
	        // Early out if the window is collapsed, as an optimization.
	        ImGui::End();
	        return;
	    }

	    // Render game in window
	    ImTextureID img = (ImTextureID)mFinalTarget->GetTexture();
	    ImGui::Image(img, ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), 
	    				ImVec2(0,1), ImVec2(1,0), ImColor(255,255,255,255), ImColor(255,255,255,0));


	    /*
	    auto pos = ImGui::GetWindowPos();
	    printf("W/H: %d %d\n", (u32)ImGui::GetWindowWidth(), (u32)ImGui::GetWindowHeight());
	    printf("X/Y: %d %d\n", (u32)pos.x, (u32)pos.y);
	    */

	    // End the window
	    ImGui::End();
	}


	static void ShowExampleAppLayout(bool* p_open)
	{
	    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiSetCond_FirstUseEver);
	    if (ImGui::Begin("Example: Layout", p_open, ImGuiWindowFlags_MenuBar))
	    {
	        if (ImGui::BeginMenuBar())
	        {
	            if (ImGui::BeginMenu("File"))
	            {
	                if (ImGui::MenuItem("Close")) *p_open = false;
	                ImGui::EndMenu();
	            }
	            ImGui::EndMenuBar();
	        }

	        // left
	        static int selected = 0;
	        ImGui::BeginChild("left pane", ImVec2(150, 0), true);
	        for (int i = 0; i < 100; i++)
	        {
	            char label[128];
	            sprintf(label, "MyObject %d", i);
	            if (ImGui::Selectable(label, selected == i))
	                selected = i;
	        }
	        ImGui::EndChild();
	        ImGui::SameLine();

	        // right
	        ImGui::BeginGroup();
	            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing())); // Leave room for 1 line below us
	                ImGui::Text("MyObject: %d", selected);
	                ImGui::Separator();
	                ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
	            ImGui::EndChild();
	            ImGui::BeginChild("buttons");
	                if (ImGui::Button("Revert")) {}
	                ImGui::SameLine();
	                if (ImGui::Button("Save")) {}
	            ImGui::EndChild();
	        ImGui::EndGroup();
	    }
	    ImGui::End();
	}

}}


















