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
		OutputMessage(std::string _Text, ColorRGBA16 Color = RGBA16_LightGrey())
		{
			Text = _Text;
			TextColor = Color;
		}

		std::string Text;
		ColorRGBA16 TextColor;
	};

	bool Console::mIsVisible = false;
	GUI::GUITextBox Console::InputTextBox;
	std::vector<OutputMessage> Output;
	SpriteBatch Batch;
	Camera UICamera;
	float ScreenWidth, ScreenHeight;
	GLSLProgram* Shader;

	// Create HUDCamera
	Camera2D HUDCamera;

	void Console::Update(float DT)
	{
		InputTextBox.Update();
		HUDCamera.Update();
	}

	bool Console::ProcessInput(Input* input)
	{
	    SDL_Event event;
	   //Will keep looping until there are no more events to process
	    while (SDL_PollEvent(&event)) {
	        switch (event.type) {
	            case SDL_QUIT:
	                return false;
	                break;
				case SDL_KEYUP:
					input->ReleaseKey(event.key.keysym.sym); 
					break;
				case SDL_KEYDOWN:
					input->PressKey(event.key.keysym.sym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					input->PressKey(event.button.button);
					break;
				case SDL_MOUSEBUTTONUP:
					input->ReleaseKey(event.button.button);
					break;
				case SDL_MOUSEMOTION:
					input->SetMouseCoords((float)event.motion.x, (float)event.motion.y);
					break;
				default:
					break;
			}
	    }

		if (input->IsKeyPressed(SDLK_BACKQUOTE) || input->IsKeyPressed(SDLK_ESCAPE))
		{
			mIsVisible = false;
			return false;	
		}

		InputTextBox.ProcessInput(input, nullptr);

		return true;
	}

	bool Console::Visible()
	{
		return mIsVisible;
	}

	void Console::Visible(bool visible)
	{
		mIsVisible = visible;
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
								Vec4(-ScreenWidth * 0.5f, -ScreenHeight * 0.5f, ScreenWidth / 3, ScreenHeight / 4),
								Vec4(0, 0, 1, 1),
								Enjon::ResourceManager::GetTexture("../Assets/Textures/Default.png").id,
								SetOpacity(RGBA16_DarkGrey(), 0.8f)
							);
			}
			Batch.End();
			Batch.RenderBatch();

			Batch.Begin(GlyphSortType::FRONT_TO_BACK);
			{
				// Render the console input text box
				InputTextBox.Draw(&Batch);

				// Render the text in the console output
				auto StartPosition = Vec2(-ScreenWidth * 0.5f + 20.0f, -ScreenHeight * 0.5f + InputTextBox.Dimensions.y + 20.0f);
				auto YOffset = 30.0f;
				for (auto C = Output.rbegin(); C != Output.rend(); C++)
				{
				 	Fonts::PrintText(StartPosition.x, StartPosition.y + YOffset, 1.0f, (*C).Text, InputTextBox.TextFont, Batch, (*C).TextColor);
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
		UICamera = Camera(ScreenWidth, ScreenHeight);
		Shader = ShaderManager::GetShader("Text");

		HUDCamera.Init(ScreenWidth, ScreenHeight);
		HUDCamera.SetScale(1.0f);

		// Console text box initialization
		InputTextBox.Position = Vec2(-ScreenWidth * 0.5f + 5.0f, -ScreenHeight * 0.5f + 20.0f);
		InputTextBox.Dimensions = Vec2((ScreenWidth / 3) - 10.0f, InputTextBox.Dimensions.y);
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
				if (Elements.size() < 3) Output.emplace_back("Error: mul requires 2 arguments", RGBA16_Red());

				// Calculate the result
				else
				{
					if (!EU::is_numeric(Elements.at(1)) || !EU::is_numeric(Elements.at(2))) 
					{
						Output.emplace_back("Error: cannot multiply non numeric terms: " + Elements.at(1) + ", " + Elements.at(2), RGBA16_Red());
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
				Output.emplace_back("Console Variables Available: ", RGBA16_Yellow());
				for (auto& c : registeredCommands) Output.emplace_back(c);
			}
			else
			{
				// Register command with CVar System
				if (Elements.size() < 2) Output.emplace_back("Error: need argument for cvar \"" + Elements.at(0) + "\"", RGBA16_Red());
				else
				{
					if (!CVarsSystem::Set(Elements.at(0), std::atof(Elements.at(1).c_str())))
					{
						Output.emplace_back("Error: cvar does not exist: " + Elements.at(0), RGBA16_Red());
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