#include "ECS/ComponentSystems.h"
#include "Utils/Errors.h"
#include "Math/Random.h"
#include "Graphics/SpriteSheet.h"
#include "IO/ResourceManager.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>

#define WALKPACE	3.0f
#define SPRINTPACE  WALKPACE * 2.0f

void printDebug(char* message);

namespace ECS { namespace Systems {

	///////////////////////////
	// Entity Manager System //
	///////////////////////////

	namespace EntitySystem {

		// Creates new EntityManager and returns it
		struct EntityManager* NewEntityManager(int Width, int Height)
		{
			struct EntityManager* Manager = new EntityManager;

			if (Manager == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_ENTITY_MANAGER::Manager is null"); 

			// Initialize NextAvailableID to 0
			Manager->NextAvailableID = 0;

			// Initialize Length to 0
			Manager->Length = 0;

			// Initialize LastUsedID to 0
			Manager->MaxAvailableID = 0;

			// Initialize the component systems
			Manager->TransformSystem			= TransformSystem::NewTransform3DSystem(Manager);
			Manager->PlayerControllerSystem 	= PlayerController::NewPlayerControllerSystem(Manager);
			Manager->AIControllerSystem			= AIController::NewAIControllerSystem(Manager);
			Manager->LabelSystem				= Label::NewLabelSystem(Manager);
			Manager->Animation2DSystem			= Animation2D::NewAnimation2DSystem(Manager);
			Manager->AttributeSystem 			= Attributes::NewAttributeSystem(Manager);
			Manager->Renderer2DSystem 			= Renderer2D::NewRenderer2DSystem(Manager);
			Manager->InventorySystem 			= Inventory::NewInventorySystem(Manager);

			Manager->Width = Width;
			Manager->Height = Height;

			// Initialize spatial hash
			Manager->Grid = new SpatialHash::Grid;
			SpatialHash::Init(Manager->Grid, Manager->Width, Manager->Height);

			return Manager;
		} 

		// NOTE(John): Creating and Removing entities is buggy right now in the way that it manager ids
		// Creates blank entity and returns eid
		eid32 CreateEntity(struct EntityManager* Manager, bitmask32 Components)
		{
			eid32 Entity;

			// Generate a unique id for our entity if possible
			if (Manager->Length != MAX_ENTITIES) 
			{
				// Set id of entity to next available one
				Entity = Manager->NextAvailableID; 

				// Increment both NextAvailableID and Length if they're equal
				if (Manager->NextAvailableID == Manager->MaxAvailableID) 
				{
					Manager->NextAvailableID++;
					Manager->MaxAvailableID++;
				}
				else
				{
					// Otherwise set next to length, which is the greatest unused id
					Manager->NextAvailableID = Manager->MaxAvailableID;
				} 

				// Increment Length
				Manager->Length++;
				
				// Set bitfield up
				Manager->Masks[Entity] = Components;
				
				return Entity;
			}
			
			// Otherwise return MAX_ENTITIES as an error
			return MAX_ENTITIES;
		}

		// Removes selected entity from manager by setting bitfield to COMPONENT_NONE
		void RemoveEntity(struct EntityManager* Manager, eid32 Entity)
		{
			// Set component mask to COMPONENT_NONE to remove
			Manager->Masks[Entity] = COMPONENT_NONE;

			// Decrement length
			if (Manager->Length > 0) Manager->Length--; 

			// Set next available entity id to entity
			Manager->NextAvailableID = Entity; 
		}

		// Verifies whether or not entity is alive
		bool IsAlive(struct EntityManager* Manager, eid32 Entity)
		{
			// If alive, then its component mask will not equal COMPONENT_NONE
			return Manager->Masks[Entity] != COMPONENT_NONE;
		}

