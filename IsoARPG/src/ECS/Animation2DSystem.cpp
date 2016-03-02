#include "ECS/Animation2DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/PlayerControllerSystem.h"
#include "ECS/EntityFactory.h"

#include <Graphics/Camera2D.h>
#include <Graphics/SpriteSheetManager.h>
#include <IO/ResourceManager.h>

#include <iostream>

namespace ECS { namespace Systems { namespace Animation2D {

	// TESTING THIS ONLY
	static EntityAnimationState PlayerState = EntityAnimationState::WALKING;
	static Weapons CurrentWeapon = Weapons::DAGGER;
	bool HitFrame = false;
	
	void SetPlayerState(EntityAnimationState State)
	{
		PlayerState = State;
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
					const Animation* CurrentAnimation = AnimationComponent->CurrentAnimation;

					// Get what the current animation is based on the player state
					switch(PlayerState)
					{
						case EntityAnimationState::WALKING: 	CurrentAnimation = AnimationManager::GetAnimation("Player", "walk");	break;
						case EntityAnimationState::ATTACKING:
							switch(CurrentWeapon)
							{
								case Weapons::DAGGER: 	CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_dagger"); break;
								case Weapons::BOW: 		CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_bow"); break;
								case Weapons::AXE: 		CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_axe"); break;
								default: 				CurrentAnimation = AnimationManager::GetAnimation("Player", "attack_dagger"); break;
							}
							break;
					}

					// Setting animation beginning frame based on view vector
					if (PlayerState == EntityAnimationState::ATTACKING && !(*SetStart))
					{
						// Set spritesheet
						if (CurrentWeapon == Weapons::AXE) AnimationComponent->Sheet = Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("PlayerSheet2");
						else AnimationComponent->Sheet = Enjon::Graphics::SpriteSheetManager::GetSpriteSheet("PlayerSheet");

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
						if (CurrentWeapon == Weapons::BOW) AttackSpeed = 10.0f;
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
										auto C = Enjon::Graphics::RGBA16_Orange();
										C.r += 2.0f;
										if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));
										eid32 id = Factory::CreateWeapon(Manager, Enjon::Math::Vec3(Position->XY(), 40.0f),
																  Enjon::Math::Vec2(80.0f, 80.0f), &ItemSheet, (Masks::Type::WEAPON | 
																  												Masks::WeaponOptions::PROJECTILE | 
																  												Masks::GeneralOptions::PICKED_UP | 
																  												Masks::GeneralOptions::COLLIDABLE), 
																  												Component::EntityType::PROJECTILE, "arrow", 
																												C);
										Manager->Masks[id] |= COMPONENT_TRANSFORM3D;


										// Set arrow velocity to normalize: mousepos - arrowpos
										Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[e].Input->GetMouseCoords();
										Manager->Camera->ConvertScreenToWorld(MousePos);
										Enjon::Math::Vec2 Pos = Position->XY();
										// MousePos.y += 80.0f;

										// // Find vector between the two and normalize
										Enjon::Math::Vec2 ArrowVelocity = Enjon::Math::Vec2::Normalize(Enjon::Math::IsoToCartesian(MousePos) - Enjon::Math::IsoToCartesian(Pos));
										auto RX = Enjon::Random::Roll(-10, 2) / 100.0f;
										auto RY = Enjon::Random::Roll(-10, 2) / 100.0f;
										ArrowVelocity = Enjon::Math::CartesianToIso(ArrowVelocity);

										float speed = 50.0f;

										// // Fire in direction of mouse
										// Manager->TransformSystem->Transforms[id].Velocity = speed * Enjon::Math::Vec3(ArrowVelocity.x, ArrowVelocity.y, 0.0f);
										// Manager->TransformSystem->Transforms[id].VelocityGoal = speed * Enjon::Math::Vec3(ArrowVelocity.x, ArrowVelocity.y, 0.0f);
										Manager->TransformSystem->Transforms[id].VelocityGoal = speed * Enjon::Math::Vec3(ArrowVelocity.x + RX, ArrowVelocity.y + RY, 0.0f);
										Manager->TransformSystem->Transforms[id].Velocity = speed * Enjon::Math::Vec3(ArrowVelocity.x + RX, ArrowVelocity.y + RY, 0.0f);
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

}}}


