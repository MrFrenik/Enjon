#include "ECS/TestSystem.h"

#include <stdio.h>

namespace ECS { namespace Systems { namespace Test {

	struct TestSystem* NewTestSystem(Systems::EntityManager* Manager)
	{
		struct TestSystem* System = new TestSystem;
		printf("Worked!\n");
		return System;
	}

	void Update(Systems::EntityManager* Manager)
	{
		
	}

}}}
