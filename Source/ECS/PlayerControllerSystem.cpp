
#include <Graphics/SpriteSheetManager.h>

#include "ECS/PlayerControllerSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/Animation2DSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/EntityFactory.h"

namespace ECS { namespace Systems { namespace PlayerController {

	bool Targeting = false;

	void ShootGrenade(struct EntityManager* Manager, Enjon::Math::Vec3 Pos, EG::SpriteSheet* Sheet);
	void MakeExplosion(struct EntityManager* Manager, EM::Vec3 Pos);
	void MakeVortex(struct EntityManager* Manager, EM::Vec3 Pos);

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
		static bool RightButtonDown = false;

		ECS::Systems::EntityManager* Manager = System->Manager;

		for (eid32 e = Manager->Player; e < Manager->Player + 1; e++)
		{
			if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
			{ 
				Enjon::Input::InputManager* Input = System->PlayerControllers[e].Input; 
				Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[e];

				SDL_Event event;
				static SDL_Joystick* Stick;
				static int joyX;
				static int joyY;
				const static int JOY_DEADZONE = 4500;
				static float goal = 2.5f;

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
							if (event.jaxis.axis == 0) 
							{
								float value = static_cast<float>(event.jaxis.value) / 32768.0f;
	
								if (event.jaxis.value < -JOY_DEADZONE)
								{
									Input->GamePadController.Axis0Value = value;
								}
								else if (event.jaxis.value > JOY_DEADZONE)
								{
									Input->GamePadController.Axis0Value = value;
								}
								else
								{
									Input->GamePadController.Axis0Value = 0.0f;
								}

							}
							if (event.jaxis.axis == 1)
							{
								float value = -1.0f * static_cast<float>(event.jaxis.value) / 32768.0f;

								if (event.jaxis.value < -JOY_DEADZONE)
								{
									Input->GamePadController.Axis1Value = value;
								}
								else if (event.jaxis.value > JOY_DEADZONE)
								{
									Input->GamePadController.Axis1Value = value;
								}
								else
								{
									Input->GamePadController.Axis1Value = 0.0f;
								}
							}

							// Right stick L/R
							if (event.jaxis.axis == 2)
							{
								float value = static_cast<float>(event.jaxis.value) / 32768.0f;

								if (event.jaxis.value < -JOY_DEADZONE)
								{
									// std::cout << "Right stick L: " << value << std::endl;
								}
								else if (event.jaxis.value > JOY_DEADZONE)
								{
									// std::cout << "Right stick R: " << value << std::endl;
								}
							}

							// Right stick U/D
							if (event.jaxis.axis == 3)
							{
								float value = -1.0f * static_cast<float>(event.jaxis.value) / 32768.0f;

								if (event.jaxis.value < -JOY_DEADZONE)
								{
									// std::cout << "Right stick U: " << value << std::endl;
								}
								else if (event.jaxis.value > JOY_DEADZONE)
								{
									// std::cout << "Right stick D: " << value << std::endl;
								}
							}

							if (event.jaxis.axis == 4)
							{
								float value = static_cast<float>(event.jaxis.value) / 32768.0f;

								if (event.jaxis.value < -JOY_DEADZONE)
								{
									// std::cout << "Trigger L: " << value << std::endl;
								}
								else if (event.jaxis.value > JOY_DEADZONE)
								{
									// std::cout << "Trigger L: " << value << std::endl;
								}
							}

							if (event.jaxis.axis == 5)
							{
								float value = static_cast<float>(event.jaxis.value) / 32768.0f;

								if (event.jaxis.value < -JOY_DEADZONE)
								{
									Input->ReleaseKey(SDL_BUTTON_LEFT);
								}
								else if (event.jaxis.value > JOY_DEADZONE)
								{
									Input->PressKey(SDL_BUTTON_LEFT);
								}
							}
							break;
						
						case SDL_CONTROLLERBUTTONDOWN:
							Input->GamePadController.PressButton(static_cast<unsigned int>(event.cbutton.button));
							break;
						case SDL_CONTROLLERBUTTONUP:
							Input->GamePadController.ReleaseButton(static_cast<unsigned int>(event.cbutton.button));
							break;
						default:
							break;
					}
			    } 

				// NOTE(John): I don't like this here...
				static float Multiplier = 1.0f;
				static eid32 id = 0;
				Component::Animation2D* Animation = &Manager->Animation2DSystem->Animations[e];

