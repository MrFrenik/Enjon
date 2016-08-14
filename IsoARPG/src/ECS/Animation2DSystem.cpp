#include "ECS/Animation2DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/EntityFactory.h"
#include "AnimManager.h"

#include <Graphics/Camera2D.h>
#include <Graphics/SpriteSheetManager.h>
#include <IO/ResourceManager.h>

#include <iostream>

namespace ECS { namespace Systems { namespace Animation2D {

	// TESTING THIS ONLY
	static EntityAnimationState PlayerState = EntityAnimationState::IDLE;
	static Weapons CurrentWeapon = Weapons::DAGGER;
	bool HitFrame = false;
	static bool attack_switch = false;
	static bool NewState = false;
	
	void SetPlayerState(EntityAnimationState State)
	{
		PlayerState = State;
		if (PlayerState != State) NewState = true;
	}

	EntityAnimationState GetPlayerState() { return PlayerState; }

	void SetCurrentWeapon(Weapons CW)
	{
		CurrentWeapon = CW;
	}

	struct Animation2DSystem* NewAnimation2DSystem(struct EntityManager* Manager)
	{
		struct Animation2DSystem* System = new Animation2DSystem;
		System->Manager = Manager;
		return System;
	}

	void Update(struct EntityManager* Manager)
	{
		// Attack speed
		// TODO(John): Make this dependent on equipped weapon and player stats
		static float AttackSpeed = 0.8f;

		// Get System
		struct Animation2DSystem* System = Manager->Animation2DSystem;
		// Loop through all entities with animations

		for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
		{
			static float damaged_counter = 0.0f;

			// If has an animation component
			if (Manager->Masks[e] & COMPONENT_ANIMATION2D)
			{
				// If damaged
				// NOTE(John): Belongs in renderer system
				if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::DAMAGED)
				{
					Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA16(100.0f, 0.0f, 0.0f, 100.0f);  // damaged color for now 
					damaged_counter += 0.1f;
					if (damaged_counter >= 0.5f)
					{
						Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA16_White();  
						Manager->AttributeSystem->Masks[e] &= ~Masks::GeneralOptions::DAMAGED;
						damaged_counter = 0.0f;
					}						
				}

				// If is a player
				if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
				{
					// Get necessary items
					Transform3DSystem* TransformSystem = Manager->TransformSystem;
					Enjon::Math::Vec2* ViewVector = &TransformSystem->Transforms[e].ViewVector;
					Enjon::Math::Vec2* AttackVector = &TransformSystem->Transforms[e].AttackVector;
					Enjon::Math::Vec3* Velocity = &TransformSystem->Transforms[e].Velocity;
					Enjon::Math::Vec3* Position = &TransformSystem->Transforms[e].Position;


					Component::AnimComponent* AnimComponent = &Manager->Animation2DSystem->AnimComponents[e];
					Enjon::uint32* SetStart = &AnimComponent->SetStart;
					auto CurrentAnimation = AnimComponent->CurrentAnimation;

					// Check for new state
					if (NewState)
					{
						AnimComponent->CurrentIndex = 0;
						NewState = false;
						AnimComponent->SetStart = false;
					}


					// Walking
					if (PlayerState == EntityAnimationState::WALKING)
					{
						
						if (ViewVector->x > 0) Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Walk");
						else 				  Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Walk_Mirror");
						AnimComponent->SetStart = false;
						attack_switch = false;
					}

					// Attacking
					else if (PlayerState == EntityAnimationState::ATTACKING)
					{
						attack_switch = !attack_switch;

						switch(CurrentWeapon)
						{
							case Weapons::DAGGER: 	
													{
														if (!(*SetStart))
														{
															if (attack_switch) 	CurrentAnimation = AnimManager::GetAnimation("Enemy_Attack");
															else 				CurrentAnimation = AnimManager::GetAnimation("Enemy_Attack");
														}
														break;
													}
							case Weapons::BOW: 		CurrentAnimation = AnimManager::GetAnimation("Player_Attack_OH_L_SE"); break;
							case Weapons::AXE: 		CurrentAnimation = AnimManager::GetAnimation("Player_Attack_OH_L_SE"); break;
							default: 				CurrentAnimation = AnimManager::GetAnimation("Player_Attack_OH_L_SE"); break;
						}
					}

					// Idle
					else
					{
						Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Pixel");
						AnimComponent->SetStart = false;
					}

					if (PlayerState == EntityAnimationState::ATTACKING && !(*SetStart))
					{
						*SetStart = TRUE;

						// Reset current index
						AnimComponent->CurrentIndex = 0;

						if (PlayerController::GetTargeting())
						{
							// Stuff
						}
						

						else
						{
							// Get direction to mouse
							Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[e].Input->GetMouseCoords();
							Manager->Camera->ConvertScreenToWorld(MousePos);
	
							if (MousePos.x <= Position->x)
							{
								if (attack_switch) 	Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Attack_Mirror");
								else 				Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Attack_Mirror");	
								if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector; 
							}
							else if (MousePos.x > Position->x)  
							{
								if (attack_switch) 	Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Attack");
								else 				Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Attack");	
								if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector; 
							}
						}
					}

					if (PlayerState == EntityAnimationState::ATTACKING)
					{
						// if (AnimComponent->CurrentIndex > CurrentAnimation->Frames.size()) AnimComponent->CurrentIndex = 0;

						// Increase timer (should do this with delta time passed in)
						AnimComponent->AnimationTimer += 0.3f * AttackSpeed;

						// Get handle to current frame
						auto* Frame = &CurrentAnimation->Frames.at(AnimComponent->CurrentIndex);

						if (AnimComponent->AnimationTimer >= Frame->Delay)
						{
							// Reset timer
							AnimComponent->AnimationTimer = 0.0f;

							// Increase current index
							AnimComponent->CurrentIndex++;
						}

						// Bounds checking
						if (AnimComponent->CurrentIndex >= CurrentAnimation->Frames.size())
						{
							// Reset current index
							AnimComponent->CurrentIndex = 0;

							// Reset player state
							PlayerState = EntityAnimationState::WALKING;

							// Reset start
							*SetStart = FALSE;
						}

						eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
						eid32 id = Manager->InventorySystem->Inventories[e].WeaponEquipped;

						Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[e].Input->GetMouseCoords();
						Manager->Camera->ConvertScreenToWorld(MousePos);
						Enjon::Math::Vec2 Pos = Manager->TransformSystem->Transforms[id].Position.XY();

						// Find vector between the two and normalize
						static Enjon::Math::Vec2 AttackVelocity;

						if (AnimComponent->CurrentIndex == 1)
						{
							// Activate collision with dagger "hit frame"
							if (CurrentWeapon == Weapons::DAGGER)
							{
								AttackVelocity = Enjon::Math::Vec2::Normalize(MousePos - Enjon::Math::Vec2(Pos.x + 32.0f, Pos.y + 32.0f));

								float speed = 50.0f;

								if (!PlayerController::GetTargeting())
								{
									// Set attack vector of player to this velocity
									int Mult = 1.0f;
									Enjon::Math::Vec2 AttackVector;
									// X < 0
									if (AttackVelocity.x < 0 && AttackVelocity.x >= -0.3f) AttackVector.x = 0.0f;
									else if (AttackVelocity.x < 0 && AttackVelocity.x < -0.3f) AttackVector.x = -1.0f;
									// X > 0
									if (AttackVelocity.x >= 0 && AttackVelocity.x < 0.5f) AttackVector.x = 0.0f;
									else if (AttackVelocity.x >= 0 && AttackVelocity.x >= 0.5f) AttackVector.x = 1.0f;
									// Y < 0
									if (AttackVelocity.y < 0 && AttackVelocity.y > -0.3f) AttackVector.y = 0.0f;
									else if (AttackVelocity.y < 0 && AttackVelocity.y <= -0.3f) AttackVector.y = -1.0f;
									// Y > 0
									if (AttackVelocity.y >= 0 && AttackVelocity.y < 0.3f) AttackVector.y = 0.0f;
									else if (AttackVelocity.y >= 0 && AttackVelocity.y >= 0.3f) AttackVector.y = 1.0f;

									Manager->TransformSystem->Transforms[e].AttackVector = AttackVector;
								}
							}

						}
						
						else if (AnimComponent->CurrentIndex == 10)
						{
							// Make Weapon visible and collidable
							eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
							Manager->Masks[Weapon] |= (COMPONENT_TRANSFORM3D);

							// Move player in direction of attack vector
							*Velocity = 5.0f * Enjon::Math::Vec3(AttackVelocity, 0.0f);
						}

						else 
						{ 
							eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
							EntitySystem::RemoveComponents(Manager, Weapon, COMPONENT_RENDERER2D | COMPONENT_TRANSFORM3D);
							// Manager->Masks[Weapon] |= (COMPONENT_TRANSFORM3D);
						} 
					}

					else if (PlayerState == EntityAnimationState::WALKING || PlayerState == EntityAnimationState::IDLE)
					{
						// Increase timer (should do this with delta time passed in)
						AnimComponent->AnimationTimer += 0.15f;

						// Get handle to current frame
						auto* Frame = &CurrentAnimation->Frames.at(AnimComponent->CurrentIndex);

						if (AnimComponent->AnimationTimer >= Frame->Delay)
						{
							// Reset timer
							AnimComponent->AnimationTimer = 0.0f;

							// Increase current index
							AnimComponent->CurrentIndex++;
						}

						// Bounds checking
						if (AnimComponent->CurrentIndex >= CurrentAnimation->Frames.size())
						{
							// Reset current index
							AnimComponent->CurrentIndex = 0;
						}
					}

					else
					{
						// Increase timer (should do this with delta time passed in)
						AnimComponent->AnimationTimer += 0.15f;

						// Get handle to current frame
						auto* Frame = &CurrentAnimation->Frames.at(AnimComponent->CurrentIndex);

						if (AnimComponent->AnimationTimer >= Frame->Delay)
						{
							// Reset timer
							AnimComponent->AnimationTimer = 0.0f;

							// Increase current index
							AnimComponent->CurrentIndex++;
						}

						// Bounds checking
						if (AnimComponent->CurrentIndex >= CurrentAnimation->Frames.size())
						{
							// Reset current index
							AnimComponent->CurrentIndex = 0;
						}
					}

				}

				// If AI
				else if (Manager->Masks[e] & COMPONENT_AICONTROLLER)
				{
					// Get necessary items
					Transform3DSystem* TransformSystem = Manager->TransformSystem;
					Enjon::Math::Vec2* ViewVector = &TransformSystem->Transforms[e].ViewVector;
					Enjon::Math::Vec2* AttackVector = &TransformSystem->Transforms[e].AttackVector;
					Enjon::Math::Vec3* Velocity = &TransformSystem->Transforms[e].Velocity;
					Enjon::Math::Vec3* Position = &TransformSystem->Transforms[e].Position;

					Component::AnimComponent* AnimComponent = &Manager->Animation2DSystem->AnimComponents[e];
					Enjon::uint32* SetStart = &AnimComponent->SetStart;
					auto CurrentAnimation = AnimComponent->CurrentAnimation;
					auto State = AnimComponent->AnimState;

					// Walking
					if (State == Component::AnimationState::WALKING)
					{
						if (ViewVector->x > 0) Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Walk");
						else 				 Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Walk_Mirror");
					}

					// Idle
					else
					{
						Manager->Animation2DSystem->AnimComponents[e].CurrentAnimation = AnimManager::GetAnimation("Enemy_Pixel");
						AnimComponent->SetStart = false;
					}

					if (State == Component::AnimationState::WALKING || State == Component::AnimationState::IDLE)
					{
						// Increase timer (should do this with delta time passed in)
						AnimComponent->AnimationTimer += 0.15f;

						// Get handle to current frame
						auto* Frame = &CurrentAnimation->Frames.at(AnimComponent->CurrentIndex);

						if (AnimComponent->AnimationTimer >= Frame->Delay)
						{
							// Reset timer
							AnimComponent->AnimationTimer = 0.0f;

							// Increase current index
							AnimComponent->CurrentIndex++;
						}

						// Bounds checking
						if (AnimComponent->CurrentIndex >= CurrentAnimation->Frames.size())
						{
							// Reset current index
							AnimComponent->CurrentIndex = 0;
						}
					}
				}

			}
		}
	}


