#ifndef ENJON_GUI_GROUP_H
#define ENJON_GUI_GROUP_H

#include "GUIElement.h"
#include "GUIButton.h"

namespace Enjon { namespace GUI { 

	// Group is responsible for holding other gui elements and then transforming them together
	struct GUISceneGroup : GUIElement<GUISceneGroup>
	{
		GUISceneGroup() 
		{ 
			// Init
			Init();
		}

		GUISceneGroup(Enjon::Vec2 P)
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
			Name 			= std::string("GUISceneGroup");		// Default Name
			Dimensions		= Enjon::Vec2(250.0f, 300.0f);		// Default Dimensions
			TextColor		= RGBA16_MidGrey();
			Color 			= RGBA16(0.12, 0.12, 0.12, 1.0f);
			HoveredElement	= nullptr;
			Visibility 		= VisibleState::VISIBLE;

			// Set up GUISceneGroup's on_hover signal
			on_hover.connect([&]()
			{
				HoverState = HoveredState::ON_HOVER;
			});

			// Set up GUISceneGroup's off_hover signal
			off_hover.connect([&]()
			{
				HoverState = HoveredState::OFF_HOVER;
			});
		}

		void AddToGroup(GUIElementBase* Element, const std::string& Name)
		{
			// Push back into group's children
			Children.push_back(Element);

			// Set Group as parent of child
			Element->Parent = this;
		}

		void Update()
		{
			// Update each child in group
			for (auto& E : Children)
			{
				E->Update();
			}
		}

		bool ProcessInput(Input* Input, Camera2D* Camera)
		{
			static Enjon::Vec2 MouseFrameOffset(0.0f);
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

			return false;			
		}

		void Draw(SpriteBatch* Batch)
		{
			// Try and draw this shiz
			for(auto& E : Children)
			{
				// Print Child contents
				E->Draw(Batch);
			}

		}

		// Vector of children
		std::vector<GUIElementBase*> Children;
		GUIElementBase* HoveredElement;
	};

	// Group is responsible for holding other gui elements and then transforming them together
	struct GUIVerticleGroup : GUIElement<GUIVerticleGroup>
	{
		GUIVerticleGroup() 
		{ 
			// Init
			Init();
		}

		GUIVerticleGroup(Enjon::Vec2 P)
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
			Name 			= std::string("GUIVerticleGroup");		// Default Name
			Dimensions		= Enjon::Vec2(250.0f, 300.0f);		// Default Dimensions
			TextColor		= RGBA16_MidGrey();
			Color 			= RGBA16(0.12, 0.12, 0.12, 1.0f);
			TextFont 		= nullptr;
			HoveredElement	= nullptr;
			Visibility 		= VisibleState::VISIBLE;

			// Set up MinimizeButton button
			MinimizeButton.Dimensions = Enjon::Vec2(12.0f, 12.0f);

			// Get font
			FontScale = 1.0f;

			// Set up GUIVerticleGroup's on_hover signal
			on_hover.connect([&]()
			{
				HoverState = HoveredState::ON_HOVER;

				Color = RGBA16(0.2f, 0.2f, 0.2f, 0.5f);
			});

			// Set up GUIVerticleGroup's off_hover signal
			off_hover.connect([&]()
			{
				HoverState = HoveredState::OFF_HOVER;

				Color 			= RGBA16(0.12, 0.12, 0.12, 1.0f);
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
			Element->Position = Enjon::Vec2(Position.x + Dimensions.x - X1Offset, Position.y + Dimensions.y - ElementIndex * YOffset - TitlePadding);
			Element->Name = Name;

			// Increment element index
			ElementIndex++;	
		}

		void Update()
		{
			// Update AABB of group based on position
			this->AABB.Min = this->Position + Enjon::Vec2(0.0f, this->Dimensions.y - YOffset);
			this->AABB.Max = this->AABB.Min + Enjon::Vec2(this->Dimensions.x, YOffset);

			// Update AABB of MinimizeButton button
			MinimizeButton.Position = this->Position + Enjon::Vec2(Dimensions.x - MinimizeButton.Dimensions.x - 10.0f, Dimensions.y - MinimizeButton.Dimensions.y - 5.0f);
			MinimizeButton.AABB.Min = MinimizeButton.Position;
			MinimizeButton.AABB.Max = MinimizeButton.AABB.Min + MinimizeButton.Dimensions;


			// Loop through all children and update their positions based on their index in the vector
			auto index = 2;
			for(auto C : Children)
			{
				// Update position
				C->Position = Enjon::Vec2(Position.x + Dimensions.x - X1Offset, Position.y + Dimensions.y - index * YOffset - TitlePadding);

				// Update AABB
				C->AABB.Min = C->Position;
				C->AABB.Max = C->AABB.Min + C->Dimensions;

				// Call update on child
				C->Update();
				index++;
			}
		}

		bool ProcessInput(Enjon::Input* Input, Camera2D* Camera)
		{
			static Enjon::Vec2 MouseFrameOffset(0.0f);
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
				/*
		    	if (Input->IsKeyPressed(SDL_BUTTON_LEFT))
		    	{
		    		std::cout << "Hoo ah!" << std::endl;	
		    		return true;
		    	}	
				*/
		    }

			/*
		    if (Input->IsKeyDown(SDL_BUTTON_LEFT))
		    {
				auto X = MousePos.x;
				auto Y = MousePos.y;

	    		if (JustFocused) 
	    		{
	    			MouseFrameOffset = Enjon::Vec2(X - this->AABB.Min.x, Y - this->Position.y);
	    			JustFocused = false;
	    		}

				// Update offsets
				this->Position = Enjon::Vec2(X - MouseFrameOffset.x, Y - MouseFrameOffset.y);
		    }

	    	else 
	    	{
	    		this->lose_focus.emit();
	    		JustFocused = true;
	    		return true;
	    	}
			*/

			return false;			
		}

