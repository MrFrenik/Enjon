#ifndef ENJON_GUI_GROUP_H
#define ENJON_GUI_GROUP_H

#include "GUIElement.h"
#include "GUIButton.h"

namespace Enjon { namespace GUI { 

	// Group is responsible for holding other gui elements and then transforming them together
	struct GUIGroup : GUIElement<GUIGroup>
	{
		GUIGroup() 
		{ 
			// Init
			Init();
		}

		GUIGroup(EM::Vec2 P)
		{
			Position = P;

			// Init
			Init();
		}
	
		void Init()
		{
			// Set up type
			Type = GUIType::GROUP; 

			// Set up member variables
			ElementIndex 	= 2;
			X0Offset 		= 15.0f;
			X1Offset		= 140.0f;
			YOffset 		= 25.0f;						// Not exact way but close estimate for now
			TitlePadding 	= 15.0f;
			Name 			= std::string("GUIGroup");		// Default Name
			Dimensions		= EM::Vec2(250.0f, 300.0f);		// Default Dimensions
			TextColor		= EG::RGBA16_MidGrey();
			Color 			= EG::RGBA16(0.12, 0.12, 0.12, 1.0f);
			TextFont 		= nullptr;
			HoveredElement	= nullptr;
			Visibility 		= VisibleState::VISIBLE;

			// Set up MinimizeButton button
			MinimizeButton.Dimensions = EM::Vec2(12.0f, 12.0f);

			// Get font
			FontScale = 1.0f;

			// Set up GUIGroup's on_hover signal
			on_hover.connect([&]()
			{
				HoverState = HoveredState::ON_HOVER;

				Color = EG::RGBA16(0.2f, 0.2f, 0.2f, 0.5f);
			});

			// Set up GUIGroup's off_hover signal
			off_hover.connect([&]()
			{
				HoverState = HoveredState::OFF_HOVER;

				Color 			= EG::RGBA16(0.12, 0.12, 0.12, 1.0f);
			});

			MinimizeButton.on_hover.connect([&]()
			{
				off_hover.emit();
			});

			MinimizeButton.on_click.connect([&]()
			{
				this->Visibility = VisibleState::HIDDEN;
			});
		}

		void AddToGroup(GUIElementBase* Element, const std::string& Name)
		{
			// Push back into group's children
			Children.push_back(Element);

			// Set Group as parent of child
			Element->Parent = this;

			// Set up position of Element in relation to group
			Element->Position = EM::Vec2(Position.x + Dimensions.x - X1Offset, Position.y + Dimensions.y - ElementIndex * YOffset - TitlePadding);
			Element->Name = Name;

			// Increment element index
			ElementIndex++;	
		}

		void Update()
		{
			// Update AABB of group based on position
			this->AABB.Min = this->Position + EM::Vec2(0.0f, this->Dimensions.y - YOffset);
			this->AABB.Max = this->AABB.Min + EM::Vec2(this->Dimensions.x, YOffset);

			// Update AABB of MinimizeButton button
			MinimizeButton.Position = this->Position + EM::Vec2(Dimensions.x - MinimizeButton.Dimensions.x - 10.0f, Dimensions.y - MinimizeButton.Dimensions.y - 5.0f);
			MinimizeButton.AABB.Min = MinimizeButton.Position;
			MinimizeButton.AABB.Max = MinimizeButton.AABB.Min + MinimizeButton.Dimensions;


			// Loop through all children and update their positions based on their index in the vector
			auto index = 2;
			for(auto C : Children)
			{
				// Update position
				C->Position = EM::Vec2(Position.x + Dimensions.x - X1Offset, Position.y + Dimensions.y - index * YOffset - TitlePadding);

				// Update AABB
				C->AABB.Min = C->Position;
				C->AABB.Max = C->AABB.Min + C->Dimensions;

				// Call update on child
				C->Update();
				index++;
			}
		}

		bool ProcessInput(EI::InputManager* Input, EG::Camera2D* Camera)
		{
			static EM::Vec2 MouseFrameOffset(0.0f);
			static bool JustFocused = true;

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

		    auto MouseOverMinimizeButton = EP::AABBvsPoint(&MinimizeButton.AABB, MousePos);
		    if (MouseOverMinimizeButton)
		    {
		    	if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
		    	{
		    		std::cout << "Hoo ah!" << std::endl;	
		    		return true;
		    	}	
		    }

		    if (Input->IsKeyDown(SDL_BUTTON_LEFT))
		    {
				auto X = MousePos.x;
				auto Y = MousePos.y;

	    		if (JustFocused) 
	    		{
	    			MouseFrameOffset = EM::Vec2(X - this->AABB.Min.x, Y - this->Position.y);
	    			JustFocused = false;
	    		}

				// Update offsets
				this->Position = EM::Vec2(X - MouseFrameOffset.x, Y - MouseFrameOffset.y);
		    }

	    	else 
	    	{
	    		this->lose_focus.emit();
	    		JustFocused = true;
	    		return true;
	    	}

			return false;			
		}

