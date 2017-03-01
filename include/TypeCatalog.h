#pragma once
#ifndef ENJON_TYPECATALOG_H
#define ENJON_TYPECATALOG_H

#include "System/Types.h"

#include <unordered_map>

namespace Enjon
{
	class TypeCatalog
	{
		template <typename T>
		void Register(T type);
	};
}


#endif