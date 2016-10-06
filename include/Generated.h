#ifndef ENJON_GENERATED_H
#define ENJON_GENERATED_H

/*
	This file has been generated. All modifications will be lost.
*/

namespace TypeCatalog
{
	enum MetaType
	{
		MetaType_PositionComponent,
		MetaType_float,
		MetaType_VelocityComponent,
		MetaType_HealthComponent,
		MetaType_DamageComponent,
	};

	// Type information as string
	template <typename T>
	const char* TypeName();

	// Get MetaType from class type
	template <typename T>
	MetaType GetMetaType();

} // TypeCatalog

#endif

