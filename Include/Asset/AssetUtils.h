
// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: AssetUtils.h

#pragma once
#ifndef ENJON_ASSET_UTILS_H 
#define ENJON_ASSET_UTILS_H 

#include "Base/Object.h"

namespace Enjon
{
	class Asset;

	class AssetUtils
	{
		public: 

			static Asset* GetDefaultAsset( const MetaClass* cls ); 
	};
}

#endif
