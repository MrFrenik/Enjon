#include "TestComponent.h"
#include "Generated.h"

#include "IO/InputManager.h"

#include <iostream>

PositionComponent::PositionComponent(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
{
	this->TypeName = TypeCatalog::TypeName<PositionComponent>(); 
};

VelocityComponent::VelocityComponent(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
{
	this->TypeName = TypeCatalog::TypeName<VelocityComponent>(); 
};

HealthComponent::HealthComponent(float _amount) : Amount(_amount)
{
	this->TypeName = TypeCatalog::TypeName<HealthComponent>(); 
};

DamageComponent::DamageComponent(float _amount) : Amount(_amount)
{
	this->TypeName = TypeCatalog::TypeName<DamageComponent>(); 
};

