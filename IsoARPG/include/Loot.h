#ifndef LOOT_H
#define LOOT_H

#include <System/Types.h>
#include <Math/Maths.h>

#include "ECS/ComponentSystems.h"

#include <unordered_map>

// drop profile: 
// amount of drops
// chance to drop common, uncommon, magic, rare, legend
// 

typedef struct
{
	Enjon::uint32 NumOfDrops;
	float ChanceToDrop;
	float LegendRate;
	float RareRate;
	float MagicRate;
	float UncommonRate;
	float CommoneRate;
} LootProfile;

namespace Loot {

	/* Creates drop profiles for entities */	
	void Init();		

	/* Gets specific profile based on entity type */	
	const LootProfile* GetProfile(std::string& N);

	/* Drops loot based on specified profile */
	void DropLootBasedOnProfile(ECS::Systems::EntityManager* Manager, ECS::eid32 E);
}



#endif