		// Creats player entity and returns eid
		eid32 CreatePlayer(struct EntityManager* Manager, Enjon::Input::InputManager* Input, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, 
						   Enjon::Graphics::SpriteSheet* Sheet, char* Name, float VelocityScale, Enjon::Math::Vec3 Velocity, Enjon::Math::Vec3 VelocityGoal, float Health, Enjon::Graphics::ColorRGBA8 Color)
		{
			// Get id for player
			eid32 Player = CreateEntity(Manager,
										COMPONENT_RENDERER2D		| 
										COMPONENT_TRANSFORM3D 		| 
										COMPONENT_PLAYERCONTROLLER 	| 
										COMPONENT_LABEL 			| 
										COMPONENT_ANIMATION2D 		| 
										COMPONENT_HEALTH); 

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[Player];
			Transform->Velocity = Velocity;
			Transform->Dimensions = Dimensions;
			Transform->Position = Position;
			Transform->VelocityGoalScale = VelocityScale;
			Transform->VelocityGoal = VelocityGoal; 
			Transform->ViewVector = Enjon::Math::Vec2(1, 1);
			Transform->AttackVector = Enjon::Math::Vec2(1, 1);
			Transform->GroundPosition = Enjon::Math::Vec2(Position.x + Dimensions.x / 2.0f, Position.y);
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;

			// Set up Input
			Manager->PlayerControllerSystem->PlayerControllers[Player].Input = Input;
			
			// Set up Animation2D
			Component::Animation2D* Animation2D = &Manager->Animation2DSystem->Animations[Player];
			Animation2D->Sheet = Sheet; 
			// These are redundant...
			Animation2D->CurrentFrame = 0; 
			Animation2D->BeginningFrame = 0;
			Animation2D->CurrentAnimation = AnimationManager::GetAnimation("Walk");
			Animation2D->SetStart = 0;

			// Set up Label 
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[Player].Name = Name;
			Manager->LabelSystem->Labels[Player].Entity = Player;

			// Set up health component
			Manager->AttributeSystem->HealthComponents[Player].Health = Health;
			Manager->AttributeSystem->HealthComponents[Player].Entity = Player;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[Player].Color = Color;

			// Set up Type
			Manager->Types[Player] = Component::EntityType::PLAYER;
			
			return Player;
		} 

		// Creates ai entity and returns eid
		eid32 CreateAI(struct EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, char* Name, float VelocityScale,
						 Enjon::Math::Vec3 Velocity, Enjon::Math::Vec3 VelocityGoal, float Health, Enjon::Graphics::ColorRGBA8 Color)
		{
			// Get id for ai
			eid32 AI = CreateEntity(Manager, 
									COMPONENT_RENDERER2D	| 
									COMPONENT_TRANSFORM3D 	| 
									COMPONENT_AICONTROLLER 	| 
									COMPONENT_ANIMATION2D 	| 
									COMPONENT_LABEL 		| 
									COMPONENT_HEALTH		);

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[AI];
			Transform->Velocity = Velocity;
			Transform->Position = Position;
			Transform->Dimensions = Dimensions;
			Transform->VelocityGoal = VelocityGoal;
			Transform->VelocityGoalScale = VelocityScale;
			Transform->ViewVector = Enjon::Math::Vec2(1, 1);
			Transform->AttackVector = Enjon::Math::Vec2(1, 1);
			Transform->GroundPosition = Enjon::Math::Vec2(Position.XY());
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;

			// Set up Animation2D
			Component::Animation2D* Animation2D = &Manager->Animation2DSystem->Animations[AI];
			Animation2D->Sheet = Sheet; 
			// These are redundant...
			Animation2D->CurrentFrame = 0; 
			Animation2D->BeginningFrame = 0; 

			
			// Set up Label
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[AI].Name = Name;

			// Set up Attributes
			Manager->AttributeSystem->HealthComponents[AI].Health = Health;
			Manager->AttributeSystem->HealthComponents[AI].Entity = AI;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[AI].Color = Color;

			// Set up type
			Manager->Types[AI] = Component::EntityType::ENEMY;

			return AI;
		} 

		// Creates ai entity and returns eid
		eid32 CreateItem(struct EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, char* Name, Component::EntityType Type, 
								Enjon::Graphics::ColorRGBA8 Color)
		{
			// Get id for ai
			eid32 Item = CreateEntity(Manager, 
									COMPONENT_RENDERER2D	| 
									COMPONENT_TRANSFORM3D 	| 
									COMPONENT_ANIMATION2D 	| 
									COMPONENT_LABEL 		);

			// Set up Transform
			Component::Transform3D* Transform = &Manager->TransformSystem->Transforms[Item];
			Transform->Position = Position;
			Transform->VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
			Transform->Dimensions = Dimensions;
			Transform->GroundPosition = Enjon::Math::Vec2(Position.XY());
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;

			// Set up Animation2D
			Manager->Animation2DSystem->Animations[Item].Sheet = Sheet; 
			Manager->Animation2DSystem->Animations[Item].CurrentFrame = 0; 
			Manager->Animation2DSystem->Animations[Item].BeginningFrame = 0; 
			
			// Set up Label
			// NOTE(John): This isn't the best way to do this; most likely will throw an error at some point
			Manager->LabelSystem->Labels[Item].Name = Name;

			// Set up Renderer
			Manager->Renderer2DSystem->Renderers[Item].Color = Color;

			// Set up type
			Manager->Types[Item] = Type;

			return Item;
		} 

	} // namespace EntitySystem

	/////////////////////
	// AttributeSystem //
	/////////////////////

	namespace Attributes
	{
		// Constructs and returns new AIControllerSystem
		// TODO(John): Write custom allocator for this
		struct AttributeSystem* NewAttributeSystem(struct EntityManager* Manager)
		{
			struct AttributeSystem* System = (AttributeSystem*)malloc(sizeof(AttributeSystem));
			if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_ATTRIBUTE_SYSTEM::System is null"); 
			System->Manager = Manager;
			return System;
		}

