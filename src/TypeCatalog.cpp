#include <TypeCatalog.h>
#include <Entity/EntityManager.h>
#include <Entity/Component.h>



// Generated code
namespace TypeCatalog 
{
	template <>
	ComponentType GetType<PositionComponent>() { return ComponentType::COMPONENTTYPE_POSITION; }

	template <>
	ComponentType GetType<PositionComponent&>() { return ComponentType::COMPONENTTYPE_POSITION; }

	template <>
	ComponentType GetType<PositionComponent*>() { return ComponentType::COMPONENTTYPE_POSITION; }

	template <>
	ComponentType GetType<VelocityComponent>()  { return ComponentType::COMPONENTTYPE_VELOCITY; }

	template <>
	ComponentType GetType<VelocityComponent&>()  { return ComponentType::COMPONENTTYPE_VELOCITY; }

	template <>
	ComponentType GetType<TestComponent>() { return ComponentType::COMPONENTTYPE_TEST; }

	template <>
	ComponentType GetType<TestComponent&>() { return ComponentType::COMPONENTTYPE_TEST; }

	template <>
	ComponentType GetType<MovementComponent>() { return ComponentType::COMPONENTTYPE_MOVEMENT; }

	template <>
	ComponentType GetType<MovementComponent&>() { return ComponentType::COMPONENTTYPE_MOVEMENT; }

	template<>
	ComponentType GetType<A>() { return ComponentType::a; }

	template<>
	ComponentType GetType<A&>() { return ComponentType::a; }

	template<>
	ComponentType GetType<B>() { return ComponentType::b; }

	template<>
	ComponentType GetType<B&>() { return ComponentType::b; }

	template<>
	ComponentType GetType<J>() { return ComponentType::j; }

	template<>
	ComponentType GetType<J&>() { return ComponentType::j; }


	RegisteredTypes rt;
};