				// Game Controller			
				if (Input->GamePadController.ControllerHandle)
				{
					auto Controller = &Input->GamePadController;
					auto CHandle = Controller->ControllerHandle;
	
					if (Input->IsKeyDown(SDLK_w) || Controller->Axis1Value > 0) {

						auto Val = Controller->Axis1Value;
						Transform->VelocityGoal.y = Val ? Val * Multiplier * goal : Multiplier * goal;
						Transform->ViewVector.y = 1.0f;
					}
					if (Input->IsKeyDown(SDLK_s) || Controller->Axis1Value < 0) {

						auto Val = Controller->Axis1Value;
						Transform->VelocityGoal.y = Val ? Val * Multiplier * goal : Multiplier * -goal;
						Transform->ViewVector.y = -1.0f;
					}
					if (Input->IsKeyDown(SDLK_a) || Controller->Axis0Value < 0) {

						auto Val = Controller->Axis0Value;
						Transform->VelocityGoal.x = Val ? Val * Multiplier * goal : Multiplier * -goal;
						Transform->ViewVector.x = -1.0f; 
					}

					if (Input->IsKeyDown(SDLK_d) || Controller->Axis0Value > 0) {

						auto Val = Controller->Axis0Value;
						Transform->VelocityGoal.x = Val ? Val * Multiplier * goal : Multiplier * goal;
						Transform->ViewVector.x = 1.0f; 
					}

					if (!Input->WasKeyDown(SDLK_w) && !Input->WasKeyDown(SDLK_s) && !Controller->Axis1Value)
					{
						if (Manager->TransformSystem->Transforms[e].ViewVector.x != 0) Manager->TransformSystem->Transforms[e].ViewVector.y = 0;
					} 
					
					if (!Input->WasKeyDown(SDLK_a) && !Input->WasKeyDown(SDLK_d) && !Controller->Axis0Value)
					{
						if (Manager->TransformSystem->Transforms[e].ViewVector.y != 0) Manager->TransformSystem->Transforms[e].ViewVector.x = 0;
					} 

					static bool Jumping = false;
					if (Input->IsKeyPressed(SDLK_SPACE) || Controller->IsButtonPressed(static_cast<unsigned int>(SDL_CONTROLLER_BUTTON_A))) {
						// Keep from double jumping
						if (Transform->Position.z <= Transform->BaseHeight) Transform->Velocity.z = 1.0f * goal;
						Jumping = true;
					}

					if (!Jumping) {
						Transform->VelocityGoal.z = 2 * -9.8f;	
						Jumping = false;
					}

					if (Input->IsKeyDown(SDLK_LCTRL)) {
						goal = WALKPACE / 2.0f;
					}
					else if (Input->IsKeyDown(SDLK_LSHIFT) || Controller->IsButtonDown(static_cast<unsigned int>(SDL_CONTROLLER_BUTTON_X))) {
						goal = SPRINTPACE;
					}
	 				else goal = WALKPACE;

					if (!Input->IsKeyDown(SDLK_w) && !Input->IsKeyDown(SDLK_s) && !Controller->Axis1Value) Transform->VelocityGoal.y = 0;
					if (!Input->IsKeyDown(SDLK_a) && !Input->IsKeyDown(SDLK_d) && !Controller->Axis0Value) Transform->VelocityGoal.x = 0;
				}

				// No Controller
				else
				{
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

					if (!Input->WasKeyDown(SDLK_w) && !Input->WasKeyDown(SDLK_s))
					{
						if (Manager->TransformSystem->Transforms[e].ViewVector.x != 0) Manager->TransformSystem->Transforms[e].ViewVector.y = 0;
					} 
					
					if (!Input->WasKeyDown(SDLK_a) && !Input->WasKeyDown(SDLK_d))
					{
						if (Manager->TransformSystem->Transforms[e].ViewVector.y != 0) Manager->TransformSystem->Transforms[e].ViewVector.x = 0;
					} 

					static bool Jumping = false;
					if (Input->IsKeyPressed(SDLK_SPACE)) {
						// Keep from double jumping
						if (Transform->Position.z <= Transform->BaseHeight) Transform->Velocity.z = 1.0f * goal;
						Jumping = true;
					}

					if (!Jumping) {
						Transform->VelocityGoal.z = 2 * -9.8f;	
						Jumping = false;
					}

					if (Input->IsKeyDown(SDLK_LCTRL)) {
						goal = WALKPACE / 2.0f;
					}
					else if (Input->IsKeyDown(SDLK_LSHIFT)) {
						goal = SPRINTPACE;
					}
	 				else goal = WALKPACE;

					if (!Input->IsKeyDown(SDLK_w) && !Input->IsKeyDown(SDLK_s)) Transform->VelocityGoal.y = 0;
					if (!Input->IsKeyDown(SDLK_a) && !Input->IsKeyDown(SDLK_d)) Transform->VelocityGoal.x = 0;
				}

				if (Input->IsKeyDown(SDL_BUTTON_LEFT)) 
				{
					// Set to attack?
					Animation2D::SetPlayerState(Animation2D::EntityAnimationState::ATTACKING);  // NOTE(John): THIS IS FUCKING AWFUL
				}