		// Updates Attributes of entity that it's attached to
		void Update(struct AttributeSystem* System)
		{

		}	
	}

	/////////////////
	// LabelSystem //
	/////////////////

	namespace Label
	{ 
		// Constructs and returns new AIControllerSystem
		// TODO(John): Write custom allocator for this
		struct LabelSystem* NewLabelSystem(struct EntityManager* Manager)
		{
			struct LabelSystem* System = (LabelSystem*)malloc(sizeof(LabelSystem));
			if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_LABEL_SYSTEM::System is null"); 
			System->Manager = Manager;
			return System;
		}
		
		// Updates Label Component of entity it is attached to
		void Update(struct LabelSystem* System)
		{

		}
	} 


	/////////////////////
	// TransformSystem //
	/////////////////////

	// Reponsible for managing all the entity transforms 
	namespace TransformSystem
	{ 
		// Create a new Transform3D Sytem
		Transform3DSystem* NewTransform3DSystem(struct EntityManager* Manager)
		{
			Transform3DSystem* System = (Transform3DSystem*)malloc(sizeof(Transform3DSystem));
			if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_TRANSFORM_3D_SYSTEM::System is null"); 
			System->Manager = Manager;
			return System;
		}

		// Updates a Transform3D system
		// NOTE(John): It would be much faster to draw the entity in here after being updated instead of having to pass through the entities again later on
		void Update(Transform3DSystem* System)
		{
			EntityManager* Manager = System->Manager;
			// Look at the entities in the Manager up to the last entered position and then update based on component masks
			for (eid32 e = 0; e <Manager->MaxAvailableID; e++)
			{
				// To manage collisions, need to split the screen into quadrants, then check all other entities in that quadrant for collision 
				// If equal then transform that entity
				if (Manager->Masks[e] & COMPONENT_TRANSFORM3D)
				{
					// First transform the velocity by LERPing it
					Component::Transform3D* Transform = &System->Transforms[e];
					float Scale = Transform->VelocityGoalScale; 
					Enjon::Math::Vec3* Velocity = &Transform->Velocity; 
					Enjon::Math::Vec3* VelocityGoal = &Transform->VelocityGoal;
					Enjon::Math::Vec3* Position = &Transform->Position; 
					Enjon::Math::Vec2* GroundPosition = &Transform->GroundPosition; 
					float TileWidth = 32.0f;
					
					Velocity->x = Enjon::Math::Lerp(VelocityGoal->x, Velocity->x, Scale); 
					Velocity->y = Enjon::Math::Lerp(VelocityGoal->y, Velocity->y, Scale); 
					Velocity->z = Enjon::Math::Lerp(VelocityGoal->z, Velocity->z, Scale); 
					
					// Set entity's position after interpolating
					Position->x += Velocity->x;
					Position->z += Velocity->z; 
					
					// Clamp z position to BaseHeight
					if (Position->z < Transform->BaseHeight) 
					{
						Velocity->z = 0.0f;
						Position->z = Transform->BaseHeight;
						Position->y = GroundPosition->y + Position->z;
					} 
					
					// Set position.y to be a sum of y and z velocities
					Position->y += Velocity->y + Velocity->z; 
					
					// Set up GroundPosition
					GroundPosition->x = Position->x + Transform->Dimensions.x / 2.0f - TileWidth;
					GroundPosition->y = Position->y - Position->z; 

					// Set up CartesianPosition
					Transform->CartesianPosition = Enjon::Math::IsoToCartesian(*GroundPosition);

					// Make sure that position is within bounds of World
					int Width = Manager->Width, Height = Manager->Height;
					bool CollideWithLevel = false;
					if (Transform->CartesianPosition.x < -Width + TileWidth) { Transform->CartesianPosition.x = -Width + TileWidth; Velocity->x *= -1; CollideWithLevel = true; }   
					if (Transform->CartesianPosition.x > 0.0f) { Transform->CartesianPosition.x = 0.0f; Velocity->x *= -1; CollideWithLevel = true; }
					if (Transform->CartesianPosition.y > 0.0f) { Transform->CartesianPosition.y = 0.0f; Velocity->y *= -1; CollideWithLevel = true; }
					if (Transform->CartesianPosition.y < -Height + TileWidth) { Transform->CartesianPosition.y = -Height + TileWidth; Velocity->y *= -1; CollideWithLevel = true; }

					// Delete item for now if it collides with level
					if ((Manager->Types[e] == Component::EntityType::ITEM | Manager->Types[e] == Component::EntityType::PROJECTILE) && CollideWithLevel)
					{
						printf("EntityAmount before delete: %d\n", Manager->Length);
						EntitySystem::RemoveEntity(Manager, e);
						printf("EntityAmount after delete: %d\n", Manager->Length);
					}

					*GroundPosition = Enjon::Math::CartesianToIso(Transform->CartesianPosition);
					Position->y = GroundPosition->y + Position->z;
					Position->x = GroundPosition->x - Transform->Dimensions.x / 2.0f + TileWidth;
				}
			}
		}
	} // namespace TransformSystem

	
	/////////////////////
	// InventorySystem //
	/////////////////////

