/*-- Enjon includes --*/
#include "Enjon.h"
#include "IO/InputManager.h"
#include "IO/ResourceManager.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/FontManager.h"
#include "Graphics/SpriteBatch.h"
#include "Graphics/CursorManager.h"
#include "GUI/GUIGroup.h"
#include "Physics/AABB.h"
#include "Utils/Errors.h"
#include "Defines.h"

/*-- IsoARPG includes --*/
#include "EnjonAnimation.h"
#include "AnimationManager.h"
#include "AnimManager.h"
#include "Level.h"

/*-- Standard Library includes --*/
#include <unordered_map>
#include <vector>
#include <string>

/*-- 3rd Party Includes --*/
#include <SDL2/SDL.h>

using namespace EA;
using json = nlohmann::json;

const Enjon::uint32 NUM_LIGHTS = 1;

namespace Enjon { namespace LightEditor {

	typedef struct
	{
		EM::Vec3 Position;
		EG::ColorRGBA16 Color;
		float Radius;
		EM::Vec3 Falloff;
	} Light;

	using namespace GUI;
	using namespace Graphics;

	/*-- Function Delcarations --*/
	bool ProcessInput(EI::InputManager* Input);

	EI::InputManager* Input = nullptr;

	////////////////////////////
	// LIGHT EDITOR ////////////
	
	float SCREENWIDTH;
	float SCREENHEIGHT;

	bool IsRunning = true;

	// Shaders	
	EG::GLSLProgram* BasicShader 	= nullptr;
	EG::GLSLProgram* TextShader 	= nullptr;
	EG::GLSLProgram* DeferredShader = nullptr;
	EG::GLSLProgram* DiffuseShader 	= nullptr;
	EG::GLSLProgram* NormalsShader 	= nullptr;
	EG::GLSLProgram* DepthShader 	= nullptr;
	EG::GLSLProgram* ScreenShader 	= nullptr;

	// SpriteBatches
	EG::SpriteBatch UIBatch;
	EG::SpriteBatch SceneBatch;
	EG::SpriteBatch BGBatch;
	EG::SpriteBatch DiffuseBatch;
	EG::SpriteBatch NormalsBatch;
	EG::SpriteBatch DepthBatch;
	EG::SpriteBatch DeferredBatch;

	EG::FrameBufferObject* DiffuseFBO 	= nullptr;
	EG::FrameBufferObject* NormalsFBO 	= nullptr;
	EG::FrameBufferObject* DepthFBO 	= nullptr;
	EG::FrameBufferObject* DeferredFBO 	= nullptr;
	
	// Cameras	
	EG::Camera2D Camera;
	EG::Camera2D HUDCamera;

	EG::Window* Window = nullptr;

	EM::Vec2 CameraScaleVelocity;

	GLfloat quadVertices[] = {   // Vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // Positions   // TexCoords

        -1.0f,  1.0f,  0.0f, 1.0f,   	// TL
        -1.0f, -1.0f,  0.0f, 0.0f,   	// BL
         1.0f, -1.0f,  1.0f, 0.0f,		// BR

        -1.0f,  1.0f,  0.0f, 1.0f,		// TL
         1.0f, -1.0f,  1.0f, 0.0f,		// BR
         1.0f,  1.0f,  1.0f, 1.0f 		// TR
    };

    GLuint quadVAO, quadVBO;

    // Vector of lights
	std::vector<Light> Lights;

	/*-- Function Definitions --*/
	bool Init(EI::InputManager* _Input, EG::Window* _Window, float SW, float SH, EG::FrameBufferObject* diffuse_FBO, EG::FrameBufferObject* normals_FBO, EG::FrameBufferObject* depth_FBO, EG::FrameBufferObject* deferred_FBO)
	{
		// Shader for frame buffer
		BasicShader			= EG::ShaderManager::GetShader("Basic");
		TextShader			= EG::ShaderManager::GetShader("Text");  
		DeferredShader 		= EG::ShaderManager::GetShader("DeferredShader");
		DiffuseShader 		= EG::ShaderManager::GetShader("DiffuseShader");
		NormalsShader 		= EG::ShaderManager::GetShader("NormalsShader");
		DepthShader 		= EG::ShaderManager::GetShader("DepthShader");
		ScreenShader 		= EG::ShaderManager::GetShader("NoCameraProjection");

		// // FBO's
		DiffuseFBO 	= new EG::FrameBufferObject(SW, SH);
		NormalsFBO 	= normals_FBO;
		DepthFBO 	= depth_FBO;
		DeferredFBO = deferred_FBO;

		// UI Batch
		UIBatch.Init();
		SceneBatch.Init();
		BGBatch.Init();
		DiffuseBatch.Init();
		NormalsBatch.Init();
		DepthBatch.Init();
		DeferredBatch.Init();

		SCREENWIDTH = SW;
		SCREENHEIGHT = SH;

		// Create Camera
		Camera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Create HUDCamera
		HUDCamera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Set input
		Input = _Input;

		// Set window
		Window = _Window;

		// Generate OpenGL stuff
	    glGenVertexArrays(1, &quadVAO);
	    glGenBuffers(1, &quadVBO);
	    glBindVertexArray(quadVAO);
	    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	    glEnableVertexAttribArray(0);
	    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	    glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
	    glBindVertexArray(0);

	    // Set up lights
		const GLfloat constant = 1.0f; 
	    const GLfloat linear = 0.1f;
	    const GLfloat quadratic = 5.0f;

		for (GLuint i = 0; i < NUM_LIGHTS; i++)
		{
			EG::ColorRGBA16 Color = EG::RGBA16(0.6f, 0.7f, ER::Roll(0, 255) / 255.0f, 1.0f);
		    GLfloat maxBrightness = std::fmaxf(std::fmaxf(Color.r, Color.g), Color.b);  // max(max(lightcolor.r, lightcolor.g), lightcolor.b)
		    GLfloat Radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2 * quadratic);
			Light L = {
						  EM::Vec3(0.0f, 0.0f, 0.0f), 
						  Color, 
						  Radius, 
						  EM::Vec3(constant, linear, quadratic)
					  };

			Lights.push_back(L);
	}
		
