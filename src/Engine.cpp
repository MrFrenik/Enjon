#include "Engine.h"

#include <assert.h>

namespace Enjon
{
	Engine* Engine::mInstance = nullptr;

	//-------------------------------------------------------
	Engine::Engine()
	{
		assert(mInstance == nullptr);
		mInstance = this;
	}	

	//-------------------------------------------------------
	Engine::~Engine()
	{
	}

	//-------------------------------------------------------
	Engine* Engine::GetInstance()
	{
		return mInstance;
	}
}