		void Draw(SpriteBatch* Batch)
		{
			// Draw Group border
			Batch->Add(	
						Vec4(Position, Dimensions),
						Vec4(0, 0, 1, 1),
						Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						SetOpacity(RGBA16(0.12f, 0.12f, 0.12f, 1.0f), 0.3f),
						0.0f,
						SpriteBatch::DrawOptions::BORDER | SpriteBatch::DrawOptions::SHADOW,
						SetOpacity(RGBA16_DarkGrey(), 0.2f),
						1.0f
					);

			// Draw Group inner border
			Batch->Add(	
						Vec4(Position + Enjon::Vec2(5.0f, 5.0f), Dimensions - Enjon::Vec2(10.0f, 10.0f + YOffset)),
						Vec4(0, 0, 1, 1),
						Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						SetOpacity(RGBA16(0.14f, 0.14f, 0.14f, 1.0f), 0.3f),
						0.0f,
						SpriteBatch::DrawOptions::BORDER,
						SetOpacity(RGBA16_DarkGrey(), 0.8f),
						1.0f
					);


			if (TextFont == nullptr) TextFont = FontManager::GetFont("WeblySleek_12");

			// Draw Title border
			Batch->Add(	
						Vec4(Position.x, Position.y + Dimensions.y - YOffset, Dimensions.x, YOffset),
						Vec4(0, 0, 1, 1),
						Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						Color,
						0.0f,
						SpriteBatch::DrawOptions::BORDER,
						SetOpacity(RGBA16_DarkGrey(), 0.8f),
						1.0f
					);

			// Draw Title inner border
			Batch->Add(	
						Vec4(Position.x + 2.0f, Position.y + Dimensions.y - YOffset + 2.0f, Dimensions.x - 4.0f, YOffset - 4.0f),
						Vec4(0, 0, 1, 1),
						Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						SetOpacity(RGBA16(0.14f, 0.14f, 0.14f, 1.0f), 0.3f),
						0.0f,
						SpriteBatch::DrawOptions::BORDER,
						SetOpacity(RGBA16_DarkGrey(), 0.8f),
						1.0f
					);

			// Draw title of widget
			auto TitleFont = FontManager::GetFont("WeblySleek");
			// Calculate total width of title to find placement
			float TitleAdvance = 0.0f;
			for (auto& c : Name)
			{
				TitleAdvance += Fonts::GetAdvance(c, TitleFont, 1.0f);
			}

			Fonts::PrintText(
									Position.x + Dimensions.x / 2.0f - TitleAdvance / 2.0f,
									Position.y + Dimensions.y - YOffset + Fonts::GetHeight(Name[0], TitleFont, 1.0f) / 2.0f,
									1.0f,
									Name,
									TitleFont,
									*Batch,
									SetOpacity(RGBA16_MidGrey(), 0.6f)
								);

			// Draw MinimizeButton button
			Batch->Add(	
						Vec4(MinimizeButton.Position, MinimizeButton.Dimensions),
						Vec4(0, 0, 1, 1),
						Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
						MinimizeButton.Color,
						0.0f,
						SpriteBatch::DrawOptions::BORDER,
						SetOpacity(RGBA16_DarkGrey(), 0.8f),
						1.0f
					);

			Fonts::PrintText(
									MinimizeButton.Position.x + MinimizeButton.Dimensions.x / 2.0f,
									MinimizeButton.Position.y + MinimizeButton.Dimensions.y / 2.0f,
									1.0f,
									"x",
									FontManager::GetFont("Arrows7"),
									*Batch,
									SetOpacity(RGBA16_MidGrey(), 0.6f), 
									Fonts::TextStyle::SHADOW,
									Enjon::ToRadians(180.0f)
								);



			// Try and draw this shiz
			for(auto& E : Children)
			{
				// Draw border around hovered element
				if (HoveredElement == E)
				{
					Batch->Add(	
								Vec4(Position.x, E->Position.y, Dimensions.x, E->Dimensions.y),
								Vec4(0, 0, 1, 1),
								Enjon::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/HealthBarWhite.png").id,
								SetOpacity(RGBA16(0.2f, 0.2f, 0.2f, 1.0f), 0.1f),
								0.0f,
								SpriteBatch::DrawOptions::BORDER,
								SetOpacity(RGBA16_DarkGrey(), 0.2f),
								1.0f
							);
				}

				// Print name of child
				Fonts::PrintText(
										Position.x + X0Offset, 																						// X Position
										E->Position.y + E->Dimensions.y / 2.0f - Fonts::GetHeight(E->Name[0], TextFont, FontScale) / 6.0f,		// Y Position
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
		Fonts::Font* TextFont;
		ColorRGBA16 TextColor;
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