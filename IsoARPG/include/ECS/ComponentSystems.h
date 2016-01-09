#ifndef COMPONENT_SYSTEMS_H
#define COMPONENT_SYSTEMS_H

#include "ECS/Components.h"
#include <ECS/Entity.h>
#include <SpatialHash.h>

#include <SDL2/SDL.h> 

#define SCREEN_VERTICAL_CENTER			1440 / 2
#define SCREEN_HORIZONTAL_CENTER		900 / 2

// EntityManagers hold pointers to their system, which hold the arrays of data
// ComponentSystems don't need to know of mangers...or do they? Yes, they'll hold pointers to the managers of which they belong
namespace ECS { namespace Systems {
	
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

		bitmask32 Masks[MAX_ENTITIES];
		Component::EntityType Types[MAX_ENTITIES];

		int Width; 
		int Height;

		SpatialHash::Grid* Grid;
	}; 

	struct AttributeSystem
	{
		struct EntityManager* Manager;
		Component::HealthComponent HealthComponents[MAX_ENTITIES];
	};
	
	struct PlayerControllerSystem
	{
		struct EntityManager* Manager;
		Component::PlayerController PlayerControllers[MAX_ENTITIES];
	}; 
		
	struct Transform3DSystem
	{
		struct EntityManager* Manager;
		Component::Transform3D Transforms[MAX_ENTITIES];
	}; 

	struct CollisionSystem
	{
		struct EntityManager* Manager;
	};
	
	struct InventorySystem
	{
		struct EntityManager* Manager;
		Component::InventoryComponent Inventories[MAX_ENTITIES];
	}; 
	
	struct AIControllerSystem
	{
		struct EntityManager* Manager;
		Component::AIController AIControllers[MAX_ENTITIES];
	}; 
	
	struct Animation2DSystem
	{
		struct EntityManager* Manager;
		Component::Animation2D Animations[MAX_ENTITIES];
	}; 
	
	struct LabelSystem
	{
		struct EntityManager* Manager;
		Component::Label Labels[MAX_ENTITIES];
	};

	struct Renderer2DSystem
	{
		struct EntityManager* Manager;
		Component::Renderer2DComponent Renderers[MAX_ENTITIES];
	};

	////////////////////
	// Entity Manager //
	////////////////////
	
	namespace EntitySystem 
	{ 
		// Creates and returns instance of an Entity Manager
		// TODO(John): Write custom allocators instead of using malloc/delete
		struct EntityManager* NewEntityManager(int Width, int Height); 
		
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
		eid32 CreateItem(struct EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, char* Name, 
								Component::EntityType Type = Component::EntityType::ITEM, Enjon::Graphics::ColorRGBA8 Color = Enjon::Graphics::RGBA8_White());
	} 

	/////////////////////
	// TransformSystem //
	///////////////////// 

	// Reponsible for managing all the entity transforms 
	namespace TransformSystem 
	{ 
		// Updates Transforms of EntityManager
		void Update(Transform3DSystem* System);

		// Creates new Transform3DSystem
		Transform3DSystem* NewTransform3DSystem(struct EntityManager* Manager);		
	}
	
	/////////////////////
	// CollisionSystem //
	/////////////////////

	namespace Collision
	{
		// Updates all possible collisions
		void Update(struct EntityManager* Manager);
	}
	
	/////////////////////
	// InventorySystem //
	///////////////////// 

	// Reponsible for managing all the inventories 
	namespace Inventory 
	{ 
		// Updates Transforms of EntityManager
		void Update(InventorySystem* System);

		// Creates new Transform3DSystem
		InventorySystem* NewInventorySystem(struct EntityManager* Manager);		
	}
	
	////////////////////
	// RendererSystem //
	////////////////////

	// Reponsible for managing all the renderers
	namespace Renderer2D
	{ 
		// Updates Renderers of EntityManager
		void Update(struct EntityManager* Manager);

		// Create new Render2DSystem
		Renderer2DSystem* NewRenderer2DSystem(struct EntityManager* Manager);	
	}
	
	///////////////////////
	// Animation2DSystem //
	/////////////////////// 

	// Reponsible for managing all the renderers
	namespace Animation2D
	{ 
		// Updates Transforms of EntityManager
		void Update(struct Animation2DSystem* System);	
		
		// Creates new Transform3DSystem
		Animation2DSystem* NewAnimation2DSystem(struct EntityManager* Manager);		
	}
	
	////////////////////////////
	// PlayerControllerSystem //
	//////////////////////////// 

	namespace PlayerController
	{ 
		// Constructs and returns new PlayerControllerSystem
		// TODO(John): Write custom allocator for this
		struct PlayerControllerSystem* NewPlayerControllerSystem(struct EntityManager* Manager);
		
		// Updates Controller of player it is attached to
		void Update(struct PlayerControllerSystem* System);	
	} 
	
	////////////////////////
	// AIControllerSystem //
	////////////////////////

	namespace AIController
	{ 
		// Constructs and returns new AIControllerSystem
		// TODO(John): Write custom allocator for this
		struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager);
		
		// Updates Controller of AI it is attached to
		void Update(struct AIControllerSystem* System, eid32 Player);	
	} 
	
	/////////////////
	// LabelSystem //
	/////////////////

	namespace Label
	{ 
		// Constructs and returns new AIControllerSystem
		// TODO(John): Write custom allocator for this
		struct LabelSystem* NewLabelSystem(struct EntityManager* Manager);
		
		// Updates Controller of AI it is attached to
		void Update(struct LabelSystem* System);	
	} 

	/////////////////////
	// AttributeSystem //
	/////////////////////

	namespace Attributes
	{
		// Constructs and returns new AIControllerSystem
		// TODO(John): Write custom allocator for this
		struct AttributeSystem* NewAttributeSystem(struct EntityManager* Manager);

		// Updates Attributes of entity that it's attached to
		void Update(struct AttributeSystem* System);	
	}
}}



#endif