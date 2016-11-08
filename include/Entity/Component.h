#ifndef ENJON_COMPONENT_H
#define ENJON_COMPONENT_H
#pragma once

#define ENJON_COMPONENT(tags)
#define ENJON_OBJECT(tags)

struct vec3
{
	vec3(){}
	vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	float x;
	float y;
	float z;
};

struct ComponentBase
{
	virtual void Base() = 0;
	struct EntityHandle* Entity;
};

template <typename T>
struct Component : public ComponentBase
{
	void Base()
	{
		static_cast<T*>(this)->Base();
	}
};

ENJON_OBJECT("Component")
struct PositionComponent : Component<PositionComponent>
{
	void Base() {}
	PositionComponent(){}
	PositionComponent(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	float x;
	float y;
	float z;
};

ENJON_OBJECT("Component")
struct VelocityComponent : Component<VelocityComponent>
{
	void Base() {}

	VelocityComponent(){}
	VelocityComponent(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	float x;
	float y;
	float z;
};

ENJON_OBJECT("Component")
struct TestComponent : Component<TestComponent>
{
	void Base(){}

	vec3 a;
};

ENJON_OBJECT("Component")
struct MovementComponent : Component<MovementComponent>
{
	void Base(){}

	vec3 Position;
	vec3 Velocity;
};

ENJON_OBJECT("Component")
struct J : Component<J>
{
	void Base(){}

	vec3 MemberVarible;
};

ENJON_OBJECT("Component")
struct A : Component<A>
{
	void Base(){}

	vec3 MemberVarible;
};

ENJON_OBJECT("Component")
struct B : Component<B>
{
	void Base(){}

	vec3 MemberVarible;
};

#endif
