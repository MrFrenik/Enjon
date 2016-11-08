#include <Entity/EntityManager.h>
#include <Entity/Component.h>

#include <array>
#include <vector>
#include <assert.h>

#include <stdio.h>

// Generated? Possibly, that way we can do the stupid shit I'm about to do...
// Would really like a cleaner way that doesn't expose all of this to the entity manager
// Whatever... for now, this is just how it is until I get smarter about it
// But I'm tired of trying to figure out how to do this correctly
void EntityManager::DetachAll(EntityHandle* Entity)
{
	if (Entity->HasComponent<PositionComponent>()) 		Entity->Detach<PositionComponent>();
	if (Entity->HasComponent<VelocityComponent>()) 		Entity->Detach<VelocityComponent>();
	if (Entity->HasComponent<TestComponent>()) 			Entity->Detach<TestComponent>();
	if (Entity->HasComponent<MovementComponent>()) 		Entity->Detach<TestComponent>();
	if (Entity->HasComponent<A>())						Entity->Detach<A>();
	if (Entity->HasComponent<B>()) 						Entity->Detach<B>();
	if (Entity->HasComponent<J>()) 						Entity->Detach<J>();
}

	// void EntityManager::DetachAll(EntityHandle* Entity)
	// {
	// 	// Grab id
	// 	auto ID = Entity->ID;

	// 	TypeCatalog::RegisteredTypes rt;

	// 	for (auto i = 0; i < static_cast<size_t>(TypeCatalog::ComponentType::COUNT); i++)
	// 	{
	// 		// Get component wrapper
	// 		// auto CWrapper = static_cast<ComponentWrapper<decltype(member)>*>(Components.at(static_cast<size_t>(TypeCatalog::GetType<decltype(member)>())));
	// 	}

	// }