	namespace Inventory
	{
		// Updates Transforms of EntityManager
		void Update(InventorySystem* System)
		{

		}

		// Creates new Transform3DSystem
		InventorySystem* NewInventorySystem(struct EntityManager* Manager)
		{
			struct InventorySystem* System = (InventorySystem*)malloc(sizeof(InventorySystem));
			if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_INVENTORY_SYSTEM::System is null");
			System->Manager = Manager;
			return System;
		}
	} 
	
	////////////////////
	// RendererSystem //
	////////////////////

	// Reponsible for managing all the renderers
	namespace Renderer2D
	{
		// Updates Renderers of EntityManager
		void Update(struct EntityManager* Manager)
		{

		}

		// Create new Render2DSystem
		Renderer2DSystem* NewRenderer2DSystem(struct EntityManager* Manager)
		{
			struct Renderer2DSystem* System = (Renderer2DSystem*)malloc(sizeof(Renderer2DSystem));
			if (System == nullptr) Enjon::Utils::FatalError("COMPOPNENT_SYSTEMS::NEW_RENDERER2D_SYSTEM::System is null"); 
			System->Manager = Manager;
			return System;
		}
	} 
	
	namespace AIController
	{
		struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager)
		{ 
			struct AIControllerSystem* System = (struct AIControllerSystem*)malloc(sizeof(struct AIControllerSystem));
			System->Manager = Manager;
			return System;
		}

		// Updates Controller of AI it is attached to
		void Update(struct AIControllerSystem* System, eid32 Player)
		{
			struct EntityManager* Manager = System->Manager;
			const Component::Transform3D* Target = &Manager->TransformSystem->Transforms[Player];
			// Let's just make it go towards the player for testing
			for (eid32 ai = 0; ai < Manager->MaxAvailableID; ai++)
			{
				// Check to see if entity has ai controller
				if ((Manager->Masks[ai] & (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D)) == (COMPONENT_AICONTROLLER | COMPONENT_TRANSFORM3D))
				{
					Component::Transform3D* AI = &Manager->TransformSystem->Transforms[ai];

					// Find difference in positions	
					Enjon::Math::Vec3 Difference = Enjon::Math::Vec3::Normalize(Enjon::Math::Vec3(Target->CartesianPosition, Target->Position.z) - Enjon::Math::Vec3(AI->CartesianPosition, AI->Position.z));

					Enjon::Math::Vec2 a = Target->GroundPosition;
					Enjon::Math::Vec2 b = AI->GroundPosition;
					float distance = a.DistanceTo(b);

					if (distance <= 60.0f) AI->VelocityGoal = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);
					// Move towards player
					// TODO(John): Come up with some kind of passed in speed parameter to multiply by the difference vector
					//AI->Velocity = Difference; 
					else AI->VelocityGoal = Difference * 2.0f;
				}
			}
		}
	} 
	
	///////////////////////
	// Animation2DSystem //
	///////////////////////

	namespace Animation2D 
	{ 
		// TESTING THIS ONLY
		static enum EntityAnimationState { WALKING, ATTACKING, IDLE }; // This should be split up into continuous and discrete states
		static enum Weapons { BOW, DAGGER };
		static EntityAnimationState PlayerState = EntityAnimationState::WALKING;
		static Weapons CurrentWeapon = Weapons::DAGGER;
		bool HitFrame = false;
		
		struct Animation2DSystem* NewAnimation2DSystem(struct EntityManager* Manager)
		{
			struct Animation2DSystem* System = (struct Animation2DSystem*)malloc(sizeof(Animation2DSystem));
			System->Manager = Manager;
			return System;
		}
		
