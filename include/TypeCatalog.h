#ifndef ENJON_TYPE_CATALOG_H
#define ENJON_TYPE_CATALOG_H
#pragma once

// struct PositionComponent;
// struct VelocityComponent;
// struct TestComponent;
// struct MovementComponent;
// struct PointLightComponent;

// #include <bitset>

/*
// Generated code
namespace TypeCatalog 
{
	enum class ObjectMetaType
	{
		None,
		PositionComponent,
		VelocityComponent,
		TestComponent,
		MovementComponent,
		PointLightComponent,
		COUNT
	};


	enum class ComponentType
	{
		COMPONENTTYPE_NONE,
		COMPONENTTYPE_POSITION,
		COMPONENTTYPE_VELOCITY,
		COMPONENTTYPE_TEST,
		COMPONENTTYPE_MOVEMENT,
		COMPONENTTYPE_POINTLIGHT,
		COUNT
	};

	typedef std::bitset<static_cast<size_t>(ComponentType::COUNT)> ComponentBitset;

	template <typename T>
	ComponentType GetType();

	template <typename T>
	ComponentBitset GetBitMask() 
	{ 
		ComponentBitset BitSet;
		BitSet.set(static_cast<size_t>(GetType<T>()));
		return BitSet;
	}
};
*/

#endif