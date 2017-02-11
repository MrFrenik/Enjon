#pragma once
#ifndef ENJON_COMPONENT_H
#define ENJON_COMPONENT_H

#define ENJON_COMPONENT(tags)
#define ENJON_OBJECT(tags)

#include "Entity/EntityDefines.h"
#include "Math/Transform.h"
#include "System/Types.h"

#include <array>
#include <vector>
#include <bitset>
#include <type_traits>
#include <unordered_map>

namespace Enjon {

	// Forward declaration
	class EntityHandle;
	class EntityManager;

	class Component
	{
		friend EntityHandle;
		friend EntityManager;

		public:
			Component(){}
			virtual void Update(float dt) = 0;

			Enjon::EntityHandle* GetEntity();
			u32 GetID() const { return mID; }

		private:
			void SetEntity(EntityHandle* entity);
			void SetID(u32 id);

		private:
			Enjon::EntityHandle* mEntity;
			u32 mID;
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

			typedef std::vector<T*> ComponentPtrs;
			typedef std::unordered_map<u32, T> ComponentMap;

			ComponentPtrs mComponentPtrs;
			ComponentMap mComponentMap;
	};

	using ComponentID = u32;

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

	typedef std::bitset<static_cast<u32>(MAX_COMPONENTS)> ComponentBitset;

	template <typename T>
	ComponentBitset GetComponentBitMask() 
	{ 
		ComponentBitset BitSet;
		BitSet.set(static_cast<u32>(GetComponentType<T>()));
		return BitSet;
	}
}

#endif
