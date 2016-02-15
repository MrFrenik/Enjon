#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include "Animation.h"
#include "Utils/Errors.h"

#include <unordered_map>

namespace AnimationManager {

	// Init the AnimationManager
	void Init();

	// Add animations to the animation manager
	void AddAnimation(std::unordered_map<std::string, Animation>& M, std::string N, Animation A);

	// Get specificed animation with given name
	const Animation* GetAnimation(std::string M, std::string AnimationName);

	// Delete all animations and clear any memory used
	void DeleteAnimations();
}



#endif