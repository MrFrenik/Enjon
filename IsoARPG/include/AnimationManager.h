#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include "Animation.h"
#include "Utils/Errors.h"

namespace AnimationManager {

	// Init the AnimationManager
	void Init();

	// Add animations to the animation manager
	void AddAnimation();

	// Get specificed animation with given name
	const Animation* GetAnimation(const char* AnimationName);

	// Delete all animations and clear any memory used
	void DeleteAnimations();
}



#endif