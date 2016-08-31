#ifndef ENJON_GUIBUTTON_H
#define ENJON_GUIBUTTON_H

#include <iostream>

#include "Defines.h"
#include "Graphics/Color.h"
#include "GUI/GUIElement.h"
#include "Graphics/Animations.h"
#include "Math/Maths.h"

namespace Enjon { namespace GUI {

	// Button
	struct GUIButton : GUIElement<GUIButton>
	{
		GUIButton() 
		{ 
			Type = GUIType::BUTTON; 
		
			// Set up initial states	
			this->State 		= ButtonState::INACTIVE;
			this->HoverState 	= HoveredState::OFF_HOVER;
			this->Color 		= EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
			this->Name 			= std::string("GUIButton");
			this->TextColor 	= EG::RGBA16_LightGrey();
			this->Dimensions 	= EM::Vec2(10.0f, 10.0f);
			this->BorderColor 	= EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);

			// Set up PlayButton's on_hover signal
			this->on_hover.connect([&]()
			{
				// Change color
				this->Color = EG::RGBA16(0.2f, 0.2f, 0.2f, 1.0f);

				// Set state to active
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up PlayButton's off_hover signal
			this->off_hover.connect([&]()
			{
				this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);

				// Set state to inactive
				this->HoverState = HoveredState::OFF_HOVER;
			});
		}

		void Init()
		{}

		void Update()
		{
			
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			return true;		
		}

		void Draw(EG::SpriteBatch* TB)
		{
			
		}

		std::vector<EA::ImageFrame> Frames;   // Could totally put this in a resource manager of some sort
		EG::Fonts::Font* TextFont;
	};

	// Button with Text
	struct GUITextButton : GUIElement<GUITextButton>
	{
		GUITextButton() 
		{ 
			this->Type = GUIType::TEXT_BUTTON; 

			// Set up states
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;

			// Set up color
			this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);

			// Set up text color
			this->TextColor 	= EG::RGBA16_MidGrey();
			this->Name 			= std::string("GUITextButton");
			this->TextPadding	= EM::Vec2(5.0f, 5.0f);

			// Dimensions
			Dimensions 			= EM::Vec2(160.0f, 18.0f);

			// Get font
			this->TextFont 		= nullptr;
			this->FontScale		= 1.0f;

