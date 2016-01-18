#include "ECS/Animation2DSystem.h"
#include "ECS/AttributeSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/InventorySystem.h"
#include "ECS/Renderer2DSystem.h"
#include "ECS/PlayerControllerSystem.h"

#include <Graphics/Camera2D.h>
#include <IO/ResourceManager.h>

namespace ECS { namespace Systems { namespace Animation2D {

	// TESTING THIS ONLY
	// static enum EntityAnimationState { WALKING, ATTACKING, IDLE }; // This should be split up into continuous and discrete states
	// static enum Weapons { BOW, DAGGER };
	static EntityAnimationState PlayerState = EntityAnimationState::WALKING;
	static Weapons CurrentWeapon = Weapons::DAGGER;
	bool HitFrame = false;
	
	void SetPlayerState(EntityAnimationState State)
	{
		PlayerState = State;
	}

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
		// Get System
		struct Animation2DSystem* System = Manager->Animation2DSystem;
		// Loop through all entities with animations
		for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
		{
			static float blink_counter = 0.0f;
			static float blink_increment = 1.0f;
			static float damaged_counter = 0.0f;

			Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[e];
			// Change colors based on health	
			if (Manager->AttributeSystem->Masks[e] & Masks::Type::AI)
			{
				if (HealthComponent->Health <= 50.0f) Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA8_Orange();
				if (HealthComponent->Health <= 20.0f) Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA8_Red();
			}

			// If has an animation component
			if (Manager->Masks[e] & COMPONENT_ANIMATION2D)
			{
				// Just testing out random effects based on health
				float Health = Manager->AttributeSystem->HealthComponents[e].Health;
				if (Health <= 10.0f) 		blink_increment = 0.5f;
				else if (Health <= 20.0f) 	blink_increment = 1.0f;
				else if (Health <= 50.0f) 	blink_increment = 2.0f;

				if (Manager->AttributeSystem->Masks[e] & Masks::GeneralOptions::DAMAGED)
				{
					Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA8(0, 0, 0, 0);  // damaged color for now 
					damaged_counter += 0.1f;
					if (damaged_counter >= 0.5f)
					{
						Manager->Renderer2DSystem->Renderers[e].Color = Enjon::Graphics::RGBA8_White();  
						Manager->AttributeSystem->Masks[e] &= ~Masks::GeneralOptions::DAMAGED;
						damaged_counter = 0.0f;
					}						
				}

				if (Health <= 50.0f && Health > 0.0f) 
				{
					blink_counter += 0.1f;
					if (blink_counter >= blink_increment)
					{
						blink_counter = 0.0f;
						Manager->Masks[e] ^= COMPONENT_RENDERER2D;
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
						case EntityAnimationState::WALKING: 	CurrentAnimation = AnimationManager::GetAnimation("Walk");	break;
						case EntityAnimationState::ATTACKING:
							switch(CurrentWeapon)
							{
								case Weapons::DAGGER: CurrentAnimation = AnimationManager::GetAnimation("Attack_Dagger"); break;
								case Weapons::BOW: 		CurrentAnimation = AnimationManager::GetAnimation("Attack_Bow"); break;
							}
							break;
					}

					// Setting animation beginning frame based on view vector
					if (PlayerState == EntityAnimationState::ATTACKING && !(*SetStart))
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

						// if (ViewVector->x <= 0)
						// {
						// 	*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NW]; 
						// 	*SetStart = TRUE; 
						// 	if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector; 
						// }
						// else if (ViewVector->x > 0)  
						// {
						// 	*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NE]; 
						// 	*SetStart = TRUE; 
						// 	if (Velocity->x != 0.0f || Velocity->y != 0.0f && CurrentWeapon != Weapons::BOW) *AttackVector = *ViewVector; 
						// }
					
						// Set currentframe to beginning frame
						AnimationComponent->CurrentFrame = 0;
					}

					// Set beginning frame based on view vector
					if (PlayerState == EntityAnimationState::WALKING || PlayerState == EntityAnimationState::IDLE)
					{
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
					AnimationComponent->AnimationTimer += CurrentAnimation->AnimationTimerIncrement;
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
							if (ActiveFrame == *BeginningFrame + 3)
							{
								// Activate collision with dagger "hit frame"
								if (CurrentWeapon == Weapons::DAGGER)
								{
									// Collision at this point
									HitFrame = true;

									// Make Weapon visible and collidable
									eid32 Weapon = Manager->InventorySystem->Inventories[e].WeaponEquipped;
									Manager->Masks[Weapon] |= (COMPONENT_TRANSFORM3D | COMPONENT_RENDERER2D);
								}

								if (CurrentWeapon == Weapons::BOW)
								{
									// Create an arrow projectile entity for now...
									static Enjon::Graphics::SpriteSheet ItemSheet;
									if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/arrows.png"), Enjon::Math::iVec2(8, 1));
									eid32 id = EntitySystem::CreateItem(Manager, Enjon::Math::Vec3(Position->x + 32.0f, Position->y + 32.0f, Position->z + 50.0f),
															  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, (Masks::Type::WEAPON | Masks::WeaponOptions::PROJECTILE | Masks::GeneralOptions::PICKED_UP), 
															  Component::EntityType::PROJECTILE);
									Manager->Masks[id] |= COMPONENT_TRANSFORM3D;

									// Set arrow velocity to normalize: mousepos - arrowpos
									Enjon::Math::Vec2 MousePos = Manager->PlayerControllerSystem->PlayerControllers[e].Input->GetMouseCoords();
									Manager->Camera->ConvertScreenToWorld(MousePos);
									Enjon::Math::Vec2 Pos = Manager->TransformSystem->Transforms[id].Position.XY();

									// Find vector between the two and normalize
									Enjon::Math::Vec2 ArrowVelocity = Enjon::Math::Vec2::Normalize(MousePos - Enjon::Math::Vec2(Pos.x - 32.0f, Pos.y - 32.0f));

									float speed = 30.0f;

									// Fire in direction of mouse
									Manager->TransformSystem->Transforms[id].Velocity = Enjon::Math::Vec3(ArrowVelocity.x * speed, ArrowVelocity.y * speed, 0.0f);

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