		void Update(struct Animation2DSystem* System)
		{
			// Get manager
			struct EntityManager* Manager = System->Manager;
			// Loop through all entities with animations
			for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
			{
				// If has an animation component
				if (Manager->Masks[e] & COMPONENT_ANIMATION2D)
				{
					// If is a player
					if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
					{
						// Get necessary items
						Component::Animation2D* AnimationComponent = &Manager->Animation2DSystem->Animations[e];
						Systems::Transform3DSystem* System = Manager->TransformSystem;
						Enjon::Math::Vec2* ViewVector = &System->Transforms[e].ViewVector;
						Enjon::Math::Vec2* AttackVector = &System->Transforms[e].AttackVector;
						Enjon::Math::Vec3* Velocity = &System->Transforms[e].Velocity;
						Enjon::Math::Vec3* Position = &System->Transforms[e].Position;
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
							if		(ViewVector->x <= 0)
							{
								*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NW]; 
								*SetStart = TRUE; 
								if (Velocity->x != 0.0f && Velocity->y != 0.0f) *AttackVector = *ViewVector; 
							}
							else if (ViewVector->x > 0)  
							{
								*BeginningFrame = CurrentAnimation->Profile->Starts[Orientation::NE]; 
								*SetStart = TRUE; 
								if (Velocity->x != 0.0f && Velocity->y != 0.0f) *AttackVector = *ViewVector; 
							}
						
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
									}
									if (CurrentWeapon == Weapons::BOW)
									{
										// Create an arrow projectile entity for now...
										static Enjon::Graphics::SpriteSheet ItemSheet;
										if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/arrows.png"), Enjon::Math::iVec2(8, 1));
										eid32 id = EntitySystem::CreateItem(Manager, Enjon::Math::Vec3(Position->x + AttackVector->x * 128.0f, Position->y + AttackVector->y * 128.0f, Position->z),
																  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, "Item", Component::EntityType::PROJECTILE);

										// Give the arrow some velocity
										// Manager->TransformSystem->Transforms[id].VelocityGoal = Enjon::Math::Vec3(10.0f, 10.0f, 0.0f);
										Manager->TransformSystem->Transforms[id].Velocity = Enjon::Math::Vec3(AttackVector->x * 15.0f, AttackVector->y * 15.0f, 0.0f);

										printf("Entity Amount: %d\n", Manager->MaxAvailableID);

									}
								} 

								else HitFrame = false;


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

	} // namespace Animation2D
	
	////////////////////////////
	// PlayerControllerSystem //
	////////////////////////////
	
	namespace PlayerController
	{ 
		struct PlayerControllerSystem* NewPlayerControllerSystem(struct EntityManager* Manager)
		{
			struct PlayerControllerSystem* System = (struct PlayerControllerSystem*)malloc(sizeof(struct PlayerControllerSystem));
			System->Manager = Manager;
			return System;
		}

		void Update(struct PlayerControllerSystem* System)
		{ 
			struct EntityManager* Manager = System->Manager;

			for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
			{
				if (Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER)
				{ 
					Enjon::Input::InputManager* Input = System->PlayerControllers[e].Input; 

					SDL_Event event;

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
						Animation2D::PlayerState = Animation2D::EntityAnimationState::ATTACKING;  // NOTE(John): THIS IS FUCKING AWFUL
					}

					if (Input->IsKeyPressed(SDLK_1)) {
						// Set current weapon to dagger
						Animation2D::CurrentWeapon = Animation2D::Weapons::DAGGER;
						printf("pressed dagger\n");
					}

					if (Input->IsKeyPressed(SDLK_2)) {
						// Set current weapon to bow
						Animation2D::CurrentWeapon = Animation2D::Weapons::BOW;
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
					if (!Input->IsKeyDown(SDLK_w) && !Input->IsKeyDown(SDLK_s)) Transform->VelocityGoal.y = 0;
					if (!Input->IsKeyDown(SDLK_a) && !Input->IsKeyDown(SDLK_d)) Transform->VelocityGoal.x = 0; 
				}
			}
		}
	} 
	
	/////////////////////
	// CollisionSystem //
	/////////////////////

	namespace Collision 
	{
		// Collision BitMasks
		Enjon::uint32 COLLISION_NONE		= 0x00000000;
		Enjon::uint32 COLLISION_PLAYER		= 0x00000001;
		Enjon::uint32 COLLISION_ENEMY		= 0x00000002;
		Enjon::uint32 COLLISION_ITEM		= 0x00000004;
		Enjon::uint32 COLLISION_PROJECTILE	= 0x00000008;

