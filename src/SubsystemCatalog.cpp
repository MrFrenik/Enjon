// Copyright 2016-2017, John Jackson. All Rights Reserved.  
// @file: SubsystemCatalog.cpp

#include "SubsystemCatalog.h"

#include <assert.h>

namespace Enjon
{ 
	SubsystemCatalog* SubsystemCatalog::mInstance = nullptr;

	//======================================================================================================

	SubsystemCatalog::SubsystemCatalog()
	{
		assert(mInstance == nullptr);
		mInstance = this; 
	}
	
	//======================================================================================================

	SubsystemCatalog::~SubsystemCatalog()
	{ 
	}
	
	//======================================================================================================
			
	SubsystemCatalog* SubsystemCatalog::GetInstance() const
	{
		return mInstance;
	}
		
}
