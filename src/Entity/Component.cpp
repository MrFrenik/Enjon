#include "Entity/Component.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"

namespace Enjon 
{
	Enjon::EntityHandle* Component::GetEntity()
	{
		return mEntity;
	}

	void Component::SetEntity(EntityHandle* entity)
	{
		mEntity = entity;
	}

	template <>
	CoreComponentType GetComponentType<PointLightComponent>() { return CoreComponentType::COMPONENTTYPE_POINTLIGHT; }

	template <>
	CoreComponentType GetComponentType<PointLightComponent*>() { return CoreComponentType::COMPONENTTYPE_POINTLIGHT; }

	template <>
	CoreComponentType GetComponentType<PointLightComponent&>() { return CoreComponentType::COMPONENTTYPE_POINTLIGHT; }
}
