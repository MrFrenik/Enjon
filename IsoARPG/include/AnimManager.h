#ifndef ANIM_MANAGER_H
#define ANIM_MANAGER_H

#include <Graphics/Animations.h>
#include <Defines.h>

#include <unordered_map>

namespace AnimManager {

	// Init the AnimationManager
	void Init();

	// Add animations to the animation manager
	void AddAnimation(std::unordered_map<std::string, EA::Anim*>& M, std::string N, EA::Anim* A);

	// Get specificed animation with given name
	EA::Anim* GetAnimation(std::string AnimationName);

	// Delete all animations and clear any memory used
	void DeleteAnimations();

	// Get access to map for traversal
	std::unordered_map<std::string, EA::Anim*>* GetAnimationMap();
}

#endif