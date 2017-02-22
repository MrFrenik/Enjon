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
#include "ImGui/ImGuiManager.h"

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
	}

	//------------------------------------------------------------------------------
	void DeferredRenderer::Init()
	{
		// TODO(John): Need to have a way to have an ini that's read or grab these values from a static
		// engine config file
		// mWindow.Init("Game", 1920, 1080, WindowFlagsMask((u32)WindowFlags::FULLSCREEN)); 
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

		mShowGraphicsOptionsWindow = false;
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
			if (ImGui::BeginDock("Graphics##viewport", &mShowGraphicsOptionsWindow))
			{
				// Print docking information
				ShowGraphicsWindow(&mShowGraphicsOptionsWindow);
			}
			ImGui::EndDock();
		};

		// ImGuiManager::Register(graphicsMenuOption);
		ImGuiManager::RegisterMenuOption("View", graphicsMenuOption);
		ImGuiManager::RegisterWindow(showGameViewportFunc);
		ImGuiManager::RegisterWindow(showGraphicsViewportFunc);

		// Set current render texture
		mCurrentRenderTexture = mCompositeTarget->GetTexture();

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
		if (mFXAASettings.mEnabled) FXAAPass(mLightingBuffer);
		// Composite Pass
		CompositePass(mFXAASettings.mEnabled ? mFXAATarget : mLightingBuffer);

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
		glClearBufferfv(GL_COLOR, 0, mBGColor);

		/*
		GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};
		for (u32 i = 1; i < (u32)EG::GBufferTextureType::GBUFFER_TEXTURE_COUNT; ++i)
		{
			glClearBufferfv(GL_COLOR, i, black);
		}
		*/

		glClear(GL_DEPTH_BUFFER_BIT);

		// mWindow.Clear(1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, EG::RGBA16_LightGrey());

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
				EG::ColorRGBA16 color = l->GetColor();

				directionalShader->BindTexture("u_albedoMap", 		mGbuffer->GetTexture(EG::GBufferTextureType::ALBEDO), 0);
				directionalShader->BindTexture("u_normalMap", 		mGbuffer->GetTexture(EG::GBufferTextureType::NORMAL), 1);
				directionalShader->BindTexture("u_positionMap", 	mGbuffer->GetTexture(EG::GBufferTextureType::POSITION), 2);
				directionalShader->BindTexture("u_matProps", 		mGbuffer->GetTexture(EG::GBufferTextureType::MAT_PROPS), 3);
				// directionalShader->BindTexture("u_shadowMap", 		mShadowDepth->GetDepth(), 4);
				directionalShader->SetUniform("u_resolution", 		mGbuffer->GetResolution());
				// directionalShader->SetUniform("u_lightSpaceMatrix", mShadowCamera->GetViewProjectionMatrix());
				// directionalShader->SetUniform("u_shadowBias", 		EM::Vec2(0.005f, ShadowBiasMax));
				directionalShader->SetUniform("u_lightDirection", 	l->GetDirection());															
				directionalShader->SetUniform("u_lightColor", 		EM::Vec3(color.r, color.g, color.b));
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

        // Render gui
        ImGuiManager::Render(mWindow.GetSDLWindow());

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
	void DeferredRenderer::ShowGraphicsWindow(bool* p_open)
	{
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
		    ImGui::SliderFloat("SpanMax##fxaa", &mFXAASettings.mSpanMax, 0.001, 100.0f, "%.3f");
		    ImGui::SliderFloat("ReduceMul##fxaa", &mFXAASettings.mReduceMul, 0.000001, 0.001f, "%.6f");
		    ImGui::SliderFloat("ReduceMin##fxaa", &mFXAASettings.mReduceMin, 0.000001, 0.001f, "%.6f");

		    bool enabled = (bool)mFXAASettings.mEnabled;
            ImGui::Checkbox("Enabled##fxaa", &enabled);
            mFXAASettings.mEnabled = (u32)enabled;

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
	    		ImGui::Text(string_name);
			    ImTextureID img = (ImTextureID)mGbuffer->GetTexture(i);

                if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
                {
			        mCurrentRenderTexture = mGbuffer->GetTexture(i); 
                }
	    	}

    		const char* string_name = "Final";
    		ImGui::Text(string_name);
		    ImTextureID img = (ImTextureID)mCompositeTarget->GetTexture();
            if (ImGui::ImageButton(img, ImVec2(64, 64), ImVec2(0,1), ImVec2(1, 0), 1, ImVec4(0,0,0,0), ImColor(255,255,255,255)))
            {
		        mCurrentRenderTexture = mCompositeTarget->GetTexture(); 
            }

	    	ImGui::PopStyleColor(1);
	    	ImGui::PopFont();
	    	ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("Sunlight"))
	    {
	    	static const char* labels[] = {"X", "Y", "Z"};
            static float vec4f[4] = { 0.10f, 0.20f, 0.30f, 0.44f };
	    	EM::Vec3 direction = mScene.GetSun()->GetDirection();
	    	ImGui::Text("Direction");
            ImGui::DragFloat3Labels("##sundir", labels, vec4f, 0.001f, -1.0f, 1.0f);
	    	mScene.GetSun()->SetDirection(EM::Vec3(vec4f[0], vec4f[1], vec4f[2]));

	    	float intensity = mScene.GetSun()->GetIntensity();
	    	ImGui::DragFloat("Intensity", &intensity, 0.01f, 0.0f, 50.0f);
	    	mScene.GetSun()->SetIntensity(intensity);

	    	ImGui::TreePop();
	    }

	    if (ImGui::TreeNode("Background"))
	    {
	    	static const char* labels[] = {"R", "G", "B"};
	    	ImGui::Text("Color");
            ImGui::DragFloat3Labels("##bgcolor", labels, mBGColor, 0.001f, 0.0f, 1.0f);
	    	ImGui::TreePop();
	    }
	}

	//-----------------------------------------------------------------------------------------------------
	void DeferredRenderer::ShowGameViewport(bool* open)
	{
	    // Render game in window
	    ImTextureID img = (ImTextureID)mCurrentRenderTexture;
	    ImGui::Image(img, ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), 
	    				ImVec2(0,1), ImVec2(1,0), ImColor(255,255,255,255), ImColor(255,255,255,0));

	}

}}


















