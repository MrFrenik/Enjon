#include "AnimationManager.h"

#include <unordered_map>

namespace AnimationManager {

	// NOTE(John): I'm too fucking tired / lazy to do this correctly right now, so just fix it later...

	typedef struct 
	{
		Animate::Animation Walk;
		Animate::Animation Attack_Dagger;
		Animate::Animation Attack_Bow;
	} Player_Animations;

	Player_Animations PlayerAnims;

	/* Map of player animations */	
	std::unordered_map<std::string, Animate::Animation> PlayerAnimations;

	/* Init the AnimationManager */
	void Init()
	{
		// Player animations
		AddAnimation(PlayerAnimations, std::string("walk"), Animate::CreateAnimation(Animate::CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "walk")));
		AddAnimation(PlayerAnimations, std::string("attack_dagger"), Animate::CreateAnimation(Animate::CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_dagger")));
		AddAnimation(PlayerAnimations, std::string("attack_bow"), Animate::CreateAnimation(Animate::CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_bow")));
		AddAnimation(PlayerAnimations, std::string("attack_axe"), Animate::CreateAnimation(Animate::CreateAnimationProfile("../IsoARPG/Profiles/Animations/Player/player.txt", "attack_axe")));
	}

	/* Add animations to the animation manager */
	void AddAnimation(std::unordered_map<std::string, Animate::Animation>& M, std::string N, Animate::Animation A)
	{
		// Insert into map
		// NOTE(John): Will overwrite the pre-existing kv pair if already exists!
		M[N] = A;
	}

	/* Get specificed Animate::Animation with given name */
	const Animate::Animation* GetAnimation(std::string M, std::string AnimationName)
	{
		if (M == "Player")
		{
			auto it = PlayerAnimations.find(AnimationName);
			if (it != PlayerAnimations.end()) return &it->second;
			else printf("Not found!");
			// else return &PlayerAnimations["walk"];
		}
		
		else return &PlayerAnimations["walk"];
	}

	/* Delete all animations and clear any memory used */
	void DeleteAnimations()
	{

	}


}