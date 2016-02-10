
#include "ECS/PlayerControllerSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/Animation2DSystem.h"
#include "ECS/InventorySystem.h"

namespace ECS { namespace Systems { namespace PlayerController {

	struct PlayerControllerSystem* NewPlayerControllerSystem(ECS::Systems::EntityManager* Manager)
	{
		struct PlayerControllerSystem* System = new PlayerControllerSystem;
		System->Manager = Manager;

		// Set up current index
		System->CurrentIndex = 0;

		return System;
	}

	void Update(struct PlayerControllerSystem* System)
	{
		ECS::Systems::EntityManager* Manager = System->Manager;

		for (eid32 e = Manager->Player; e < Manager->Player + 1; e++)
		{
			if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
			{ 
				Enjon::Input::InputManager* Input = System->PlayerControllers[e].Input; 

				SDL_Event event;
				static SDL_Joystick* Stick;
				static int joyX;
				static int joyY;
				const static int JOY_DEADZONE = 3000;

				while (SDL_PollEvent(&event)) {
					switch (event.type) {
						case SDL_QUIT:
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
						case SDL_JOYAXISMOTION:
							switch(event.jaxis.axis)
							{
								case 0: 
									if (event.jaxis.value < -JOY_DEADZONE)
									{
										printf ("Left\n");
									}
									if (event.jaxis.value > JOY_DEADZONE)
										printf("Right\n");
									break;
								case 1:
									if (event.jaxis.value < -JOY_DEADZONE)
										printf("Up\n");
									if (event.jaxis.value > JOY_DEADZONE)
										printf("Down\n");
									break;
							}
							break;
						case SDL_JOYBUTTONDOWN:
							printf("Button down!\n");
							break;
						default:
							break;
					}
			    } 

				// NOTE(John): I don't like this here...
				static float goal = 2.5f;
				static float Multiplier = 1.0f;
				static eid32 id = 0;
				Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[e];
				Component::Animation2D* Animation = &Manager->Animation2DSystem->Animations[e];
				
				if (Input->IsKeyPressed(SDL_BUTTON_LEFT)) {
					// Set to attack?
					Animation2D::SetPlayerState(Animation2D::EntityAnimationState::ATTACKING);  // NOTE(John): THIS IS FUCKING AWFUL
				}

				if (Input->IsKeyPressed(SDLK_r)) {
					printf("flipping on\n");
					eid32 WeaponEquipped = Manager->InventorySystem->Inventories[e].WeaponEquipped;
					Manager->Masks[WeaponEquipped] ^= COMPONENT_RENDERER2D;	
				}

				if (Input->IsKeyPressed(SDLK_1)) {
					// Set current weapon to dagger
					Animation2D::SetCurrentWeapon(Animation2D::Weapons::DAGGER);
					printf("pressed dagger\n");
				}

				if (Input->IsKeyPressed(SDLK_2)) {
					// Set current weapon to bow
					Animation2D::SetCurrentWeapon(Animation2D::Weapons::BOW);
					printf("pressed bow\n");
				}

				if (Input->IsKeyDown(SDLK_w)) {

					Transform->VelocityGoal.y = Multiplier * goal / 2.0f;
					Transform->ViewVector.y = 1.0f;
				}
				if (Input->IsKeyDown(SDLK_s)) {

					Transform->VelocityGoal.y = Multiplier * -goal / 2.0f;
					Transform->ViewVector.y = -1.0f;
				}
				if (Input->IsKeyDown(SDLK_a)) {

					Transform->VelocityGoal.x = Multiplier * -goal;
					Transform->ViewVector.x = -1.0f; 
				}

				if (Input->IsKeyDown(SDLK_d)) {

					Transform->VelocityGoal.x = Multiplier * goal;
					Transform->ViewVector.x = 1.0f; 
				}

				if (Input->IsKeyDown(SDLK_SPACE)) {
					Transform->VelocityGoal.z = Multiplier * goal;	
				}


				if (!Input->IsKeyDown(SDLK_SPACE)) {
					Transform->VelocityGoal.z = -9.8f;	
				}

				if (!Input->WasKeyDown(SDLK_w) && !Input->WasKeyDown(SDLK_s))
				{
					if (Manager->TransformSystem->Transforms[e].ViewVector.x != 0) Manager->TransformSystem->Transforms[e].ViewVector.y = 0;
				} 
				
				if (!Input->WasKeyDown(SDLK_a) && !Input->WasKeyDown(SDLK_d))
				{
					if (Manager->TransformSystem->Transforms[e].ViewVector.y != 0) Manager->TransformSystem->Transforms[e].ViewVector.x = 0;
				} 

				if (Input->IsKeyDown(SDLK_LSHIFT)) {
					goal = SPRINTPACE;
				}
 				else goal = WALKPACE;

				if (Input->IsKeyPressed(SDLK_TAB)) {

					printf("Size: %d\n", System->Targets.size());

					if (System->Targets.size())
					{
						Manager->Renderer2DSystem->Renderers[System->CurrentTarget].Color = Enjon::Graphics::RGBA8_White();

						printf("getting next target...\n");
						System->CurrentIndex = (System->CurrentIndex + 1) % System->Targets.size();
						System->CurrentTarget = System->Targets[System->CurrentIndex];

						System->CurrentTarget = System->Targets[System->CurrentIndex];
					}
				}
			
				if (!Input->IsKeyDown(SDLK_w) && !Input->IsKeyDown(SDLK_s)) Transform->VelocityGoal.y = 0;
				if (!Input->IsKeyDown(SDLK_a) && !Input->IsKeyDown(SDLK_d)) Transform->VelocityGoal.x = 0; 
			}
		}
	}

}}}


