#include "ECS/ComponentSystems.h"
#include "Utils/Errors.h"
#include "Math/Random.h"
#include "Graphics/SpriteSheet.h"
#include "IO/ResourceManager.h"

#include <stdlib.h>
#include <stdio.h>
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
			Manager->TransformSystem		= TransformSystem::NewTransform3DSystem(Manager);
			Manager->PlayerControllerSystem = PlayerController::NewPlayerControllerSystem(Manager);
			Manager->AIControllerSystem		= AIController::NewAIControllerSystem(Manager);
			Manager->LabelSystem			= Label::NewLabelSystem(Manager);
			Manager->Animation2DSystem		= Animation2D::NewAnimation2DSystem(Manager);
			Manager->AttributeSystem 		= Attributes::NewAttributeSystem(Manager);
			Manager->Renderer2DSystem 		= Renderer2D::NewRenderer2DSystem(Manager);
			Manager->InventorySystem 		= Inventory::NewInventorySystem(Manager);

			Manager->Width = Width;
			Manager->Height = Height;

			// Initialize spatial hash
			Manager->Grid = new SpatialHash::Grid;
			SpatialHash::Init(Manager->Grid, Manager->Width, Manager->Height);

			return Manager;
		} 

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
					Manager->Length++;
				}
				else
				{
					// Otherwise set next to length, which is the greatest unused id
					Manager->NextAvailableID = Manager->MaxAvailableID;
				} 
				
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
			// Set all of the entity's components to 0
			bitmask32 Mask = Manager->Masks[Entity];
			if ((Mask & COMPONENT_TRANSFORM3D) == COMPONENT_TRANSFORM3D) Manager->TransformSystem->Transforms[Entity].Velocity = Enjon::Math::Vec3(0.0f, 0.0f, 0.0f);

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
			Transform->ViewVector = Enjon::Math::Vec2(0, 0);
			Transform->GroundPosition = Enjon::Math::Vec2(Position.x + Dimensions.x / 2.0f, Position.y);
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;

			// Set up Input
			Manager->PlayerControllerSystem->PlayerControllers[Player].Input = Input;
			
			// Set up Animation2D
			Manager->Animation2DSystem->Animations[Player].Sheet = Sheet;
			Manager->Animation2DSystem->Animations[Player].CurrentFrame = 0; 
			Manager->Animation2DSystem->Animations[Player].BeginningFrame = 0; 

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
			Transform->GroundPosition = Enjon::Math::Vec2(Position.XY());
			Transform->CartesianPosition = Enjon::Math::IsoToCartesian(Transform->GroundPosition);
			Transform->BaseHeight = Position.z;

			// Set up Animation2D
			Manager->Animation2DSystem->Animations[AI].Sheet = Sheet; 
			Manager->Animation2DSystem->Animations[AI].CurrentFrame = 0; 
			Manager->Animation2DSystem->Animations[AI].BeginningFrame = 0; 
			
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
				if ((Manager->Masks[e] & COMPONENT_TRANSFORM3D) == COMPONENT_TRANSFORM3D)
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
					if (Transform->CartesianPosition.x < -Width + TileWidth) { Transform->CartesianPosition.x = -Width + TileWidth; Velocity->x *= -1; }   
					if (Transform->CartesianPosition.x > 0.0f) { Transform->CartesianPosition.x = 0.0f; Velocity->x *= -1; }
					if (Transform->CartesianPosition.y > 0.0f) { Transform->CartesianPosition.y = 0.0f; Velocity->y *= -1; }
					if (Transform->CartesianPosition.y < -Height + TileWidth) { Transform->CartesianPosition.y = -Height + TileWidth; Velocity->y *= -1; }

					*GroundPosition = Enjon::Math::CartesianToIso(Transform->CartesianPosition);
					Position->y = GroundPosition->y + Position->z;
					Position->x = GroundPosition->x - Transform->Dimensions.x / 2.0f + TileWidth;

					static float time = 0.0f;
					if (Manager->LabelSystem->Labels[e].Name == "Player")
					{
						time += 0.1f;
						if (time > 1.0f)
						{ 
							//const Enjon::Math::Vec2* CartesianPos = &Transform->CartesianPosition;
							//printf("Cartesian Position: %.2f, %.2f\n", CartesianPos.x, CartesianPos.y);
						}
					}
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
		enum EntityAnimationState { WALKING, ATTACKING }; // This should be split up into continuous and discrete states
		EntityAnimationState PlayerState = EntityAnimationState::WALKING;
		bool HitFrame = false;
		
		struct Animation2DSystem* NewAnimation2DSystem(struct EntityManager* Manager)
		{
			struct Animation2DSystem* System = (struct Animation2DSystem*)malloc(sizeof(Animation2DSystem));
			System->Manager = Manager;
			return System;
		}
		
		void Update(struct Animation2DSystem* System)
		{
			// How are we going to update our sytems?
			// I guess we'll just, uh, try some shit out
			struct EntityManager* Manager = System->Manager;
			for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
			{
				// If has an animation component
				if ((Manager->Masks[e] & COMPONENT_ANIMATION2D) == COMPONENT_ANIMATION2D)
				{
					// If is a player
					if ((Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER) == COMPONENT_PLAYERCONTROLLER)
					{
						Component::Animation2D* Animation = &Manager->Animation2DSystem->Animations[e];
						const Enjon::Math::Vec2* ViewVector = &Manager->TransformSystem->Transforms[e].ViewVector;
						const Enjon::Math::Vec3* Velocity = &Manager->TransformSystem->Transforms[e].Velocity;
						Enjon::uint32* BeginningFrame = &Animation->BeginningFrame;
						Enjon::uint32 EndFrame = *BeginningFrame + Animation->Sheet->dims.x;
						Enjon::uint32 XDim = Animation->Sheet->dims.x;
						
						// static variables for testing purposes
						// TODO(John): Get this shit out of here and store it appropriately in Math constants or some shit like that
						static float time = 0.0f;
						static Enjon::Math::Vec2 E(1, 0);
						static Enjon::Math::Vec2 NE(1, 1);
						static Enjon::Math::Vec2 N(0, 1);
						static Enjon::Math::Vec2 NW(-1, 1);
						static Enjon::Math::Vec2 W(-1, 0);
						static Enjon::Math::Vec2 SW(-1, -1);
						static Enjon::Math::Vec2 S(0, -1);
						static Enjon::Math::Vec2 SE(1, -1); 

						// Increment time
						// TODO(John): Put this as a variable in a JSON file; it will be the specified delay between two consecutive frames
						static float increment = 1.0f;
						static float frametimer = 0.0f;
						static int RowOffset = 0;
						static bool SetStart = false;

						// NOTE(John): Okay, so this might work, but it will get pretty crazy, since I'll have to calculate the starting position for all these animations
						
						if (PlayerState == EntityAnimationState::WALKING) RowOffset = 0;
						else if (PlayerState == EntityAnimationState::ATTACKING) RowOffset = 8;
					
						// Setting animation beginning frame based on view vector
						if (PlayerState == EntityAnimationState::ATTACKING && !SetStart)
						{
							if		(ViewVector->x <= 0)   {*BeginningFrame = XDim * (0 + RowOffset); SetStart = true;}
							else if (ViewVector->x > 0)    {*BeginningFrame = XDim * (1 + RowOffset); SetStart = true;}
						
							// Set currentframe to beginning frame
							Animation->CurrentFrame = 0;
						}
						else if (PlayerState == EntityAnimationState::WALKING)
						{
							if		(*ViewVector == NW)		*BeginningFrame = XDim * (0 + RowOffset);
							else if (*ViewVector == NE)		*BeginningFrame = XDim * (1 + RowOffset);
							else if (*ViewVector == E)		*BeginningFrame = XDim * (2 + RowOffset);
							else if (*ViewVector == W)		*BeginningFrame = XDim * (3 + RowOffset);
							else if (*ViewVector == N)		*BeginningFrame = XDim * (4 + RowOffset);
							else if (*ViewVector == S)		*BeginningFrame = XDim * (5 + RowOffset);
							else if (*ViewVector == SE)		*BeginningFrame = XDim * (6 + RowOffset);
							else if (*ViewVector == SW)		*BeginningFrame = XDim * (7 + RowOffset);
							else							*BeginningFrame = XDim * (0 + RowOffset); 
						}
						
					
						if (PlayerState == EntityAnimationState::WALKING)
						{
							if (abs(Velocity->x) > WALKPACE || abs(Velocity->y) >= WALKPACE) time += 0.5f;
							else time += 0.4f;	
						
							if (time >= 3.0f && (Velocity->x != 0 || Velocity->y != 0))
							{ 
								Animation->CurrentFrame++;
								if (Animation->CurrentFrame + *BeginningFrame >= EndFrame)
								{
									Animation->CurrentFrame = 0;
								}
								
								// Reset time 
								time = 0.0f;
							}
						} 
						else if (PlayerState == EntityAnimationState::ATTACKING)
						{
							time += 0.1f;
							if (time >= increment)
							{
								if (*BeginningFrame + Animation->CurrentFrame != *BeginningFrame + 2)
								{
									increment = 0.3f;
									Animation->CurrentFrame++;
								}
							
								// Hold the upward swing frame a bit
								if (*BeginningFrame + Animation->CurrentFrame == *BeginningFrame + 2)
								{
									frametimer += 0.1f; // this is the delay attribute which will be help in the data file for the animation
									if (frametimer >= 0.7f)
									{
										Animation->CurrentFrame++;
										frametimer = 0.0f;
									}
								}
								// Activate collision with sword
								if (*BeginningFrame + Animation->CurrentFrame == *BeginningFrame + 3)
								{
									// Collision at this point
									HitFrame = true;

									// Try adding a collision box for the weapon here

								} 
								else HitFrame = false;

								if (*BeginningFrame + Animation->CurrentFrame >= *BeginningFrame + 5)
								{
									Animation->CurrentFrame = 5;
									frametimer += 0.1f;
								if (frametimer >= 0.5f)
									{
										Animation->CurrentFrame = 0;
										frametimer = 0.0f;
										time = 0.0f;
										PlayerState = EntityAnimationState::WALKING;
										SetStart = false;
										increment = 1.0f;
									}
								}
								time = 0.0f;
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
				if ((Manager->Masks[e] & COMPONENT_PLAYERCONTROLLER) == COMPONENT_PLAYERCONTROLLER)
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

		// Updates all possible collisions
		void Update(struct EntityManager* Manager)
		{
			static Enjon::uint32 times;
			times = 0; 
			
			// Check the quadrants of entities and then check for collisions
			for (eid32 e = 0; e < Manager->MaxAvailableID; e++)
			{ 
				// If entity has no transform, then continue
				if ((Manager->Masks[e] & COMPONENT_TRANSFORM3D) != COMPONENT_TRANSFORM3D) continue;
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
						if ((Manager->Masks[collider] & COMPONENT_TRANSFORM3D) == COMPONENT_TRANSFORM3D && collider != e)
						{
							// Get EntityType of collider and entity
							Component::EntityType AType = Manager->Types[collider];
							Component::EntityType BType = Manager->Types[e];

							// Get collision mask for A and B
							Enjon::uint32 Mask = GetCollisionType(Manager, e, collider);

							// If enemy and item, then continue to next pair
							if (Mask == (COLLISION_ITEM | COLLISION_ENEMY)) continue;
							if (Mask == (COLLISION_ITEM | COLLISION_PLAYER)) 	{ CollideWithItem(Manager, collider, e); 	continue; } 
							if (Mask == (COLLISION_ENEMY | COLLISION_PLAYER)) 	{ CollideWithEnemy(Manager, e, collider); 	continue; }
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
				case Component::EntityType::ITEM:		Mask |= COLLISION_ITEM; 		break;
				case Component::EntityType::PLAYER:		Mask |= COLLISION_PLAYER; 		break;
				case Component::EntityType::ENEMY: 		Mask |= COLLISION_ENEMY; 		break;
				default: 								Mask |= COLLISION_NONE;			break; 
			}	

			// // Or the mask with TypeB collision
			switch(*TypeB)
			{
				case Component::EntityType::ITEM:		Mask |= COLLISION_ITEM; 		break;
				case Component::EntityType::PLAYER:		Mask |= COLLISION_PLAYER; 		break;
				case Component::EntityType::ENEMY: 		Mask |= COLLISION_ENEMY; 		break;
				default: 								Mask |= COLLISION_NONE;			break; 
			}

			return Mask;	
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

						if ((*Mask & COMPONENT_RENDERER2D) == COMPONENT_RENDERER2D) *Mask ^= COMPONENT_RENDERER2D;	
						if ((*Mask & COMPONENT_TRANSFORM3D) == COMPONENT_TRANSFORM3D) *Mask ^= COMPONENT_TRANSFORM3D;
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
						
						// Turn off render component of inventory
						bitmask32* Mask = &Manager->Masks[B_ID];

						if ((*Mask & COMPONENT_RENDERER2D) == COMPONENT_RENDERER2D) *Mask ^= COMPONENT_RENDERER2D;	
						if ((*Mask & COMPONENT_TRANSFORM3D) == COMPONENT_TRANSFORM3D) *Mask ^= COMPONENT_TRANSFORM3D;
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
			if ((Manager->Masks[A_ID] & COMPONENT_PLAYERCONTROLLER) == COMPONENT_PLAYERCONTROLLER && 
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
						
						// Drop some loot!
						for (int i = 0; i < 5; i++)
						{
							eid32 id = EntitySystem::CreateItem(Manager, Enjon::Math::Vec3(Enjon::Random::Roll(ColliderPosition->x + 128.0f, ColliderPosition->x + 256.0f), 
																  Enjon::Random::Roll(ColliderPosition->y + 128.0f, ColliderPosition->y + 256.0f), 0.0f), 
																  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, "Item");
							printf("Manager size: %d, New ID: %d\n", Manager->MaxAvailableID, id);
						} 

						// Remove collider
						EntitySystem::RemoveEntity(Manager, B_ID);

						EntitySystem::CreateItem(Manager, Enjon::Math::Vec3(Enjon::Random::Roll(ColliderPosition->x + 128.0f, ColliderPosition->x + 256.0f), 
															  Enjon::Random::Roll(ColliderPosition->y + 128.0f, ColliderPosition->y + 256.0f), 0.0f), 
															  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, "Item");
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