		void Draw(EG::SpriteBatch* Batch)
		{
			// Draw Group border
			Batch->Add(	
						EM::Vec4(Position, Dimensions),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						EG::SetOpacity(EG::RGBA16(0.12f, 0.12f, 0.12f, 1.0f), 0.3f),
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER | EG::SpriteBatch::DrawOptions::SHADOW,
						EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.2f),
						1.0f
					);

			// Draw Group inner border
			Batch->Add(	
						EM::Vec4(Position + EM::Vec2(5.0f, 5.0f), Dimensions - EM::Vec2(10.0f, 10.0f + YOffset)),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						EG::SetOpacity(EG::RGBA16(0.14f, 0.14f, 0.14f, 1.0f), 0.3f),
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER,
						EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.8f),
						1.0f
					);


			if (TextFont == nullptr) TextFont = EG::FontManager::GetFont("WeblySleek_12");

			// Draw Title border
			Batch->Add(	
						EM::Vec4(Position.x, Position.y + Dimensions.y - YOffset, Dimensions.x, YOffset),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						Color,
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER,
						EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.8f),
						1.0f
					);

			// Draw Title inner border
			Batch->Add(	
						EM::Vec4(Position.x + 2.0f, Position.y + Dimensions.y - YOffset + 2.0f, Dimensions.x - 4.0f, YOffset - 4.0f),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						EG::SetOpacity(EG::RGBA16(0.14f, 0.14f, 0.14f, 1.0f), 0.3f),
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER,
						EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.8f),
						1.0f
					);

			// Draw title of widget
			auto TitleFont = EG::FontManager::GetFont("WeblySleek");
			// Calculate total width of title to find placement
			float TitleAdvance = 0.0f;
			for (auto& c : Name)
			{
				TitleAdvance += EG::Fonts::GetAdvance(c, TitleFont, 1.0f);
			}

			EG::Fonts::PrintText(
									Position.x + Dimensions.x / 2.0f - TitleAdvance / 2.0f,
									Position.y + Dimensions.y - YOffset + EG::Fonts::GetHeight(Name[0], TitleFont, 1.0f) / 2.0f,
									1.0f,
									Name,
									TitleFont,
									*Batch,
									EG::SetOpacity(EG::RGBA16_MidGrey(), 0.6f)
								);

			// Draw MinimizeButton button
			Batch->Add(	
						EM::Vec4(MinimizeButton.Position, MinimizeButton.Dimensions),
						EM::Vec4(0, 0, 1, 1),
						EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						MinimizeButton.Color,
						0.0f,
						EG::SpriteBatch::DrawOptions::BORDER,
						EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.8f),
						1.0f
					);

			EG::Fonts::PrintText(
									MinimizeButton.Position.x + MinimizeButton.Dimensions.x / 2.0f,
									MinimizeButton.Position.y + MinimizeButton.Dimensions.y / 2.0f,
									1.0f,
									"x",
									EG::FontManager::GetFont("Arrows7"),
									*Batch,
									EG::SetOpacity(EG::RGBA16_MidGrey(), 0.6f), 
									EG::Fonts::TextStyle::SHADOW,
									EM::ToRadians(180.0f)
								);



			// Try and draw this shiz
			for(auto& E : Children)
			{
				// Draw border around hovered element
				if (HoveredElement == E)
				{
					Batch->Add(	
								EM::Vec4(Position.x, E->Position.y, Dimensions.x, E->Dimensions.y),
								EM::Vec4(0, 0, 1, 1),
								EI::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								EG::SetOpacity(EG::RGBA16(0.2f, 0.2f, 0.2f, 1.0f), 0.1f),
								0.0f,
								EG::SpriteBatch::DrawOptions::BORDER,
								EG::SetOpacity(EG::RGBA16_DarkGrey(), 0.2f),
								1.0f
							);
				}

				// Print name of child
				EG::Fonts::PrintText(
										Position.x + X0Offset, 																						// X Position
										E->Position.y + E->Dimensions.y / 2.0f - EG::Fonts::GetHeight(E->Name[0], TextFont, FontScale) / 6.0f,		// Y Position
										FontScale,																									// Font Scale
										E->Name + std::string(":"),																					// Child Name
										TextFont,																									// Font
										*Batch,																										// SpriteBatch
										TextColor																									// Font Color
									);

				// Print Child contents
				E->Draw(Batch);
			}

		}

		// Vector of children
		std::vector<GUIElementBase*> Children;
		GUIElementBase* HoveredElement;
		EG::Fonts::Font* TextFont;
		EG::ColorRGBA16 TextColor;
		float TitlePadding;
		float FontScale;
		float X0Offset;
		float X1Offset;
		float YOffset;
		GUIButton MinimizeButton;
		int32_t ElementIndex;
	};


}}

#endif