		// Updates all possible collisions
		void Update(struct EntityManager* Manager)
		{
			static Enjon::uint32 times;
			times = 0; 
			
			// Check the quadrants of entities and then check for collisions
			for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
			{ 
				// If entity has no transform, then continue
				if (!(Manager->Masks[e] & COMPONENT_TRANSFORM3D)) continue;
				// Get the cell that entity belongs to
				const Enjon::Math::Vec2* EPosition = &Enjon::Math::IsoToCartesian(Manager->TransformSystem->Transforms[e].GroundPosition);
				int CellIndex = SpatialHash::FindCell(Manager->Grid, e, EPosition);

				// Check all entities and neighbors
				std::vector<eid32> Entities = Manager->Grid->cells[CellIndex].entities;
				SpatialHash::GetNeighborCells(Manager->Grid, CellIndex, &Entities);  // Note(John): This is causing too much slowdown
				
				// Get entities 
				if (!Entities.empty())
				{
					for (eid32 collider : Entities)
					{
						if (Manager->Masks[collider] & COMPONENT_TRANSFORM3D && collider != e)
						{
							// Get EntityType of collider and entity
							Component::EntityType AType = Manager->Types[collider];
							Component::EntityType BType = Manager->Types[e];

							// Get collision mask for A and B
							Enjon::uint32 Mask = GetCollisionType(Manager, e, collider);

							if (Mask == (COLLISION_ITEM | COLLISION_ENEMY)) 		{ 										 		continue; }
							if (Mask == (COLLISION_PROJECTILE | COLLISION_ENEMY)) 	{ CollideWithProjectile(Manager, e, collider); 	continue; } 
							if (Mask == (COLLISION_ITEM | COLLISION_PLAYER)) 		{ CollideWithItem(Manager, collider, e); 		continue; } 
							if (Mask == (COLLISION_ENEMY | COLLISION_PLAYER)) 		{ CollideWithEnemy(Manager, e, collider); 		continue; }
							if (Mask == (COLLISION_ENEMY | COLLISION_ENEMY)) 		{ CollideWithEnemy(Manager, e, collider); 		continue; }
						}
					}	
				}
			}
		}
		

		Enjon::uint32 GetCollisionType(Systems::EntityManager* Manager, ECS::eid32 A, ECS::eid32 B)
		{
	
			// Init collision mask
			Enjon::uint32 Mask = COLLISION_NONE;

			// Make sure that manager is not null
			if (Manager == nullptr) Enjon::Utils::FatalError("COLLISION_SYSTEM::GET_COLLISION_TYPE::Manager is null");

			// Get types of A and B
			const Component::EntityType* TypeA = &Manager->Types[A];
			const Component::EntityType* TypeB = &Manager->Types[B];

			// Or the mask with TypeA collision
			switch(*TypeA)
			{
				case Component::EntityType::ITEM:			Mask |= COLLISION_ITEM; 			break;
				case Component::EntityType::PLAYER:			Mask |= COLLISION_PLAYER; 			break;
				case Component::EntityType::ENEMY: 			Mask |= COLLISION_ENEMY; 			break;
				case Component::EntityType::PROJECTILE: 	Mask |= COLLISION_PROJECTILE; 		break;
				default: 									Mask |= COLLISION_NONE;				break; 
			}	

			// // Or the mask with TypeB collision
			switch(*TypeB)
			{
				case Component::EntityType::ITEM:			Mask |= COLLISION_ITEM; 			break;
				case Component::EntityType::PLAYER:			Mask |= COLLISION_PLAYER; 			break;
				case Component::EntityType::ENEMY: 			Mask |= COLLISION_ENEMY; 			break;
				case Component::EntityType::PROJECTILE: 	Mask |= COLLISION_PROJECTILE; 		break;
				default: 									Mask |= COLLISION_NONE;				break; 
			}

			return Mask;	
		}