				if (Input->IsKeyDown(SDL_BUTTON_RIGHT)) 
				{
					static float t = 0.0f;
					t += 0.1f;
					auto P = Manager->TransformSystem->Transforms[Manager->Player].Position + Enjon::Math::Vec3(50.0f, 20.0f, 0.0f);
					Factory::CreateVortex(Manager, P);
					RightButtonDown = true;
				}

				else if (RightButtonDown)
				{
					auto P = Manager->TransformSystem->Transforms[Manager->Player].Position;
					Factory::CreateExplosion(Manager, P);
					RightButtonDown = false;
				}

				if (Input->IsKeyPressed(SDLK_r)) 
				{
					auto P = Manager->TransformSystem->Transforms[Manager->Player].Position + Enjon::Math::Vec3(50.0f, 20.0f, 0.0f);
					P += Enjon::Math::Vec3(Enjon::Random::Roll(-100, 100), Enjon::Random::Roll(-100, 100), 0.0f);
					ShootGrenade(Manager, P, Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("Orb"));
				}

				if (Animation2D::GetPlayerState() != Animation2D::EntityAnimationState::ATTACKING)
				{
					if (Input->IsKeyPressed(SDLK_1)) {
						// Set current weapon to dagger
						Animation2D::SetCurrentWeapon(Animation2D::Weapons::DAGGER);
						std::cout << "Selected Dagger" << std::endl;
					}

					if (Input->IsKeyPressed(SDLK_2)) {
						// Set current weapon to bow
						Animation2D::SetCurrentWeapon(Animation2D::Weapons::BOW);
						std::cout << "Selected Bow" << std::endl;
					}

					if (Input->IsKeyPressed(SDLK_3)) {
						//Set current weapon to axe
						Animation2D::SetCurrentWeapon(Animation2D::Weapons::AXE);
						printf("Pressed axe\n");
					}
				}

				// if (Input->IsKeyPressed(SDLK_t)) {

				// 	Targeting = !Targeting;
				// }


				if (Input->IsKeyPressed(SDLK_TAB)) {

					printf("Size: %d\n", System->Targets.size());

					if (System->Targets.size())
					{
						Manager->Renderer2DSystem->Renderers[System->CurrentTarget].Color = Enjon::Graphics::RGBA16_White();

						printf("getting next target...\n");
						System->CurrentIndex = (System->CurrentIndex + 1) % System->Targets.size();
						System->CurrentTarget = System->Targets[System->CurrentIndex];

						System->CurrentTarget = System->Targets[System->CurrentIndex];
					}
				}
			}
		}
	}

	void ShootGrenade(struct EntityManager* Manager, Enjon::Math::Vec3 Pos, Enjon::Graphics::SpriteSheet* Sheet)
	{
		ECS::eid32 Player = Manager->Player;
		auto G = Enjon::Graphics::RGBA16_ZombieGreen();
		G.g += 100.0f;
		float height = 20.0f;
		ECS::eid32 Grenade = Factory::CreateWeapon(Manager, Enjon::Math::Vec3(Pos.XY(), height / 2.0f), Enjon::Math::Vec2(16.0f, 16.0f), Sheet,
													Masks::Type::WEAPON | Masks::WeaponOptions::PROJECTILE | Masks::WeaponSubOptions::GRENADE, Component::EntityType::PROJECTILE, "Grenade", G);

		// Shoot in direction of mouse
		Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[Player].Input->GetMouseCoords();
		Manager->Camera->ConvertScreenToWorld(MousePos);
		// MousePos.y -= 20.0f;

		Manager->AttributeSystem->Masks[Grenade] |= Masks::GeneralOptions::RISING | Masks::GeneralOptions::COLLIDABLE;

		// Find vector between the two and normalize
		Enjon::Math::Vec2 GV = Enjon::Math::Vec2::Normalize(Enjon::Math::IsoToCartesian(MousePos) - Enjon::Math::IsoToCartesian(Pos.XY()));
		auto RX = Enjon::Random::Roll(-2, 2) / 100.0f;
		auto RY = Enjon::Random::Roll(-2, 2) / 100.0f;
		GV = Enjon::Math::CartesianToIso(GV);

		float speed = 10.0f;

		Manager->TransformSystem->Transforms[Grenade].Velocity = speed * Enjon::Math::Vec3(GV.x + RX, GV.y + RY, 0.0f);
		Manager->TransformSystem->Transforms[Grenade].VelocityGoal = speed * Enjon::Math::Vec3(GV.x + RX, GV.y + RY, 0.0f);
		Manager->TransformSystem->Transforms[Grenade].BaseHeight = 0.0f;
		Manager->TransformSystem->Transforms[Grenade].MaxHeight = height;
	}

	bool GetTargeting() { return Targeting; }

}}}



