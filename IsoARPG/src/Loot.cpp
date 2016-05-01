#include "Loot.h"

#include <Graphics/SpriteSheet.h>
#include <Graphics/Color.h>
#include <Math/Maths.h>

#include "ECS/AttributeSystem.h"
#include "ECS/Transform3DSystem.h"
#include "ECS/Masks.h"
#include "ECS/Components.h"
#include "ECS/EntityFactory.h"

#include <stdio.h>

namespace Loot {

	typedef struct 
	{
		Enjon::uint32 Common;
		Enjon::uint32 Uncommon;
		Enjon::uint32 Rare;
		Enjon::uint32 Magic;
		Enjon::uint32 Legend;	
	} RarityCount;

	RarityCount Counts;

	std::unordered_map<std::string, LootProfile> LootProfiles;
	std::unordered_map<std::string, Weapon::WeaponProfile> WeaponProfiles;


	void AddProfile(std::string N, LootProfile L);

	/* Creates drop profiles for entities */	
	void Init()
	{
		Counts = {0, 0, 0, 0, 0};

		AddProfile(std::string("Monster1"), LootProfile{1, 0.40f, 0.0001f, 0.0002f, 0.020f, 0.030f, 0.056f});
		AddProfile(std::string("Monster2"), LootProfile{2, 0.20f, 0.0001f, 0.0005f, 0.035f, 0.025f, 0.055f});

		// Init weapon profiles
		Weapon::Init();
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

		auto Roll = (float)Enjon::Random::Roll(0, 10000) / 10000.0f;
		
		if (Roll > LP->ChanceToDrop) return;	// Leave if no chance to drop an item

		// For now, just show drop types based on item colors
		Enjon::Graphics::ColorRGBA16 ItemColor;

		// Position of entity where item(s) will be dropped
		Enjon::Math::Vec3* Position = &Manager->TransformSystem->Transforms[E].Position;

		for (Enjon::uint32 i = 0; i < DA; i++)
		{
			float Roll = (float)Enjon::Random::Roll(0, 10000) / 10000.0f;
			
			// Legendary 
			if (Roll <= LP->LegendRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Orange();
				Counts.Legend++;
				printf("Making legend...\n");
			}
			// Rare
			else if (Roll <= LP->RareRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Magenta();
				Counts.Rare++;
				printf("Making rare...\n");
			}
			// Magic
			else if (Roll <= LP->MagicRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Blue();
				Counts.Magic++;
				printf("Making magic...\n");
			}
			// Uncommon
			else if (Roll <= LP->UncommonRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Yellow();
				Counts.Uncommon++;
				printf("Making uncommon...\n");
			}
			// Common
			else if (Roll <= LP->CommoneRate)
			{
				ItemColor = Enjon::Graphics::RGBA16_Green();
				Counts.Common++;
				printf("Making common...\n");
			}
			// Otherwise continue the loop and don't drop anything	
			else continue;

			ECS::eid32 id = ECS::Factory::CreateItem(Manager, Enjon::Math::Vec3(Enjon::Random::Roll(Position->x - 64.0f, Position->x + 64.0f), 
												  Enjon::Random::Roll(Position->y - 64.0f, Position->y + 64.0f), 0.0f), 
												  Enjon::Math::Vec2(16.0f, 16.0f), &ItemSheet, (ECS::Masks::Type::ITEM | ECS::Masks::ItemOptions::CONSUMABLE), 
												  ECS::Component::EntityType::ITEM, "Item", ItemColor);
			Manager->TransformSystem->Transforms[id].VelocityGoal.x = 0.0f;
			Manager->TransformSystem->Transforms[id].VelocityGoal.y = 0.0f;
			Manager->TransformSystem->Transforms[id].Velocity = {0.0f, 0.0f, 0.0f};
		}
	}

	void PrintCounts()
	{
		printf("Legend: %d, Rare: %d, Magic: %d, Uncommon: %d, Common: %d\n", Counts.Legend, Counts.Rare, Counts.Magic, Counts.Uncommon, Counts.Common);
	}


	namespace Weapon {

		/* Function Declarations */
		void AddProfile(std::string N, WeaponProfile L);

		/* Creates weapon profiles */	
		void Init()
		{
			Weapon::AddProfile(std::string("Dagger"), WeaponProfile{Range{3, 5}, 150.0f, BaseType::DAGGER, ReachType::UNIDIRECTION});
			Weapon::AddProfile(std::string("Axe"), WeaponProfile{Range{20, 30}, 120.0f, BaseType::DAGGER, ReachType::UNIDIRECTION});
			Weapon::AddProfile(std::string("Bow"), WeaponProfile{Range{5, 10}, 30.0f, BaseType::BOW, ReachType::UNIDIRECTION});
		}

		/* Adds profile to profiles map */
		void AddProfile(std::string N, WeaponProfile L)
		{
			// Will overwrite existing profiles
			WeaponProfiles[N] = L;
		}

		/* Gets specific weapon profile given */
		const WeaponProfile* GetProfile(std::string& N)
		{
			auto it = WeaponProfiles.find(N);
			if (it != WeaponProfiles.end()) return &it->second;
			else return &WeaponProfiles["Dagger"];
		}
	}
}