		void CollideWithProjectile(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
		{
			Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
			Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
			Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
			Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
			
			V2 AMin(A->x, A->y);
			V2 AMax(A->x + TILE_SIZE, A->y + TILE_SIZE);
			V2 BMin(B->x, B->y);
			V2 BMax(B->x + TILE_SIZE, B->y + TILE_SIZE);
				
			V2 ACenter = V2(A->x + TILE_SIZE / 2.0f, A->y + TILE_SIZE / 2.0f);
			V2 BCenter = V2(B->x + TILE_SIZE / 2.0f, B->y + TILE_SIZE / 2.0f);

			float DistFromCenter = ACenter.DistanceTo(BCenter);

			// Collision didn't happen
			if (BMax.x < AMin.x || 
				BMax.y < AMin.y ||
				BMin.x > AMax.x ||
				BMin.y > AMax.y )
			{
				return;
			}
			
			else if (DistFromCenter < TILE_SIZE);
			{
				//////////////////////////////////
				// MINIMUM TRANSLATION DISTANCE //
				//////////////////////////////////

				V2 mtd;

				float left		= BMin.x - AMax.x;
				float right		= BMax.x - AMin.x;
				float top		= BMin.y - AMax.y;
				float bottom	= BMax.y - AMin.y;

				if (abs(left) < right) 
					mtd.x = left;
				else 
					mtd.x = right;

				if (abs(top) < bottom) 
					mtd.y = top;
				else 
					mtd.y = bottom;

				if (abs(mtd.x) < abs(mtd.y)) 
					mtd.y = 0;
				else
					mtd.x = 0;

				// Update velocities based on "bounce" factor
				float bf = 1.0f; // Bounce factor 
				Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
				ColliderVelocity->x = -ColliderVelocity->x * bf;
				ColliderVelocity->y = -ColliderVelocity->y * bf;

				// Hurt Collider
				// Get health and color of entity
				Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
				Enjon::Graphics::ColorRGBA8* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

				if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
				if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");
		
				// Decrement by some arbitrary amount for now	
				HealthComponent->Health -= 10.0f;

				// Change colors based on health	
				if (HealthComponent->Health <= 50.0f) *Color = Enjon::Graphics::RGBA8_Orange();
				if (HealthComponent->Health <= 20.0f) *Color = Enjon::Graphics::RGBA8_Red();

				// Remove entity if no health
				if (HealthComponent->Health <= 0.0f) EntitySystem::RemoveEntity(Manager, B_ID);

				// Remove projectile
				EntitySystem::RemoveEntity(Manager, A_ID);


				// Continue with next entity
				return;
			}

		}

		void CollideWithItem(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
		{
			Component::EntityType AType = Manager->Types[A_ID];
			Component::EntityType BType = Manager->Types[B_ID];

			Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
			Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
			Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
			Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
			
			V2 AMin(A->x, A->y);
			V2 AMax(A->x + TILE_SIZE, A->y + TILE_SIZE);
			V2 BMin(B->x, B->y);
			V2 BMax(B->x + TILE_SIZE, B->y + TILE_SIZE);
				
			V2 ACenter = V2(A->x + TILE_SIZE / 2.0f, A->y + TILE_SIZE / 2.0f);
			V2 BCenter = V2(B->x + TILE_SIZE / 2.0f, B->y + TILE_SIZE / 2.0f);

			float DistFromCenter = ACenter.DistanceTo(BCenter);

			// Collision didn't happen
			if (BMax.x < AMin.x || 
				BMax.y < AMin.y ||
				BMin.x > AMax.x ||
				BMin.y > AMax.y )
			{
				return;
			}
			
			else if (DistFromCenter < TILE_SIZE);
			{
				// Picking up an item
				if (AType == Component::EntityType::ITEM)
				{
					// If there's still space in inventory
					if (Manager->InventorySystem->Inventories[B_ID].Items.size() < MAX_ITEMS)
					{
						printf("Picked up item!\n");
					
						// Place in player inventory
						Manager->InventorySystem->Inventories[B_ID].Items.push_back(A_ID);

						printf("Inventory Size: %d\n", Manager->InventorySystem->Inventories[B_ID].Items.size());
						
						// Turn off render component of item
						bitmask32* Mask = &Manager->Masks[A_ID];

						*Mask &= ~COMPONENT_RENDERER2D;	
						*Mask &= ~COMPONENT_TRANSFORM3D;
					}
					else 
					{
						printf("Inventory already full!\n");
					}

					// Continue to next entity 	
					return;
				}

				if (BType == Component::EntityType::ITEM)
				{
					// If there's still space in inventory
					if (Manager->InventorySystem->Inventories[A_ID].Items.size() < MAX_ITEMS)
					{
						printf("Picked up item!\n");

						// Place in player inventory
						Manager->InventorySystem->Inventories[A_ID].Items.push_back(B_ID);

						printf("Inventory Size: %d\n", Manager->InventorySystem->Inventories[A_ID].Items.size());
						
						// Turn off render and transform component of inventory
						bitmask32* Mask = &Manager->Masks[B_ID];

						*Mask &= ~COMPONENT_RENDERER2D;	
						*Mask &= ~COMPONENT_TRANSFORM3D;
					}
					else
					{
						printf("Inventory already full!\n");
					}

					// Continue to next entity 	
					return;
				}
			}
		}

		// Collide Player with Enemy
		void CollideWithEnemy(Systems::EntityManager* Manager, ECS::eid32 A_ID, ECS::eid32 B_ID)
		{
			Enjon::Math::Vec3* EntityPosition = &Manager->TransformSystem->Transforms[A_ID].Position;
			Enjon::Math::Vec3* ColliderPosition = &Manager->TransformSystem->Transforms[B_ID].Position;
			Enjon::Math::Vec2* A = &Manager->TransformSystem->Transforms[B_ID].CartesianPosition;
			Enjon::Math::Vec2* B = &Manager->TransformSystem->Transforms[A_ID].CartesianPosition;
			
			V2 AMin(A->x, A->y);
			V2 AMax(A->x + TILE_SIZE, A->y + TILE_SIZE);
			V2 BMin(B->x, B->y);
			V2 BMax(B->x + TILE_SIZE, B->y + TILE_SIZE);
				
			V2 ACenter = V2(A->x + TILE_SIZE / 2.0f, A->y + TILE_SIZE / 2.0f);
			V2 BCenter = V2(B->x + TILE_SIZE / 2.0f, B->y + TILE_SIZE / 2.0f);

			float DistFromCenter = ACenter.DistanceTo(BCenter);

			float HitRadius = 64.0f;

			// NOTE(John): Stupid check for now to see if I can get some hits going on
			if (Manager->Masks[A_ID] & COMPONENT_PLAYERCONTROLLER && 
				Animation2D::PlayerState == Animation2D::EntityAnimationState::ATTACKING && Animation2D::HitFrame)
			{
				if (DistFromCenter < HitRadius)
				{
					// Get health and color of entity
					Component::HealthComponent* HealthComponent = &Manager->AttributeSystem->HealthComponents[B_ID];
					Enjon::Graphics::ColorRGBA8* Color = &Manager->Renderer2DSystem->Renderers[B_ID].Color;

					if (HealthComponent == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Collider health component is null");
					if (Color == nullptr) Enjon::Utils::FatalError("COMPONENT_SYSTEMS::COLLISION_SYSTEM::Color component is null");
			
					// Decrement by some arbitrary amount for now	
					HealthComponent->Health -= 10.0f;

					// Change colors based on health	
					if (HealthComponent->Health <= 50.0f) *Color = Enjon::Graphics::RGBA8_Orange();
					if (HealthComponent->Health <= 20.0f) *Color = Enjon::Graphics::RGBA8_Red();

					// If dead, then kill it	
					if (HealthComponent->Health <= 0.0f)
					{

						printf("Manager size: %d\n", Manager->MaxAvailableID);

						static Enjon::Graphics::SpriteSheet ItemSheet; 
						if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));

						// Remove collider
						EntitySystem::RemoveEntity(Manager, B_ID);
						
						// Drop some loot!
						for (int i = 0; i < 10; i++)
						{
							int Roll = Enjon::Random::Roll(0, 5);

							Enjon::Graphics::ColorRGBA8 ItemColor;

							if (Roll == 0) ItemColor = Enjon::Graphics::RGBA8_Red();
							if (Roll == 1) ItemColor = Enjon::Graphics::RGBA8_Orange();
							if (Roll == 2) ItemColor = Enjon::Graphics::RGBA8_Blue();
							if (Roll == 3) ItemColor = Enjon::Graphics::RGBA8_Green();
							if (Roll == 4) ItemColor = Enjon::Graphics::RGBA8_Yellow();
							if (Roll == 5) ItemColor = Enjon::Graphics::RGBA8_Magenta();

							eid32 id = EntitySystem::CreateItem(Manager, Enjon::Math::Vec3(Enjon::Random::Roll(ColliderPosition->x - 64.0f, ColliderPosition->x + 64.0f), 
																  Enjon::Random::Roll(ColliderPosition->y - 64.0f, ColliderPosition->y + 64.0f), 0.0f), 
																  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, "Item", Component::EntityType::ITEM, Enjon::Graphics::SetOpacity(ItemColor, 0.5f));
						} 

					}

					// Debug
					printf("EID: %d, Health: %.2f\n", B_ID, HealthComponent->Health);
				}
			}

			// Collision didn't happen
			if (BMax.x < AMin.x || 
				BMax.y < AMin.y ||
				BMin.x > AMax.x ||
				BMin.y > AMax.y )
			{
				return;
			}
			
			else if (DistFromCenter < TILE_SIZE);
			{
				//////////////////////////////////
				// MINIMUM TRANSLATION DISTANCE //
				//////////////////////////////////

				V2 mtd;

				float left		= BMin.x - AMax.x;
				float right		= BMax.x - AMin.x;
				float top		= BMin.y - AMax.y;
				float bottom	= BMax.y - AMin.y;

				if (abs(left) < right) 
					mtd.x = left;
				else 
					mtd.x = right;

				if (abs(top) < bottom) 
					mtd.y = top;
				else 
					mtd.y = bottom;

				if (abs(mtd.x) < abs(mtd.y)) 
					mtd.y = 0;
				else
					mtd.x = 0;

				*EntityPosition -= Enjon::Math::Vec3(Enjon::Math::CartesianToIso(mtd), EntityPosition->z); 

				// Update velocities based on "bounce" factor
				float bf = 1.0f; // Bounce factor 
				Enjon::Math::Vec3* EntityVelocity = &Manager->TransformSystem->Transforms[A_ID].Velocity; 
				Enjon::Math::Vec3* ColliderVelocity = &Manager->TransformSystem->Transforms[B_ID].Velocity; 
				EntityVelocity->x = -EntityVelocity->x * bf; 
				EntityVelocity->y = -EntityVelocity->y * bf;
				ColliderVelocity->x = -ColliderVelocity->x * bf;
				ColliderVelocity->y = -ColliderVelocity->y * bf;

				// Continue with next entity
				return;
			}
		}

	} // namespace Collision
}}

void printDebug(char* message)
{
	static int counter = 0.0f;
	counter += 0.1f;
	if (counter > 2.0f)
	{
		printf(message);
		counter = 0.0f;
	}
}



