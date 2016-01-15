#ifndef TESTSYSTEM_H
#define TESTSYSTEM_H

#include "ECS/Components.h"
#include "ECS/ComponentSystems.h"

struct TestSystem 
{
	int x;
};

namespace ECS{ namespace Systems { namespace Test {

	struct TestSystem* NewTestSystem(Systems::EntityManager* Manager);

	void Update(Systems::EntityManager* Manager);

}}}


#endif

