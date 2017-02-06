#pragma once
#ifndef ENJON_COMPONENT_H
#define ENJON_COMPONENT_H

#define ENJON_COMPONENT(tags)
#define ENJON_OBJECT(tags)

#include "Entity/EntityDefines.h"
#include "Math/Transform.h"

#include <array>
#include <vector>
#include <bitset>
#include <type_traits>

namespace Enjon {

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

	using ComponentID = std::size_t;

	namespace Internal
	{
		inline ComponentID GetUniqueComponentID() noexcept
		{
			static ComponentID lastID{0u};
			return lastID++;
		}
	}

	template <typename T>
	inline ComponentID GetComponentType() noexcept
	{
		static_assert(std::is_base_of<Component, T>::value, 
			"Component:: T must inherit from Component.");	

		static ComponentID typeID{Internal::GetUniqueComponentID()};
		return typeID;
	}

	typedef std::bitset<static_cast<size_t>(MAX_COMPONENTS)> ComponentBitset;

	template <typename T>
	ComponentBitset GetComponentBitMask() 
	{ 
		ComponentBitset BitSet;
		BitSet.set(static_cast<size_t>(GetComponentType<T>()));
		return BitSet;
	}
}

#endif
