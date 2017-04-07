#ifndef ENJON_GUI_SCENE_ELEMENT_H
#define ENJON_GUI_SCENE_ELEMENT_H

#include "GUI/GUIElement.h"
#include "Math/Maths.h"
#include "Defines.h"

namespace Enjon { namespace GUI {

	// AnimationFrameElement
	struct GUISceneElement : GUIElement<GUISceneElement>
	{
		GUISceneElement()
		{
			// Set up type
			this->Type = GUIType::SCENE_ELEMENT;

			// Set up states
			this->State = ButtonState::INACTIVE;
			this->HoverState = HoveredState::OFF_HOVER;
			this->Dimensions = Enjon::Vec2(100.0f);
			this->Position = Enjon::Vec2(0.0f);
			this->AABB.Min = this->Position;
			this->AABB.Max = this->AABB.Min + this->Dimensions;

			// Set up SceneAnimation's on_hover signal
			this->on_hover.connect([&]()
			{
				this->HoverState = HoveredState::ON_HOVER;
			});

			// Set up SceneAnimation's off_hover signal
			this->off_hover.connect([&]()
			{
				this->HoverState = HoveredState::OFF_HOVER;
			});

		}

		void Init() {}

		void Update()
		{
			// Update AABB
			this->AABB.Min = this->Position;
			this->AABB.Max = this->AABB.Min + this->Dimensions;
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

			/*
		    if (Input->IsKeyDown(SDL_BUTTON_LEFT))
		    {
				auto X = MousePos.x;
				auto Y = MousePos.y;

	    		if (JustFocused) 
	    		{
	    			MouseFrameOffset = Enjon::Vec2(MousePos.x - this->AABB.Min.x, MousePos.y - this->AABB.Min.y);
	    			JustFocused = false;

	    			// Just clicked
	    			this->on_click.emit();
	    		}

				// Update Position
				this->Position = Enjon::Vec2(X - MouseFrameOffset.x, Y - MouseFrameOffset.y);

				// Emit that value has changed
				this->on_value_change.emit();
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
			Batch->Add(
							Vec4(Position, Dimensions),
							Vec4(0, 0, 1, 1),
							Enjon::ResourceManager::GetTexture("../Assets/Textures/Default.png").id
						);

		}

		EGUI::Signal<> on_value_change;
	};

	namespace AnimationElement
	{
		void AABBSetup(GUISceneElement* SceneAnimation);
	}
}}


#endif