#include "ECS/AIControllerSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/EntityFactory.h"

#include <Graphics/SpriteSheet.h>
#include <Math/Maths.h>

namespace ECS { namespace Systems { namespace AIController {

	struct AIControllerSystem* NewAIControllerSystem(struct EntityManager* Manager)
	{ 
		struct AIControllerSystem* System = new AIControllerSystem;
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


				// Just testing projectiles from enemies
				if (Enjon::Random::Roll(0, 1000000) > 999900)
				{
					// Create an arrow projectile entity for now...
					static Enjon::Graphics::SpriteSheet ItemSheet;
					if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));
					eid32 id = Factory::CreateWeapon(Manager, Enjon::Math::Vec3(b.x, b.y, 40.0f),
											  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, (Masks::Type::WEAPON | 
											  												Masks::GeneralOptions::PICKED_UP),
											  												Component::EntityType::PROJECTILE);
					Manager->Masks[id] |= COMPONENT_TRANSFORM3D;


					// Set arrow velocity to normalize: targetpos - aipos
					// Find vector between the two and normalize
					Enjon::Math::Vec2 ArrowVelocity = Enjon::Math::Vec2::Normalize(a - b);

					float speed = 15.0f;

					// Fire in direction of mouse
					Manager->TransformSystem->Transforms[id].Velocity = speed * Enjon::Math::Vec3(ArrowVelocity.x, ArrowVelocity.y, 0.0f);
					Manager->TransformSystem->Transforms[id].BaseHeight = 0.0f;
				}

			}
		}
	}
}}}




