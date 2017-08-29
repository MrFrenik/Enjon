// @file MetaFunction.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.
#pragma once
#ifndef ENJON_META_FUNCTION_H
#define ENJON_META_FUNCTION_H 

#include "System/Types.h"
#include "Defines.h"

namespace Enjon
{
	class MetaClass;
	class MetaFunction
	{
		friend MetaClass;
		public:
			MetaFunction( );

			~MetaFunction( );

		private:

	};
}


#endif