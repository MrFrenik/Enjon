#ifndef COMPONENT_SYSTEMS_H
#define COMPONENT_SYSTEMS_H

#include <SDL2/SDL.h> 

#include "Graphics/Camera2D.h"
#include "Physics/AABB.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"

#include "Masks.h"
#include "SpatialHash.h"
#include "AnimationManager.h"

#include <vector>

#define SCREEN_VERTICAL_CENTER			1440 / 2
#define SCREEN_HORIZONTAL_CENTER		900 / 2

#define WALKPACE	3.0f
#define SPRINTPACE  WALKPACE * 2.0f

struct TestSystem;
struct LabelSystem;
struct AttributeSystem;
struct PlayerControllerSystem;
struct Transform3DSystem;
struct CollisionSystem;
struct Animation2DSystem;
struct InventorySystem;
struct Renderer2DSystem;
struct AIControllerSystem;

// EntityManagers hold pointers to their system, which hold the arrays of data
// ComponentSystems don't need to know of mangers...or do they? Yes, they'll hold pointers to the managers of which they belong
namespace ECS { namespace Systems {

	const float TILE_SIZE = 32.0f;
	
	struct EntityManager
	{
		eid32 NextAvailableID;
		eid32 MaxAvailableID;
		eid32 Length;

		struct Transform3DSystem* TransformSystem;
		struct AIControllerSystem* AIControllerSystem;
		struct PlayerControllerSystem* PlayerControllerSystem;
		struct Animation2DSystem* Animation2DSystem;
		struct LabelSystem* LabelSystem;
		struct CollisionSystem* CollisionSystem;
		struct AttributeSystem* AttributeSystem;
		struct Renderer2DSystem* Renderer2DSystem;
		struct InventorySystem* InventorySystem;
		struct TestSystem* TestSystem;

		bitmask32 Masks[MAX_ENTITIES];
		Component::EntityType Types[MAX_ENTITIES];

		eid32 Player;

		int Width; 
		int Height;

		SpatialHash::Grid* Grid;
		Enjon::Graphics::Camera2D* Camera;
	};
	
	// struct AIControllerSystem
	// {
	// 	struct EntityManager* Manager;
	// 	Component::AIController AIControllers[MAX_ENTITIES];
	// };

	////////////////////
	// Entity Manager //
	////////////////////
	
	namespace EntitySystem 
	{ 
		// Creates and returns instance of an Entity Manager
		// TODO(John): Write custom allocators instead of using malloc/delete
		struct EntityManager* NewEntityManager(int Width, int Height, Enjon::Graphics::Camera2D* Camera); 
		
		// Creates a blank entity, returns the eid and places in manager
		eid32 CreateEntity(struct EntityManager* Manager, bitmask32 Components);

		// Removes entity from manager by setting its bitfield to COMPONENT_NONE
		void RemoveEntity(struct EntityManager* Manager, eid32 Entity); 

		// Returns whether or not entity's bitfield is set to COMPONENT_NONE or not
		bool IsAlive(struct EntityManager* Manager, eid32 Entity);
	
		// Creates a Player entity and returns eid
		eid32 CreatePlayer(struct EntityManager* Manager, Enjon::Input::InputManager* Input, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, 
								char* Name = "Player", float VelocityScale = 0.3f, Enjon::Math::Vec3 Velocity = Enjon::Math::Vec3(0,0,0), 
								Enjon::Math::Vec3 VelocityGoal = Enjon::Math::Vec3(0,0,0), float Health = 100.0f, Enjon::Graphics::ColorRGBA8 Color = Enjon::Graphics::RGBA8_White());
	
		// Creates AI entity and returns eid
		eid32 CreateAI(struct EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, 
							char* Name = "Enemy", float VelocityScale = 0.3f, Enjon::Math::Vec3 Velocity = Enjon::Math::Vec3(0,0,0), 
							Enjon::Math::Vec3 VelocityGoal = Enjon::Math::Vec3(0,0,0), float Health = 100.0f, Enjon::Graphics::ColorRGBA8 Color = Enjon::Graphics::RGBA8_White());

		// Creates Item entity and returns eid
		eid32 CreateItem(struct EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, Masks::EntityMask Mask = Masks::Type::ITEM, 
									Component::EntityType Type = Component::EntityType::ITEM, char* Name = "Item",  Enjon::Graphics::ColorRGBA8 Color = Enjon::Graphics::RGBA8_White());

		// Turns off component
		void RemoveComponents(struct EntityManager* Manager, eid32 Entity, bitmask32 Components);

		// Adds components
		void AddComponents(bitmask32 Components);
	}
	
	////////////////////////
	// AIControllerSystem //
	////////////////////////

	// namespace AIController
	// { 
	// 	// Constructs and returns new AIControllerSystem
	// 	// TODO(John): Write custom allocator for this
	// 	struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager);
		
	// 	// Updates Controller of AI it is attached to
	// 	void Update(struct AIControllerSystem* System, eid32 Player);	
	// }
}}



#endif