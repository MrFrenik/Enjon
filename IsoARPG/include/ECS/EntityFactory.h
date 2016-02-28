#ifndef ENTITY_FACTORY_H
#define ENTITY_FACTORY_H

#include "ECS/ComponentSystems.h"

namespace ECS { namespace Factory {

		// Creates a Player entity and returns eid
		eid32 CreatePlayer(Systems::EntityManager* Manager, Enjon::Input::InputManager* Input, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, 
								char* Name = "Player", float VelocityScale = 0.3f, Enjon::Math::Vec3 Velocity = Enjon::Math::Vec3(0,0,0), 
								Enjon::Math::Vec3 VelocityGoal = Enjon::Math::Vec3(0,0,0), float Health = 100.0f, Enjon::Graphics::ColorRGBA16 Color = Enjon::Graphics::RGBA16_White());
	
		// Creates AI entity and returns eid
		eid32 CreateAI(Systems::EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, 
							char* Name = "Enemy", float VelocityScale = 0.3f, Enjon::Math::Vec3 Velocity = Enjon::Math::Vec3(0,0,0), 
							Enjon::Math::Vec3 VelocityGoal = Enjon::Math::Vec3(0,0,0), float Health = 60.0f, Enjon::Graphics::ColorRGBA16 Color = Enjon::Graphics::RGBA16_White());

		// Creates Item entity and returns eid
		eid32 CreateItem(Systems::EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, Masks::EntityMask Mask = Masks::Type::ITEM, 
									Component::EntityType Type = Component::EntityType::ITEM, char* Name = "Item",  Enjon::Graphics::ColorRGBA16 Color = Enjon::Graphics::RGBA16_White());

		// Creates Item entity and returns eid
		eid32 CreateWeapon(Systems::EntityManager* Manager, Enjon::Math::Vec3 Position, Enjon::Math::Vec2 Dimensions, Enjon::Graphics::SpriteSheet* Sheet, Masks::EntityMask Mask = Masks::Type::WEAPON, 
									Component::EntityType Type = Component::EntityType::WEAPON, char* Name = "Weapon",  Enjon::Graphics::ColorRGBA16 Color = Enjon::Graphics::RGBA16_White());
}}

#endif