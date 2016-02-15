#include "Loot.h"

#include <Graphics/SpriteSheet.h>
#include <Graphics/Color.h>
#include <Math/Maths.h>

#include "ECS/AttributeSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/Masks.h"
#include "ECS/Components.h"
#include "ECS/EntityFactory.h"

namespace Loot {

	std::unordered_map<std::string, LootProfile> LootProfiles;

	void AddProfile(std::string N, LootProfile L);

	/* Creates drop profiles for entities */	
	void Init()
	{
		AddProfile(std::string("Monster1"), LootProfile{1, 0.75f, 0.01f, 0.02f, 0.20f, 0.30f, 0.43f});
		AddProfile(std::string("Monster2"), LootProfile{2, 0.85f, 0.05f, 0.10f, 0.35f, 0.25f, 0.35f});
	}

	/* Adds profile to profiles map */
	void AddProfile(std::string N, LootProfile L)
	{
		// Will overwrite existing profiles
		LootProfiles[N] = L;
	}

	/* Gets specific profile based on entity type */	
	const LootProfile* GetProfile(std::string& N)
	{
		auto it = LootProfiles.find(N);
		if (it != LootProfiles.end()) return &it->second;
		else return &LootProfiles["Monster1"];
	}		

	/* Drops loot based on specified profile */
	void DropLootBasedOnProfile(ECS::Systems::EntityManager* Manager, ECS::eid32 E)
	{
		static Enjon::Graphics::SpriteSheet ItemSheet; 
		if (!ItemSheet.IsInit()) ItemSheet.Init(Enjon::Input::ResourceManager::GetTexture("../IsoARPG/Assets/Textures/orb.png"), Enjon::Math::iVec2(1, 1));

		// Get loot profile
		const LootProfile* LP = Manager->AttributeSystem->LootProfiles[E];

		// Get amount to be dropped
		auto DA = LP->NumOfDrops;

		auto Roll = (float)Enjon::Random::Roll(0, 1000) / 1000.0f;
		
		if (Roll > LP->ChanceToDrop) return;	// Leave if no chance to drop an item

		// For now, just show drop types based on item colors
		Enjon::Graphics::ColorRGBA16 ItemColor;

		// Position of entity where item(s) will be dropped
		Enjon::Math::Vec3* Position = &Manager->TransformSystem->Transforms[E].Position;

		for (Enjon::uint32 i = 0; i < DA; i++)
		{
			Roll = (float)Enjon::Random::Roll(0, 1000) / 1000.0f;	

			// Legendary 
			if (Roll <= LP->LegendRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Orange();
			}
			// Rare
			else if (Roll <= LP->RareRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Magenta();
			}
			// Magic
			else if (Roll <= LP->MagicRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Blue();
			}
			// Uncommon
			else if (Roll <= LP->UncommonRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Yellow();
			}
			// Common
			else
			{
				ItemColor = Enjon::Graphics::RGBA16_Green();
			}

			ECS::eid32 id = ECS::Factory::CreateItem(Manager, Enjon::Math::Vec3(Enjon::Random::Roll(Position->x - 64.0f, Position->x + 64.0f), 
												  Enjon::Random::Roll(Position->y - 64.0f, Position->y + 64.0f), 0.0f), 
												  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, (ECS::Masks::Type::ITEM | ECS::Masks::ItemOptions::CONSUMABLE), 
												  ECS::Component::EntityType::ITEM, "Item", Enjon::Graphics::SetOpacity(ItemColor, 0.5f));
			Manager->TransformSystem->Transforms[id].VelocityGoal.x = 0.0f;
			Manager->TransformSystem->Transforms[id].VelocityGoal.y = 0.0f;
			Manager->TransformSystem->Transforms[id].Velocity = {0.0f, 0.0f, 0.0f};
		}
	}
}
