#ifndef TEST_COMPONENT_H
#define TEST_COMPONENT_H

#include "Defines.h"

#include <cstdint>

typedef uint32_t u32;

class ComponentBase
{
public:
	virtual void Init() = 0;
	u32 entity;
	const char* TypeName;
};

template <typename T>
class Component : public ComponentBase
{
	void Init(){}
};

ENJON_CLASS("Category=Component", "Base=Component")
class PositionComponent : public Component<PositionComponent>
{
public:
	ENJON_FUNCTION()
	PositionComponent(float _x, float _y, float _z);

	float x;
	float y;
	float z;
};

ENJON_CLASS("Category=Component", "Base=Component")
class VelocityComponent : public Component<VelocityComponent>
{
public:
	ENJON_FUNCTION()
	VelocityComponent(float _x, float _y, float _z);

	float x;
	float y;
	float z;
};

ENJON_CLASS("Category=Component", "Base=Component")
class HealthComponent : public Component<HealthComponent>
{
public:
	ENJON_FUNCTION()
	HealthComponent(float _amount);

	float Amount;
};

ENJON_CLASS("Category=Component", "Base=Component")
class DamageComponent : public Component<DamageComponent>
{
public:
	ENJON_FUNCTION()
	DamageComponent(float _amount);

	float Amount;
};

#endif

