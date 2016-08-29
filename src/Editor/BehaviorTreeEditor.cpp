/*-- Enjon includes --*/
#include "Enjon.h"
#include "Editor/BehaviorTreeEditor.h"
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

void PrintJSONChild(json& Object)
{
	for (auto it = Object.begin(); it != Object.end(); ++it)
	{
		std::cout << it.key() << std::endl;
	
		if (it.value().is_object())
		{
			PrintJSONChild(it.value());
		}
	}
}

namespace Enjon { namespace BehaviorTreeEditor {

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

	// SpriteBatches
	EG::SpriteBatch UIBatch;
	EG::SpriteBatch SceneBatch;
	
	// Cameras	
	EG::Camera2D Camera;
	EG::Camera2D HUDCamera;

	EM::Vec2 CameraScaleVelocity;

	/*-- Function Definitions --*/
	bool Init(EI::InputManager* _Input, float SW, float SH)
	{
		// Shader for frame buffer
		BasicShader			= EG::ShaderManager::GetShader("Basic");
		TextShader			= EG::ShaderManager::GetShader("Text");

		// UI Batch
		UIBatch.Init();
		SceneBatch.Init();

		SCREENWIDTH = SW;
		SCREENHEIGHT = SH;

		// Create Camera
		Camera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Create HUDCamera
		HUDCamera.Init(SCREENWIDTH, SCREENHEIGHT);

		// Set input
		Input = _Input;

		auto Json = EU::read_file_sstream("../IsoARPG/Profiles/Behaviors/TestTree.json");
	    
	   	// parse and serialize JSON
	   	json j_complete = json::parse(Json);

	   	// Get handle to frames data
	   	PrintJSONChild(j_complete);

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
    		UIBatch.Begin();
    		{

    		}
    		SceneBatch.End();
    		UIBatch.End();
    	}

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
				default:
					break;
			}
	    }

	    if (Input->IsKeyPressed(SDLK_ESCAPE))
	    {
	    	return false;
	    }

		return true;
	}
}}
