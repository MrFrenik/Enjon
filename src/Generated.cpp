#include "Generated.h"
#include "TestComponent.h"

/*
	This file has been generated. All modifications will be lost.
*/

namespace TypeCatalog {

	// Type information as string
	template <typename T>
	const char* TypeName() { return "Uknown"; }

	template <>
	const char* TypeName<PositionComponent>() { return "PositionComponent"; }

	template <>
	const char* TypeName<float>() { return "float"; }

	template <>
	const char* TypeName<VelocityComponent>() { return "VelocityComponent"; }

	template <>
	const char* TypeName<HealthComponent>() { return "HealthComponent"; }

	template <>
	const char* TypeName<DamageComponent>() { return "DamageComponent"; }


	// Get MetaType from class type
	template <typename T>
	MetaType GetMetaType() { return MetaType_Unknown; }

	template <>
	MetaType GetMetaType<PositionComponent>() { return MetaType_PositionComponent; }

	template <>
	MetaType GetMetaType<float>() { return MetaType_float; }

	template <>
	MetaType GetMetaType<VelocityComponent>() { return MetaType_VelocityComponent; }

	template <>
	MetaType GetMetaType<HealthComponent>() { return MetaType_HealthComponent; }

	template <>
	MetaType GetMetaType<DamageComponent>() { return MetaType_DamageComponent; }
}
