#include "Console.h"
#include "IO/InputManager.h"
#include "Utils/Functions.h"
#include "CVarsSystem.h"
#include "Graphics/ShaderManager.h"
#include "Graphics/SpriteBatch.h"
#include "IO/ResourceManager.h"
#include "Graphics/Color.h"
#include "Math/Maths.h"
#include "Graphics/Font.h"
#include "Graphics/Camera.h"
#include "Graphics/Camera2D.h"

namespace Enjon {

	std::vector<std::string> Console::Output;
	GUI::GUITextBox Console::InputTextBox;
	EG::SpriteBatch Batch;
	EG::Camera UICamera;
	float ScreenWidth, ScreenHeight;
	EG::GLSLProgram* Shader;

	// Create HUDCamera
	EG::Camera2D HUDCamera;

	void Console::Update(float DT)
	{
		InputTextBox.Update();
		HUDCamera.Update();
	}

	bool Console::ProcessInput(Input::InputManager* Input)
	{
	    SDL_Event event;
	   //Will keep looping until there are no more events to process
	    while (SDL_PollEvent(&event)) {
	        switch (event.type) {
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
				default:
					break;
			}
	    }

		if (Input->IsKeyPressed(SDLK_BACKQUOTE) || Input->IsKeyPressed(SDLK_ESCAPE))
		{
			return false;	
		}

		return true;
	}

	void Console::Draw()
	{
		Shader->Use();
		{
			Shader->SetUniform("view", HUDCamera.GetCameraMatrix());

			Batch.Begin();
			{
				Batch.Add(
								EM::Vec4(-ScreenWidth * 0.5f, -ScreenHeight * 0.5f, ScreenWidth, ScreenHeight),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../Assets/Textures/Default.png").id,
								EG::SetOpacity(EG::RGBA16_LightPurple(), 0.4f)
							);
			}
			Batch.End();
			Batch.RenderBatch();

			Batch.Begin(EG::GlyphSortType::FRONT_TO_BACK);
			Batch.Begin();
			{
				Batch.Add(
						EM::Vec4(EM::Vec2(0, 0), EM::Vec2(100, 0.1)),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						EG::RGBA16_White() 
						// -1.0f
						// EG::SpriteBatch::DrawOptions::BORDER, 
						// BorderColor
					);
				// Render the console input text box
				InputTextBox.Draw(&Batch);

				// Render the text in the console output
				auto StartPosition = EM::Vec2(-ScreenWidth * 0.5f + 20.0f, -ScreenHeight * 0.5f + InputTextBox.Dimensions.y + 20.0f);
				auto YOffset = 30.0f;
				for (auto C = Output.rbegin(); C != Output.rend(); C++)
				{
					 EG::Fonts::PrintText(StartPosition.x, StartPosition.y + YOffset, 1.0f, *C, InputTextBox.TextFont, Batch, EG::RGBA16_LightGrey());
					 YOffset += 20.0f;
				}
			}
			Batch.End();
			Batch.RenderBatch();
		}
		Shader->Unuse();
	}

	void Console::Init(float _ScreenWidth, float _ScreenHeight)
	{
		Batch.Init();

		ScreenWidth = _ScreenWidth;
		ScreenHeight = _ScreenHeight;
		UICamera = EG::Camera(ScreenWidth, ScreenHeight);
		Shader = EG::ShaderManager::GetShader("Text");

		HUDCamera.Init(ScreenWidth, ScreenHeight);
		HUDCamera.SetScale(1.0f);

		// Console text box initialization
//		InputTextBox.Position = EM::Vec2(-ScreenWidth * 0.5f + 5.0f, -ScreenHeight * 0.5f + 20.0f);
		InputTextBox.Position = EM::Vec2(0.0f, 0.0f);
		InputTextBox.Dimensions = EM::Vec2(ScreenWidth - 10.0f, InputTextBox.Dimensions.y);
		InputTextBox.MaxStringLength = 100;
		InputTextBox.KeyboardInFocus = true;

		InputTextBox.on_enter.connect([&]()
		{
			// Parse the string
			auto& C = InputTextBox.Text;

			// Split the string into a vector on spaces
			std::vector<std::string> Elements = EU::split(C, ' ');

			if (!Elements.size()) return;

			// Check first element for function call (totally a test)
			if (Elements.at(0).compare("mul") == 0) 
			{
				// Error
				if (Elements.size() < 3) Output.push_back("mul requires 2 arguments");

				// Calculate the result
				else
				{
					if (!EU::is_numeric(Elements.at(1)) || !EU::is_numeric(Elements.at(2))) 
					{
						Output.push_back("Cannot multiply non numeric terms: " + Elements.at(1) + ", " + Elements.at(2));
					}

					// Otherwise, multiply and push back result
					else
					{
						auto Result = std::atof(Elements.at(1).c_str()) * std::atof(Elements.at(2).c_str());
						Output.push_back(std::to_string(Result));
					}
				}
			}
			// Clear the elements
			else if (Elements.at(0).compare("clear") == 0 || Elements.at(0).compare("cls") == 0)
			{
				Output.clear();
			}
			else if (Elements.at(0).compare("cvarlist") == 0)
			{
				auto registeredCommands = CVarsSystem::GetRegisteredCommands();
				Output.push_back("Console Variables Available:");
				for (auto& c : registeredCommands) Output.push_back(c);
			}
			else
			{
				// Register command with CVar System
				if (Elements.size() < 2) Output.push_back("need argument for cvar");
				else
				{
					if (!CVarsSystem::Set(Elements.at(0), std::atof(Elements.at(1).c_str())))
					{
						Output.push_back("Cvar does not exist: " + Elements.at(0));
					}
					else
					{
						Output.push_back("Set " + Elements.at(0) + ": " + Elements.at(1));
					}
				}
			}
			
			// Clear the console text
			InputTextBox.Clear();	

			// Reset focus
			InputTextBox.KeyboardInFocus = true;
		});
	}
}