		return true;	
	}

	bool Update()
	{
		// Check for quit condition
		IsRunning = ProcessInput(Input);

		// Update input
		Input->Update();

		// Update cameras
		Camera.Update();
		HUDCamera.Update();

		auto MP = Input->GetMouseCoords();
		Camera.ConvertScreenToWorld(MP);
		Lights.at(0).Position = EM::Vec3(MP, Lights.at(0).Position.z);

		return IsRunning;
	}		

	bool Draw()
	{
		// Set up necessary matricies
    	auto Model 		= EM::Mat4::Identity();	
    	auto View 		= HUDCamera.GetCameraMatrix();
    	auto Projection = EM::Mat4::Identity();

    	// Draw some shit
    	{
    		SceneBatch.Begin();
    		NormalsBatch.Begin();

    		SceneBatch.Add(
    						EM::Vec4(-SCREENWIDTH / 2.0f, -SCREENHEIGHT / 2.0f, SCREENWIDTH, SCREENHEIGHT),
    						EM::Vec4(0, 0, 1, 1),
    						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
    						EG::RGBA16(0.06f, 0.06f, 0.06f, 1.0f)
    					);

    		// Draw box diffuse
    		SceneBatch.Add(
    						EM::Vec4(0.0f, 0.0f, 118.0f, 117.0f),
    						EM::Vec4(0, 0, 1, 1),
    						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box.png").id
    					);

    		// Draw box normal
    		NormalsBatch.Add(
	    						EM::Vec4(0.0f, 0.0f, 118.0f, 117.0f),
	    						EM::Vec4(0, 0, 1, 1),
	    						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/box_normal.png").id
    						);

    		SceneBatch.End();
    		NormalsBatch.End();
    	}

    	// FBO's
    	{
			DiffuseFBO->Bind();
			{
				DiffuseShader->Use();
				{
					// Set up uniforms
					DiffuseShader->SetUniform("model", Model);
					DiffuseShader->SetUniform("view", View);
					DiffuseShader->SetUniform("projection", Projection);

					// Draw entities		
					SceneBatch.RenderBatch();
				}
				DiffuseShader->Unuse();
			}
			DiffuseFBO->Unbind();

			// Normals Rendering
			NormalsFBO->Bind();
			{
				NormalsShader->Use();
				{
					// Set up uniforms
					NormalsShader->SetUniform("model", Model);
					NormalsShader->SetUniform("view", View);
					NormalsShader->SetUniform("projection", Projection);

					NormalsBatch.RenderBatch();
				}
				NormalsShader->Unuse();
			}
			NormalsFBO->Unbind();

			// Depth Rendering
			DepthFBO->Bind();
			{
				DepthShader->Use();
				{
					// Set up uniforms
					DepthShader->SetUniform("model", Model);
					DepthShader->SetUniform("view", View);
					DepthShader->SetUniform("projection", Projection);

					DepthBatch.RenderBatch();
				}
				DepthShader->Unuse();
			}
			DepthFBO->Unbind();

			// Deferred Render
			glDisable(GL_DEPTH_TEST);
			glBlendFunc(GL_ONE, GL_ONE);
			DeferredFBO->Bind();
			{
				DeferredShader->Use();
				{
					GLuint m_diffuseID 	= glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_diffuse");
					GLuint m_normalsID  = glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_normals");
					GLuint m_positionID = glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_position");
					GLuint m_depthID  	= glGetUniformLocationARB(DeferredShader->GetProgramID(),"u_depth");

					EM::Vec3 CP = EM::Vec3(Camera.GetPosition(), 1.0f);

					// Bind diffuse
					glActiveTexture(GL_TEXTURE0);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, DiffuseFBO->GetNormalsTexture());
					glUniform1i(m_diffuseID, 0);

					// Bind normals
					glActiveTexture(GL_TEXTURE1);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, NormalsFBO->GetNormalsTexture());
					glUniform1i(m_normalsID, 1);

					// Bind position
					glActiveTexture(GL_TEXTURE2);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, DiffuseFBO->GetPositionTexture());
					glUniform1i(m_positionID, 2);

					// Bind depth
					glActiveTexture(GL_TEXTURE3);
					glEnable(GL_TEXTURE_2D);
					glBindTexture(GL_TEXTURE_2D, DepthFBO->GetDiffuseTexture());
					glUniform1i(m_depthID, 3);

					glUniform1i(glGetUniformLocation(DeferredShader->GetProgramID(), "NumberOfLights"), Lights.size());

					auto CameraScale = Camera.GetScale();
					for (GLuint i = 0; i < Lights.size(); i++)
					{
						auto L = &Lights.at(i);

						glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Position").c_str()), L->Position.x, L->Position.y, L->Position.z);
						glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Color").c_str()), L->Color.r, L->Color.g, L->Color.b, L->Color.a);
						glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Radius").c_str()), L->Radius / CameraScale);
						glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Falloff").c_str()), L->Falloff.x, L->Falloff.y, L->Falloff.z);
						glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), ("Lights[" + std::to_string(i) + "].Depth").c_str()), L->Position.y);
					}

					// Set uniforms
					glUniform2f(glGetUniformLocation(DeferredShader->GetProgramID(), "Resolution"), SCREENWIDTH, SCREENHEIGHT);
					glUniform4f(glGetUniformLocation(DeferredShader->GetProgramID(), "AmbientColor"), 1.0f, 1.0f, 1.0f, 0.1f);
					glUniform3f(glGetUniformLocation(DeferredShader->GetProgramID(), "ViewPos"), CP.x, CP.y, CP.z);
					glUniformMatrix4fv(glGetUniformLocation(DeferredShader->GetProgramID(), "InverseCameraMatrix"), 1, 0, Camera.GetCameraMatrix().Invert().elements);
					glUniformMatrix4fv(glGetUniformLocation(DeferredShader->GetProgramID(), "View"), 1, 0, Camera.GetCameraMatrix().elements);
					glUniform1f(glGetUniformLocation(DeferredShader->GetProgramID(), "Scale"), Camera.GetScale());


					// Render	
					{
						glBindVertexArray(quadVAO);
						glDrawArrays(GL_TRIANGLES, 0, 6);
						glBindVertexArray(0);

						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, 0);
					}
				}
				DeferredShader->Unuse();
			}
			DeferredFBO->Unbind();
    	}

		// Set blend function back 
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Bind default buffer and render deferred render texture
		Window->Clear(1.0f, GL_COLOR_BUFFER_BIT, EG::RGBA16(0.0f, 0.0f, 0.0f, 0.0f));
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Draw render target to screen
		ScreenShader->Use();
		{
			DeferredBatch.Begin();
			{
				DeferredBatch.Add(
					EM::Vec4(-1, -1, 2, 2),
					EM::Vec4(0, 0, 1, 1), 
					DeferredFBO->GetDiffuseTexture()
					);
			}
			DeferredBatch.End();
			DeferredBatch.RenderBatch();
		}
		ScreenShader->Unuse();

		return true;
	}

	bool ProcessInput(EI::InputManager* Input)
	{
	    SDL_Event event;
	    while (SDL_PollEvent(&event)) 
	    {
	        switch (event.type) 
	        {
	            case SDL_QUIT:
	                return false;
	                break;
				case SDL_KEYUP:
					Input->ReleaseKey(event.key.keysym.sym); 
					break;
				case SDL_KEYDOWN:
					Input->PressKey(event.key.keysym.sym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					Input->PressKey(event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					Input->ReleaseKey(event.button.button);
					break;
				case SDL_MOUSEMOTION:
					Input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
					break;
				case SDL_MOUSEWHEEL:
					Lights.at(0).Position.z += event.wheel.y;
					std::cout << "Light Position: " << Lights.at(0).Position << std::endl;
				default:
					break;
			}
	    }

	    if (Input->IsKeyPressed(SDLK_ESCAPE))
	    {
	    	return false;
	    }

	    if (Input->IsKeyPressed(SDLK_m))
	    {
	    	Lights.at(0).Falloff.z += 1.0f;
	    }

	    if (Input->IsKeyPressed(SDLK_n))
	    {
	    	Lights.at(0).Falloff.z -= 1.0f;
	    }

		return true;
	}
}}
