#pragma once
#ifndef ENJON_COMPONENT_H
#define ENJON_COMPONENT_H

#define ENJON_COMPONENT(tags)
#define ENJON_OBJECT(tags)

#include "Entity/EntityDefines.h"
#include "Graphics/PointLight.h"

#include <array>
#include <vector>
#include <bitset>

namespace Enjon {

	enum class CoreComponentType
	{
		COMPONENTTYPE_NONE,
		COMPONENTTYPE_POSITION,
		COMPONENTTYPE_VELOCITY,
		COMPONENTTYPE_TEST,
		COMPONENTTYPE_MOVEMENT,
		COMPONENTTYPE_POINTLIGHT,
		COUNT
	};

	typedef std::bitset<static_cast<size_t>(CoreComponentType::COUNT)> ComponentBitset;

	template <typename T>
	CoreComponentType GetComponentType();

	template <typename T>
	ComponentBitset GetComponentBitMask() 
	{ 
		ComponentBitset BitSet;
		BitSet.set(static_cast<size_t>(GetComponentType<T>()));
		return BitSet;
	}

	// Forward declaration
	class EntityHandle;

	class Component
	{
		public:
			Component(){}
			virtual void Update(float dt) = 0;

			Enjon::EntityHandle* GetEntity();
			void SetEntity(EntityHandle* entity);

		private:
			Enjon::EntityHandle* mEntity;
	};

	class ComponentWrapperBase
	{
		public:
			virtual void Base() = 0;
	};

	template <typename T>
	class ComponentWrapper : public ComponentWrapperBase
	{
		public:
			void Base() override {}

			std::vector<T> Components;
			std::array<uint32_t, MAX_ENTITIES> ComponentIndexMap;
	};

	class PointLightComponent : public Component
	{
		public:
			PointLightComponent(){}
			~PointLightComponent(){}

			virtual void Update(float dt) {}
			Enjon::Graphics::PointLight* GetLight() { return &mLight; }

		private:
			Enjon::Graphics::PointLight mLight;	
	};
}



#endif
