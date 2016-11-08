#ifndef ENJON_TYPE_CATALOG_H
#define ENJON_TYPE_CATALOG_H
#pragma once

struct PositionComponent;
struct VelocityComponent;
struct TestComponent;
struct MovementComponent;
struct A; 
struct B; 
struct J;

#include <bitset>

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
		a, 
		b, 
		j,
		COUNT
	};


	enum class ComponentType
	{
		COMPONENTTYPE_NONE,
		COMPONENTTYPE_POSITION,
		COMPONENTTYPE_VELOCITY,
		COMPONENTTYPE_TEST,
		COMPONENTTYPE_MOVEMENT,
		a, 
		b,
		j, 
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

	struct TypeBase
	{
		virtual void Base() = 0;
		ObjectMetaType MetaType;
	};

	template <typename T>
	struct Type : public TypeBase
	{
		void Base() override {}
		T ConcreteType;
	};

	struct NoneType
	{
	};

	struct RegisteredTypes
	{
		NoneType* r0;
		PositionComponent* r1;
		VelocityComponent* r2;
		TestComponent* r3;
		MovementComponent* r4;
		A* r5; 
		B* r6; 
		J* r7;
	};


};

#endif