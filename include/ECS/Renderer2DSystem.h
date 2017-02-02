#ifndef ENJON_RENDERER_2D_SYSTEM_H
#define ENJON_RENDERER_2D_SYSTEM_H

#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ComponentSystems.h"

#include <vector>

struct Renderer2DSystem
{
	ECS::Systems::EntityManagerDeprecated* Manager;
	ECS::Component::Renderer2DComponent Renderers[MAX_ENTITIES];
	std::vector<Enjon::uint32> Entities;
};

namespace ECS { namespace Systems { namespace Renderer2D {

	// Updates Renderers of EntityManagerDeprecated
	void Update(struct EntityManagerDeprecated* Manager);

	// Create new Render2DSystem
	Renderer2DSystem* NewRenderer2DSystem(struct EntityManagerDeprecated* Manager);	
}}}


#endif

