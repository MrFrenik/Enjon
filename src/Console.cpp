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

	struct OutputMessage
	{
		OutputMessage(){}
		OutputMessage(std::string _Text, EG::ColorRGBA16 Color = EG::RGBA16_LightGrey())
		{
			Text = _Text;
			TextColor = Color;
		}

		std::string Text;
		EG::ColorRGBA16 TextColor;
	};

	GUI::GUITextBox Console::InputTextBox;
	std::vector<OutputMessage> Output;
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

		InputTextBox.ProcessInput(Input, nullptr);

		return true;
	}

	void Console::Draw()
	{
		Shader->Use();
		{
			Shader->SetUniform("view", HUDCamera.GetCameraMatrix());

			glDisable(GL_DEPTH_TEST);

			Batch.Begin();
			{
				Batch.Add(
								EM::Vec4(-ScreenWidth * 0.5f, -ScreenHeight * 0.5f, ScreenWidth, ScreenHeight),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../Assets/Textures/Default.png").id,
								EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.8f)
							);
			}
			Batch.End();
			Batch.RenderBatch();

			Batch.Begin(EG::GlyphSortType::FRONT_TO_BACK);
			{
				// Render the console input text box
				InputTextBox.Draw(&Batch);

				// Render the text in the console output
				auto StartPosition = EM::Vec2(-ScreenWidth * 0.5f + 20.0f, -ScreenHeight * 0.5f + InputTextBox.Dimensions.y + 20.0f);
				auto YOffset = 30.0f;
				for (auto C = Output.rbegin(); C != Output.rend(); C++)
				{
				 	EG::Fonts::PrintText(StartPosition.x, StartPosition.y + YOffset, 1.0f, (*C).Text, InputTextBox.TextFont, Batch, (*C).TextColor);
				 	YOffset += 20.0f;
				}
			}
			Batch.End();
			Batch.RenderBatch();

			glEnable(GL_DEPTH_TEST);
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
		InputTextBox.Position = EM::Vec2(-ScreenWidth * 0.5f + 5.0f, -ScreenHeight * 0.5f + 20.0f);
		InputTextBox.Dimensions = EM::Vec2(ScreenWidth - 10.0f, InputTextBox.Dimensions.y);
		InputTextBox.MaxStringLength = 100;
		InputTextBox.KeyboardInFocus = true;

		InputTextBox.on_enter.connect([&]()
		{
			// Parse the string
			auto& C = InputTextBox.Text;

			// Push back into history
			InputTextBox.History.push_back(C);

			InputTextBox.HistoryIndex = InputTextBox.History.size();
			// InputTextBox.HistoryIndex = InputTextBox.HistoryIndex == 0 ? 0 : InputTextBox.HistoryIndex + 1;
			printf("%d\n", InputTextBox.HistoryIndex);

			// Split the string into a vector on spaces
			std::vector<std::string> Elements = EU::split(C, ' ');

			if (!Elements.size()) return;

			// Check first element for function call (totally a test)
			if (Elements.at(0).compare("mul") == 0) 
			{
				// Error
				if (Elements.size() < 3) Output.emplace_back("Error: mul requires 2 arguments", EG::RGBA16_Red());

				// Calculate the result
				else
				{
					if (!EU::is_numeric(Elements.at(1)) || !EU::is_numeric(Elements.at(2))) 
					{
						Output.emplace_back("Error: cannot multiply non numeric terms: " + Elements.at(1) + ", " + Elements.at(2), EG::RGBA16_Red());
					}

					// Otherwise, multiply and push back result
					else
					{
						Output.emplace_back(std::to_string(std::atof(Elements.at(1).c_str()) * std::atof(Elements.at(2).c_str())));
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
				Output.emplace_back("Console Variables Available: ", EG::RGBA16_Yellow());
				for (auto& c : registeredCommands) Output.emplace_back(c);
			}
			else
			{
				// Register command with CVar System
				if (Elements.size() < 2) Output.emplace_back("Error: need argument for cvar \"" + Elements.at(0) + "\"", EG::RGBA16_Red());
				else
				{
					if (!CVarsSystem::Set(Elements.at(0), std::atof(Elements.at(1).c_str())))
					{
						Output.emplace_back("Error: cvar does not exist: " + Elements.at(0), EG::RGBA16_Red());
					}
					else
					{
						Output.emplace_back("Set " + Elements.at(0) + ": " + Elements.at(1));
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