			// Set up off_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
				this->Color = EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0f);
				this->TextColor = EG::RGBA16_White();
			});

			// Set up on_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
				this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
				this->TextColor = EG::RGBA16_MidGrey();
			});
		}

		void Init()
		{}

		void Update()
		{
			
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			return true;	
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			// If null
			if (this->TextFont == nullptr) this->TextFont = EG::FontManager::GetFont("WeblySleek_10");

			// Draw box
			Batch->Add(
						EM::Vec4(
									this->AABB.Min,
									this->AABB.Max - this->AABB.Min
								),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
						this->Color,
						this->Depth,
						EG::SpriteBatch::DrawOptions::BORDER,
						EG::SetOpacity(EG::RGBA16(0.08f, 0.08f, 0.08f, 1.0f), 1.0f)
					);

			// Draw text
			EG::Fonts::PrintText(	
									Position.x + TextPadding.x, 
									Position.y + TextPadding.y, 
									this->FontScale, 
									this->Text, 
									this->TextFont, 
									*Batch, 
									this->TextColor,
									EG::Fonts::TextStyle::SHADOW,
									0.0f, 
									2.0f
								);
		}

		EM::Vec2 TextPadding;
		EGUI::Signal<GUIElementBase*> on_click;
		EGUI::Signal<> on_hover;
		EGUI::Signal<> off_hover;
		EG::Fonts::Font* TextFont;
		float FontScale;
	};

	// Value Text Box Button (Acts as a button group, which maybe would be better)
	template <typename T>
	struct GUIValueButton : GUIElement<GUIValueButton<T>>
	{
		GUIValueButton() 
		{ 
			// Set up type
			this->Type = GUIType::VALUE_BUTTON; 

			// Member variables
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;

			this->Value = 0;
			this->Step = 1;
			this->MaxValue = 10;
			this->MinValue = 0;
			this->Dimensions = EM::Vec2(145.0f, 18.0f);
			this->Depth = 0.0f;
			this->JustFocused = false;
			this->LoopValues = false;
			this->BorderColor = EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);
			this->FontScale = 1.0f;
			this->YOffset = 0.0f;

			ValueUp.Dimensions 		= EM::Vec2(15.0f, 16.0f);
			ValueDown.Dimensions 	= EM::Vec2(15.0f, 16.0f);
			ValueText.Dimensions 	= EM::Vec2(92.0f, 18.0f);

			ValueUp.Text = "+";
			ValueDown.Text = "-";

			ValueText.Text = "0.50";
			ValueText.MaxStringLength = 10;

			// These aren't working...
			ValueUp.Parent 		= this;
			ValueDown.Parent 	= this;
			ValueUp.Parent 		= this;

			// Set up connect for Value Up
			this->ValueUp.on_click.connect([&]()
			{
				// Get value of ValueText
				auto V = static_cast<T>(std::atof(ValueText.Text.c_str()));

				// Value goes up
				V += Step;

				if (V > MaxValue) 
				{ 
					if (LoopValues) V = MinValue;
					else V = MaxValue; 
				}

				std::cout << "Value: " << V << ", Step: " << Step << std::endl;

				// Reset value and reset string
				Value = static_cast<T>(V);
				SetText();
				this->lose_focus.emit();
			});

			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
			});

			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
			});

			// Set up connect for Value Down
			this->ValueDown.on_click.connect([&]()
			{
				// Get value of ValueText
				auto V = static_cast<T>(std::atof(ValueText.Text.c_str()));

				// Value goes up
				V -= Step;

				if (V < MinValue) 
				{ 
					if (LoopValues) V = MaxValue;
					else 			V = MinValue;
				}

				// Reset value and reset string
				Value = V;
				SetText();
				this->lose_focus.emit();
			});

			this->ValueText.on_enter.connect([&]()
			{
				// Set value to text
				auto Val = static_cast<T>(std::atof(ValueText.Text.c_str()));
				if (Val > MaxValue) Set(MaxValue);
				else if (Val <= MinValue || ValueText.Text.compare("") == 0) Set(MinValue);
				else Set(Val);

				std::cout << "Value after entering: "<< Value.get() << std::endl;

			});

			this->lose_focus.connect([&]()
			{
				std::cout << "Here, too!" << std::endl;

				// Turn ValueText caret
				ValueText.lose_focus.emit();

				// Turn off just focused
				JustFocused = false;

				ValueText.on_enter.emit();

				this->BorderColor = EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);

				this->State = ButtonState::INACTIVE;
			});

			this->on_click.connect([&]()
			{
				JustFocused = true;


				this->State = ButtonState::ACTIVE;
			});

			this->check_children.connect([&](EI::InputManager* Input, EG::Camera2D* Camera)
			{
			    auto MousePos = Input->GetMouseCoords();
			    Camera->ConvertScreenToWorld(MousePos);

			    // Check for collision with ValueText
			    auto MouseOverText = EP::AABBvsPoint(&ValueText.AABB, MousePos);
			    if (MouseOverText)
			    {
			    	ValueText.on_hover.emit();	
			    } 
			    else
			    {
			    	if (ValueText.HoverState = HoveredState::ON_HOVER) ValueText.off_hover.emit();
			    }
	
			    // Check if over value up
			    auto MouseOverValueUp = EP::AABBvsPoint(&ValueUp.AABB, MousePos);
			    if (MouseOverValueUp)
			    {
			    	ValueUp.on_hover.emit();
			    }
			    else if (ValueUp.HoverState) ValueUp.off_hover.emit();

			    // Check if over value down
			    auto MouseOverValueDown = EP::AABBvsPoint(&ValueDown.AABB, MousePos);
			    if (MouseOverValueDown)
			    {
			    	ValueDown.on_hover.emit();
			    }
			    else if (ValueDown.HoverState) ValueDown.off_hover.emit();
			});

		}

		void Set(T Val)
		{
			Value = static_cast<T>(Val);
			ValueText.Text = std::to_string(Value.get());
		}

		void SetText()
		{
			ValueText.Text = std::to_string(Value.get());	
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			unsigned int CurrentKey = 0;
			char CurrentChar = 0;

		    SDL_Event event;
		    while (SDL_PollEvent(&event)) 
		    {
		        switch (event.type) 
		        {
					case SDL_KEYUP:
						Input->ReleaseKey(event.key.keysym.sym); 
						CurrentKey = 0;
						break;
					case SDL_KEYDOWN:
						Input->PressKey(event.key.keysym.sym);
						CurrentKey = event.key.keysym.sym;
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

		    auto MousePos = Input->GetMouseCoords();
		    Camera->ConvertScreenToWorld(MousePos);

		    // Check for collision with ValueText
		    auto MouseOverText = EP::AABBvsPoint(&ValueText.AABB, MousePos);
		    if (MouseOverText)
		    {
		    	ValueText.on_hover.emit();	
		    } 
		    else
		    {
		    	if (ValueText.HoverState = HoveredState::ON_HOVER) ValueText.off_hover.emit();
		    }

		    // Check if over value up
		    auto MouseOverValueUp = EP::AABBvsPoint(&ValueUp.AABB, MousePos);
		    if (MouseOverValueUp)
		    {
		    	ValueUp.on_hover.emit();
		    }
		    else if (ValueUp.HoverState) ValueUp.off_hover.emit();

		    // Check if over value down
		    auto MouseOverValueDown = EP::AABBvsPoint(&ValueDown.AABB, MousePos);
		    if (MouseOverValueDown)
		    {
		    	ValueDown.on_hover.emit();
		    }
		    else if (ValueDown.HoverState) ValueDown.off_hover.emit();


		    if (Input->IsKeyPressed(SDL_BUTTON_LEFT) || JustFocused)
		    {
		    	if (JustFocused) JustFocused = false;

		    	if (MouseOverText)
		    	{
					this->BorderColor = EG::SetOpacity(EG::RGBA16(0.20f, 0.635f, 1.0f, 1.0f), 0.5f);
		    		ValueText.on_click.emit(MousePos.x);
		    	}
		    	else ValueText.lose_focus.emit();

		    	if (MouseOverValueUp)
		    	{
		    		ValueUp.on_click.emit();
		    		return true;
		    	}

		    	if (MouseOverValueDown)
		    	{
		    		ValueDown.on_click.emit();
		    		return true;
		    	}

		    	if (ValueText.KeyboardInFocus && !MouseOverText)
		    	{
		    		ValueText.lose_focus.emit();
		    		return true;
		    	}

		    }

		    // Text Input
			if (ValueText.KeyboardInFocus)
			{
				auto str = ValueText.GetNumericString(Input);

				// Check for modifiers first
				if (!ValueText.IsModifier(str[0]))
				{
					if (Input->IsKeyPressed(SDLK_BACKSPACE)) ValueText.on_backspace.emit();
					
					else if (Input->IsKeyPressed(SDLK_LEFT))
					{
						if (ValueText.CursorIndex > 0) ValueText.CursorIndex--;
					}
				
					else if (Input->IsKeyPressed(SDLK_RIGHT))
					{
						if (ValueText.CursorIndex < ValueText.Text.length()) ValueText.CursorIndex++;
					}

					else if (Input->IsKeyPressed(SDLK_PERIOD))
					{
						if (ValueText.Text.find('.') == std::string::npos) ValueText.on_keyboard.emit(".");
					}

					else if (Input->IsKeyPressed(SDLK_RETURN))
					{
						ValueText.on_enter.emit();
						return true;
					}
			
					else if (str.compare("") != 0) ValueText.on_keyboard.emit(str);
				}
			}

			return false;
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			if (!this->HoverState)
			{
				Batch->Add(
							EM::Vec4(AABB.Min, AABB.Max - AABB.Min - EM::Vec2(21, 0)),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
							ValueText.Color,
							0.0f 
						);
			}

			Batch->Add(
						EM::Vec4(this->AABB.Min, this->AABB.Max - this->AABB.Min - EM::Vec2(21, 0)),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						ValueText.Color,
						0.0f, 
						EG::SpriteBatch::DrawOptions::BORDER, 
						this->BorderColor
					);

			// Draw Text Box
			ValueText.Draw(Batch);

			if (this->HoverState)
			{
				// Draw Value Up
				Batch->Add(
							EM::Vec4(ValueUp.AABB.Min, ValueUp.AABB.Max - ValueUp.AABB.Min),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
							ValueUp.Color,
							0.0f 
						);


				auto F = EG::FontManager::GetFont("WeblySleek");
				EG::Fonts::PrintText(	
										ValueUp.Position.x + ValueUp.Dimensions.x / 2.0f - EG::Fonts::GetAdvance(ValueUp.Text[0], F, FontScale) / 2.0f, 
										ValueUp.Position.y + ValueUp.Dimensions.y / 2.0f - EG::Fonts::GetHeight(ValueDown.Text[0], F, FontScale) / 2.0f + YOffset, 
										FontScale, 
										ValueUp.Text, 
										EG::FontManager::GetFont("WeblySleek"), 
										*Batch, 
										ValueText.TextColor	
									);

				// Draw Value Down
				Batch->Add(
							EM::Vec4(ValueDown.AABB.Min, ValueDown.AABB.Max - ValueDown.AABB.Min),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
							ValueDown.Color,
							0.0f 
						);

				EG::Fonts::PrintText(	
										ValueDown.Position.x + ValueDown.Dimensions.x / 2.0f - EG::Fonts::GetAdvance(ValueDown.Text[0], F, FontScale) / 2.0f, 
										ValueDown.Position.y + ValueDown.Dimensions.y / 2.0f - EG::Fonts::GetHeight(ValueDown.Text[0], F, FontScale) / 2.0f + YOffset, 
										FontScale, 
										ValueDown.Text, 
										EG::FontManager::GetFont("WeblySleek"), 
										*Batch, 
										ValueText.TextColor	
									);

			}
		}

		void Update()
		{
			// Update positions and AABB of Value Text
			ValueText.Position = Position;
			ValueText.AABB.Min = ValueText.Position;
			ValueText.AABB.Max = ValueText.AABB.Min + ValueText.Dimensions;

			ValueDown.Position = EM::Vec2(Position.x + ValueText.Dimensions.x + 2.0f, Position.y + 1.0f);
			ValueDown.AABB.Min = ValueDown.Position;
			ValueDown.AABB.Max = ValueDown.AABB.Min + ValueDown.Dimensions;	

			ValueUp.Position = EM::Vec2(ValueDown.Position.x + ValueDown.Dimensions.x, Position.y + 1.0f);
			ValueUp.AABB.Min = ValueUp.Position;
			ValueUp.AABB.Max = ValueUp.AABB.Min + ValueUp.Dimensions;	

			// Call update on Input Text
			ValueText.Update();
		}

		void Init() {}

		Property<T> Value; 
		// T Value;
		T Step;
		T MaxValue;
		T MinValue;
		float FontScale;
		float YOffset;
		GUIButton ValueUp;
		GUIButton ValueDown;
		GUITextBox ValueText;
		uint32_t LoopValues;

	};

	// DropDownMenuButton
	struct GUIDropDownButton : GUIElement<GUIDropDownButton>
	{
		GUIDropDownButton() 
		{ 
			// Set up type
			this->Type = GUIType::DROP_DOWN_BUTTON; 

			// Member variables
			this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;

			// Get font
			this->TextFont 		= nullptr;
			this->FontScale = 1.0f;
			this->Name = std::string("GUIDropDownButton");
			this->TextColor		= EG::RGBA16_LightGrey();
			this->TextPadding = EM::Vec2(5.0f, 6.0f);
			this->BorderColor = EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);
			this->Dimensions = EM::Vec2(123.0f, 20.0f);
			this->YOffset = 20.0f;
			this->XPadding = 1.0f;
			this->Visibility = VisibleState::VISIBLE;

			// Set up Drop Down Menu Button's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
				this->Color = EG::RGBA16(0.1f, 0.1f, 0.1f, 1.0f);
			});

			// Set up Drop Down Menu Button's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
				if (!this->State) this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
			});

			// Set up Drop Down Menu Button's on_click signal
			this->on_click.connect([&]()
			{
				// Need a drop down box here with all the options to be selected
				if (this->State == ButtonState::INACTIVE)
				{
					this->Color = EG::RGBA16(0.08f, 0.08f, 0.08f, 1.0f);
					this->BorderColor = EG::SetOpacity(EG::RGBA16(0.20f, 0.635f, 1.0f, 1.0f), 0.5f);
					this->State = ButtonState::ACTIVE;
					this->Depth = 1.0f;
				} 
				else
				{
					this->Color = EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f);
					this->BorderColor = EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);
					this->State = ButtonState::INACTIVE;
					this->Depth = 0.0f;
				} 
			});

			// Set up lose focus
			this->lose_focus.connect([&]()
			{
				// Set to inactive
				if (State) this->on_click.emit();
			});
		}

		void Init()
		{}

		void Update() 
		{
			// Loop through all children and update their AABB's and positions
			auto index = 1;
			for (auto C : List)
			{
				// Update AABB of child
				C->Position = EM::Vec2(Position.x + XPadding, Position.y - YOffset * index);
				C->AABB.Min = C->Position;
				C->AABB.Max = C->AABB.Min + C->Dimensions;
				index++;
			}
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
		    SDL_Event event;
		    while (SDL_PollEvent(&event)) 
		    {
		        switch (event.type) 
		        {
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

		    auto MousePos = Input->GetMouseCoords();
		    Camera->ConvertScreenToWorld(MousePos);

		    // Check list components for being hovered over
			static EGUI::GUITextButton* ListElement = nullptr;
			if (State == ButtonState::ACTIVE)
			{
				bool Found = false;
				for (auto i = 0; i < List.size(); ++i)
				{
					auto e = List.at(i);

					if (EP::AABBvsPoint(&e->AABB, MousePos))
					{
						// If previously assigned but now not the same, then emit off hover
						if (ListElement && ListElement != e)
						{
							if (ListElement->HoverState == HoveredState::ON_HOVER) ListElement->off_hover.emit();
						}

						// Assign list element
						ListElement = e;

						// If not hoverstate active, then emit
						if (ListElement->HoverState == HoveredState::OFF_HOVER) 
						{
							ListElement = e;
							e->on_hover.emit();
						}

						// Found, so break out of loop
						Found = true;
						break;
					}
				}
				// Nothing was selected but previous list element assigned
				if (!Found && ListElement)
				{
					ListElement->off_hover.emit();
					ListElement = nullptr;
					return false;
				}
			}
			// If there was something, need to get rid of it
			else if (ListElement)
			{
				ListElement->off_hover.emit();
				ListElement = nullptr;
			}

			if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
		    {
		    	// Do AABB test with AnimationSelection List Element
		    	if (ListElement)
		    	{
		    		ListElement->on_click.emit(ListElement);
		    		ListElement->off_hover.emit();
		    		ListElement = nullptr;
		    		return true;
		    	}
		    }

		    return false;
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			if (this->TextFont == nullptr) this->TextFont = EG::FontManager::GetFont("WeblySleek_10");

			Batch->Add(
							EM::Vec4(Position.x, Position.y, this->Dimensions),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
							Color, 
							0.0f, 
							(EG::SpriteBatch::DrawOptions::BORDER), 
							this->BorderColor	
						);

			EG::Fonts::PrintText(	
									Position.x + TextPadding.x, 
									Position.y + TextPadding.y, 
									this->FontScale, 
									this->Text, 
									this->TextFont, 
									*Batch, 
									this->TextColor	
								);

			EG::Fonts::PrintText(	
									Position.x + TextPadding.x + 112.0f, 
									Position.y + TextPadding.y + 4.0f, 
									this->FontScale * 0.8f, 
									"d", 
									EG::FontManager::GetFont("Arrows7"), 
									*Batch, 
									this->TextColor, 
									EG::Fonts::TextStyle::SHADOW,
									EM::ToRadians(180.0f)
								);

			// If activated
			if (this->State)
			{
				float XPadding = 5.0f;
				auto Amount = this->List.size();
				auto GroupWidth = 160.0f;

				// Draw box for group
				Batch->Add(
							EM::Vec4(
										this->Position.x,
										this->Position.y - this->YOffset * Amount - 2.0f, 
										GroupWidth, 
										20.0f * Amount
									),
							EM::Vec4(0, 0, 1, 1),
							EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
							EG::RGBA16_DarkGrey(), 
							0.0f,
							EG::SpriteBatch::DrawOptions::BORDER | EG::SpriteBatch::DrawOptions::SHADOW,
							EG::RGBA16_DarkGrey(), 
							1.0f
						);

				// Draw each child
				for(auto C : this->List)
				{
					C->Depth = this->Depth + 1.0f;
					C->Draw(Batch);
				}

			}	
		}

		void CalculateDimensions()
		{
			std::string& T = this->Text;
			float A = 0.0f;
			auto period_count = 0;
			auto index = 0;

			std::cout << T << std::endl;

			if (this->TextFont == nullptr) this->TextFont = EG::FontManager::GetFont("WeblySleek_10");

			// Get advance
			for (auto& c : T)
			{
				// Summation of all characters
				A += EG::Fonts::GetAdvance(c, this->TextFont, this->FontScale);

				if (A > 90.0f) 
				{
					c = '.';
					period_count++;
				}

				if (period_count >= 4)
				{
					// Get substring and replace it
					T = T.substr(0, index);
					break;			
				} 

				index++;
			}

			// this->Dimensions = EM::Vec2(A + 20.0f, 20.0f);
		}

		std::vector<GUITextButton*> List;
		EG::Fonts::Font* TextFont;
		EM::Vec2 TextPadding;
		float XPadding;
		float FontScale;
		float YOffset;
	};


	// GUI Radial Button
	struct GUIRadialButton : GUIElement<GUIRadialButton>
	{
		GUIRadialButton() 
		{ 
			Type = GUIType::RADIAL_BUTTON; 

			// Load in color themes from data
			this->ActiveColor 	= EG::RGBA16_LimeGreen();
			this->InactiveColor = EG::RGBA16_DarkGrey();
			this->BorderColor = EG::SetOpacity(EG::RGBA16(0.18f, 0.18f, 0.18f, 1.0f), 0.5f);
			this->Dimensions = EM::Vec2(10.0f, 10.0f);

			// Set up initial color
			this->Color = this->InactiveColor;
			this->Name 			= std::string("GUIRadialButton");

			// Set up states
			this->State 		= ButtonState::INACTIVE;
			this->HoverState 	= HoveredState::OFF_HOVER;
	
			// Set up ToggleOnionSkin's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up ToggleOnionSkin's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
			});

			this->on_click.connect([&]()
			{
				// Toggle on and off
				this->State = this->State == ButtonState::INACTIVE ? ButtonState::ACTIVE : ButtonState::INACTIVE;

				// Toggle color
				this->Color = this->State == ButtonState::INACTIVE ? this->InactiveColor : this->ActiveColor;
			});

		}

		void Init() {}

		void Update()
		{

		}

		void Draw(EG::SpriteBatch* Batch)
		{
			Batch->Add(
						EM::Vec4(
									this->Position.x,
									this->Position.y, 
									this->Dimensions
								),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id, 
						this->Color, 
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER | EG::SpriteBatch::DrawOptions::SHADOW,
						this->BorderColor, 
						1.0f, 
						EM::Vec2(2.0f, 2.0f)	
					);
			
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			return true;	
		}

		EG::ColorRGBA16 ActiveColor;
		EG::ColorRGBA16 InactiveColor;
	};

}}


#endif