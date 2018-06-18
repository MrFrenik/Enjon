// File: Archetype.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#ifndef ENJON_ARCHETYPE_H
#define ENJON_ARCHETYPE_H
#pragma once

#include "Asset/Asset.h"

namespace Enjon
{
	ENJON_CLASS( )
	class Archetype : public Asset
	{
		ENJON_CLASS_BODY( Archetype )

		public:
			/**
			* @brief
			*/
			virtual Result SerializeData( ByteBuffer* buffer ) const override;

			/**
			* @brief
			*/
			virtual Result DeserializeData( ByteBuffer* buffer ) override; 

		protected:

			ByteBuffer mEntityData; 
	};
}

/*
	// What does it look like to instantiate an archetype? 
	// What does it look like to create an archetype from an existing entity? 
	// Probably need a way to parent entities within the editor first
*/

#endif