	/*	
	void Update(struct EntityManager* Manager)
	{
		// Attack speed
		// TODO(John): Make this dependent on equipped weapon and player stats
		static float AttackSpeed = 1.0f;

		// Get System
		struct Animation2DSystem* System = Manager->Animation2DSystem;
		// Loop through all entities with animations
		for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
		{
			static float damaged_counter = 0.0f;

			Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[e];
			// Change colors based on health	
			if (Manager->AttributeSystem->Masks[e] & Masks::Type::AI)
			{
				auto* RS = &Manager->Renderer2DSystem->Renderers[e];
				if (HealthComponent->Health <= 50.0f) RS->Color = Enjon::Graphics::RGBA16_Orange();
				if (HealthComponent->Health <= 20.0f) RS->Color = Enjon::Graphics::RGBA16_Red();
				else RS->Color = Enjon::Graphics::RGBA16_White();
			}

			// If has an animation component
			if (Manager->Masks[e] & COMPONENT_ANIMATION2D)
			{
				if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::DAMAGED)
				{
					Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA16(100.0f, 0.0f, 0.0f, 100.0f);  // damaged color for now 
					damaged_counter += 0.1f;
					if (damaged_counter >= 0.5f)
					{
						Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA16_White();  
						Manager->AttributeSystem->Masks[e] &= ~Masks::GeneralOptions::DAMAGED;
						damaged_counter = 0.0f;
					}						
				}

				// If is a player
				if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
				{
					// Get necessary items
					Component::Animation2D* AnimationComponent = &Manager->Animation2DSystem->Animations[e];
					Transform3DSystem* TransformSystem = Manager->TransformSystem;
					Enjon::Math::Vec2* ViewVector = &TransformSystem->Transforms[e].ViewVector;
					Enjon::Math::Vec2* AttackVector = &TransformSystem->Transforms[e].AttackVector;
					Enjon::Math::Vec3* Velocity = &TransformSystem->Transforms[e].Velocity;
					Enjon::Math::Vec3* Position = &TransformSystem->Transforms[e].Position;
					Enjon::uint32* BeginningFrame = &AnimationComponent->BeginningFrame;
					Enjon::uint32* SetStart = &AnimationComponent->SetStart;
					const Animate::Animation* CurrentAnimation = AnimationComponent->CurrentAnimation;


					Component::AnimComponent* AnimComponent = &Manager->Animation2DSystem->AnimComponents[e];
					Enjon::uint32* AnimSetStart = &AnimComponent->SetStart;
					const EA::Anim* CurrentAnim = AnimComponent->CurrentAnimation;


					// Get what the current animation is based on the player state
					switch(PlayerState)
					{
						case EntityAnimationState::WALKING: 	CurrentAnimation = AnimationManager::GetAnimation("Player", "walk");	break;
						case EntityAnimationState::ATTACKING:
							switch(CurrentWeapon)
							{
								case Weapons::DAGGER: 	
														{
															CurrentAnim = AnimManager::GetAnimation("Player_Attack_OH_L_SE");
															CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_dagger"); 
															break;
														}
								case Weapons::BOW: 		CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_bow"); break;
								case Weapons::AXE: 		CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_axe"); break;
								default: 				CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_dagger"); break;
							}
							break;
					}

					// We can get whether or not we're targeting here and set view vector accordingly
					if (PlayerController::GetTargeting())
					{
						// Get velocity of target
						auto Target = Manager->PlayerControllerSystem->CurrentTarget;
						EM::Vec2* TargetPosition = &Manager->TransformSystem->Transforms[Target].GroundPosition;
						EM::Vec2* Pos = &Manager->TransformSystem->Transforms[e].GroundPosition;

						// // Find vector between the two and normalize
						Enjon::Math::Vec2 NormalizedDistance = Enjon::Math::Vec2::Normalize(*TargetPosition - *Pos);

						// Get right basis vector
						EM::Vec2 R(1, 0);

						// Get dot product
						auto Dot = R.DotProduct(NormalizedDistance);

						// Get angle
						auto a = acos(Dot) * 180.0f / M_PI;
						if (NormalizedDistance.y < 0) a *= -1;

						// a += 180.0f;

						// Rotation freedom 
						auto DOF = 22.5f;

						std::cout << "angle: " << a << std::endl;

						if 		(a >= -22.0f && a < 22.5f) 	*ViewVector = EAST;
						else if (a >= 22.5f && a < 67.5)	*ViewVector = NORTHEAST;
						else if (a >= 67.5f && a < 112.5f)	*ViewVector = NORTH;
						else if (a >= 112.5f && a < 157.5f) *ViewVector = NORTHWEST;
						else if (a >= -157.5f && a < -112.0f) * ViewVector = SOUTHWEST;
						else if (a >= -112.5f && a < -67.0f) * ViewVector = SOUTH;
						else if (a >= -67.0f && a < -22.0f) * ViewVector = SOUTHEAST;
						else *ViewVector = WEST;
					}

					// Setting animation beginning frame based on view vector
					if (PlayerState == EntityAnimationState::ATTACKING && !(*SetStart))
					{
						// Set spritesheet
						if (CurrentWeapon == Weapons::AXE) AnimationComponent->Sheet = Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("PlayerSheet2");
						else AnimationComponent->Sheet = Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("PlayerSheet");


						if (PlayerController::GetTargeting())
						{
							// Get velocity of target
							auto Target = Manager->PlayerControllerSystem->CurrentTarget;
							EM::Vec2* TargetPosition = &Manager->TransformSystem->Transforms[Target].GroundPosition;
							EM::Vec2* Pos = &Manager->TransformSystem->Transforms[e].GroundPosition;

							if (TargetPosition->x < Pos->x)
							{
								*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NW]; 
								*SetStart = TRUE; 
								if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector;
							}
							else 
							{
								*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NE]; 
								*SetStart = TRUE; 
								if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector;
							}

						}

						else
						{
							// Get direction to mouse
							Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[e].Input->GetMouseCoords();
							Manager->Camera->ConvertScreenToWorld(MousePos);
	
							if (MousePos.x <= Position->x)
							{
								*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NW]; 
								*SetStart = TRUE; 
								if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector; 
							}
							else if (MousePos.x > Position->x)  
							{
								*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NE]; 
								*SetStart = TRUE; 
								if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector; 
							}
						}
					
						// Set currentframe to beginning frame
						AnimationComponent->CurrentFrame = 0;
					}

					// Set beginning frame based on view vector
					if (PlayerState == EntityAnimationState::WALKING || PlayerState == EntityAnimationState::IDLE)
					{
						AnimationComponent->Sheet = Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("PlayerSheet");

						if		(*ViewVector == NORTHWEST)		*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NW];
						else if (*ViewVector == NORTHEAST)		*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NE];
						else if (*ViewVector == EAST)			*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::E];
						else if (*ViewVector == WEST)			*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::W];
						else if (*ViewVector == NORTH)			*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::N];
						else if (*ViewVector == SOUTH)			*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::S];
						else if (*ViewVector == SOUTHEAST)		*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::SE];
						else if (*ViewVector == SOUTHWEST)		*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::SW];
						else									*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NW];
					}

					// NOTE(John): Leave if Idle, for now
					if (PlayerState != EntityAnimationState::ATTACKING && Velocity->x == 0.0f && Velocity->y == 0.0f) return;

					// Animation
					if (PlayerState == EntityAnimationState::ATTACKING) 
					{
						if (CurrentWeapon == Weapons::BOW) AttackSpeed = 1.0f;
						else AttackSpeed = 1.0f;
						AnimationComponent->AnimationTimer += CurrentAnimation->AnimationTimerIncrement * AttackSpeed;
					}
					else AnimationComponent->AnimationTimer += CurrentAnimation->AnimationTimerIncrement;
					if (AnimationComponent->AnimationTimer >= CurrentAnimation->Profile->Delays[AnimationComponent->CurrentFrame % CurrentAnimation->Profile->FrameCount])
					{
						// Increase current frame
						AnimationComponent->CurrentFrame++;

						Enjon::uint32 ActiveFrame = AnimationComponent->CurrentFrame + *BeginningFrame;

						// Reset timer
						AnimationComponent->AnimationTimer = 0.0f;

						// Check for hit frame if attacking
						if (PlayerState == EntityAnimationState::ATTACKING)
						{
							eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
							eid32 id = Manager->InventorySystem->Inventories[e].WeaponEquipped;

							Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[e].Input->GetMouseCoords();
							Manager->Camera->ConvertScreenToWorld(MousePos);
							Enjon::Math::Vec2 Pos = Manager->TransformSystem->Transforms[id].Position.XY();

							// Find vector between the two and normalize
							static Enjon::Math::Vec2 AttackVelocity;
							// AttackVelocity = Enjon::Math::Vec2::Normalize(MousePos - Enjon::Math::Vec2(Pos.x + 32.0f, Pos.y + 32.0f));

							if (ActiveFrame == *BeginningFrame + 1)
							{
								// Activate collision with dagger "hit frame"
								if (CurrentWeapon == Weapons::DAGGER || CurrentWeapon == Weapons::AXE)
								{
									AttackVelocity = Enjon::Math::Vec2::Normalize(MousePos - Enjon::Math::Vec2(Pos.x + 32.0f, Pos.y + 32.0f));

									// Make Weapon visible and collidable
									Manager->Masks[Weapon] |= (COMPONENT_TRANSFORM3D);

									float speed = 50.0f;

									if (!PlayerController::GetTargeting())
									{
										// Set attack vector of player to this velocity
										int Mult = 1.0f;
										Enjon::Math::Vec2 AttackVector;
										// X < 0
										if (AttackVelocity.x < 0 && AttackVelocity.x >= -0.3f) AttackVector.x = 0.0f;
										else if (AttackVelocity.x < 0 && AttackVelocity.x < -0.3f) AttackVector.x = -1.0f;
										// X > 0
										if (AttackVelocity.x >= 0 && AttackVelocity.x < 0.5f) AttackVector.x = 0.0f;
										else if (AttackVelocity.x >= 0 && AttackVelocity.x >= 0.5f) AttackVector.x = 1.0f;
										// Y < 0
										if (AttackVelocity.y < 0 && AttackVelocity.y > -0.3f) AttackVector.y = 0.0f;
										else if (AttackVelocity.y < 0 && AttackVelocity.y <= -0.3f) AttackVector.y = -1.0f;
										// Y > 0
										if (AttackVelocity.y >= 0 && AttackVelocity.y < 0.3f) AttackVector.y = 0.0f;
										else if (AttackVelocity.y >= 0 && AttackVelocity.y >= 0.3f) AttackVector.y = 1.0f;


										Manager->TransformSystem->Transforms[e].AttackVector = AttackVector;

									}
								}

							}
							if (ActiveFrame == *BeginningFrame + 3 || ActiveFrame == *BeginningFrame + 6)
							{
								// Activate collision with dagger "hit frame"
								if (CurrentWeapon == Weapons::DAGGER && ActiveFrame == *BeginningFrame + 3)
								{
									// Collision at this point
									HitFrame = true;

									// Make Weapon visible and collidable
									eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
									Manager->Masks[Weapon] |= (COMPONENT_TRANSFORM3D);

									eid32 id = Manager->InventorySystem->Inventories[e].WeaponEquipped;

									// Move player in direction of attack vector
									auto Vel = &Manager->TransformSystem->Transforms[e].Velocity;
									*Vel = 5.0f * Enjon::Math::Vec3(AttackVelocity, 0.0f);
								}

								if (CurrentWeapon == Weapons::AXE && ActiveFrame == *BeginningFrame + 6)
								{
									HitFrame = true;
									eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
									Manager->Masks[Weapon] |= (COMPONENT_TRANSFORM3D);

									auto Vel = &Manager->TransformSystem->Transforms[e].Velocity;
									*Vel = 6.5f * Enjon::Math::Vec3(AttackVelocity, Vel->z);
								}

								if (CurrentWeapon == Weapons::BOW && ActiveFrame == *BeginningFrame + 3)
								{
									for (auto i = 0; i < 1; i++)
									{
										// Create an arrow projectile entity for now...
										static Enjon::Graphics::SpriteSheet ItemSheet;
										static float t = 0.0f;
										t += 0.1f;

										auto C = Enjon::Graphics::RGBA16(100.0f, 0.0f, 0.0f, 100.0f);
										// C.r += 2.0f;
										if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/verticlebar.png"), Enjon::Math::iVec2(1, 1));
										eid32 id = Factory::CreateWeapon(Manager, Enjon::Math::Vec3(Position->x + 60.0f, Position->y + 40.0f, 50.0f),
																  Enjon::Math::Vec2(10.0f + ER::Roll(0, 60), 5.0f), &ItemSheet, (Masks::Type::WEAPON | 
																  												Masks::WeaponOptions::PROJECTILE | 
																  												Masks::GeneralOptions::PICKED_UP | 
																  												Masks::GeneralOptions::COLLIDABLE), 
																  												Component::EntityType::PROJECTILE, "Arrow", 
																												C);
										Manager->Masks[id] |= COMPONENT_TRANSFORM3D;

										Manager->TransformSystem->Transforms[id].AABBPadding = EM::Vec2(15);

										// Set arrow velocity to normalize: mousepos - arrowpos
										Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[e].Input->GetMouseCoords();
										Manager->Camera->ConvertScreenToWorld(MousePos);
										Enjon::Math::Vec2 Pos = Manager->TransformSystem->Transforms[id].Position.XY();

										// // Find vector between the two and normalize
										Enjon::Math::Vec2 ArrowVelocity = Enjon::Math::Vec2::Normalize(Enjon::Math::IsoToCartesian(MousePos) - Enjon::Math::IsoToCartesian(Pos));

										EM::Vec2 R(1,0);
										float a = acos(ArrowVelocity.DotProduct(R)) * 180.0f / M_PI;
										if (ArrowVelocity.y < 0) a *= -1;

										Manager->TransformSystem->Transforms[id].Angle = EM::ToRadians(a);

										auto ArrowX = Position->x;
										auto ArrowY = Position->y;
										auto Rad = 5.0f;
										EM::Vec3* ArrowPos = &Manager->TransformSystem->Transforms[id].Position;
										*ArrowPos = EM::Vec3(ArrowPos->XY() + Rad * EM::CartesianToIso(EM::Vec2(cos(EM::ToRadians(a)), sin(EM::ToRadians(a)))), 40.0f);
										
										auto RX = sin(t) * Enjon::Random::Roll(-10, 2) / 100.0f;
										auto RY = sin(t) * Enjon::Random::Roll(-10, 2) / 100.0f;
										ArrowVelocity = Enjon::Math::CartesianToIso(ArrowVelocity);

										float speed = 60.0f;

										// // Fire in direction of mouse
										Manager->TransformSystem->Transforms[id].VelocityGoal = speed * Enjon::Math::Vec3(ArrowVelocity.x + RX, ArrowVelocity.y + RY, 0.0f);
										Manager->TransformSystem->Transforms[id].Velocity = speed * Enjon::Math::Vec3(ArrowVelocity.x + RX, ArrowVelocity.y + RY, 0.0f);
										Manager->TransformSystem->Transforms[id].Velocity.z = -5.0f;
										Manager->TransformSystem->Transforms[id].BaseHeight = 0.0f;

										// Set attack vector of player to this velocity
										int Mult = 1.0f;
										Enjon::Math::Vec2 AttackVector;
										// X < 0
										if (ArrowVelocity.x < 0 && ArrowVelocity.x >= -0.3f) AttackVector.x = 0.0f;
										else if (ArrowVelocity.x < 0 && ArrowVelocity.x < -0.3f) AttackVector.x = -1.0f;
										// X > 0
										if (ArrowVelocity.x >= 0 && ArrowVelocity.x < 0.5f) AttackVector.x = 0.0f;
										else if (ArrowVelocity.x >= 0 && ArrowVelocity.x >= 0.5f) AttackVector.x = 1.0f;
										// Y < 0
										if (ArrowVelocity.y < 0 && ArrowVelocity.y > -0.3f) AttackVector.y = 0.0f;
										else if (ArrowVelocity.y < 0 && ArrowVelocity.y <= -0.3f) AttackVector.y = -1.0f;
										// Y > 0
										if (ArrowVelocity.y >= 0 && ArrowVelocity.y < 0.3f) AttackVector.y = 0.0f;
										else if (ArrowVelocity.y >= 0 && ArrowVelocity.y >= 0.3f) AttackVector.y = 1.0f;


										Manager->TransformSystem->Transforms[e].AttackVector = AttackVector;

										// Set up coordinate format
										Manager->Renderer2DSystem->Renderers[id].Format = EG::CoordinateFormat::ISOMETRIC;	

										// Set up parent
										Manager->AttributeSystem->Groups[id].Parent = e;
									}
								}
							}

							else 
							{ 
								HitFrame = false;
								eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
								EntitySystem::RemoveComponents(Manager, Weapon, COMPONENT_RENDERER2D | COMPONENT_TRANSFORM3D);
							} 

							// End attacking animation state
							if (ActiveFrame >= *BeginningFrame + CurrentAnimation->Profile->FrameCount)
							{
								AnimationComponent->CurrentFrame = 0;
								PlayerState = EntityAnimationState::WALKING;
								*SetStart = FALSE;
							}
						}

						// Reset animation	
						if (ActiveFrame >= *BeginningFrame + CurrentAnimation->Profile->FrameCount) 
						{
							AnimationComponent->CurrentFrame = 0;
						}
					}
				}
			}
		} 
	}
	*/

}}}


