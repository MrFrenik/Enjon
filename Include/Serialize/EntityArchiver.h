// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: EntityArchiver.h

#pragma once 
#ifndef ENJON_ENTITY_ARCHIVER_H
#define ENJON_ENTITY_ARCHIVER_H 

#include "Base/Object.h" 
#include "Serialize/ByteBuffer.h"
#include "Serialize/ObjectArchiver.h"
#include "Entity/EntityManager.h"

namespace Enjon
{
	class Entity; 

	class EntityArchiver : public ObjectArchiver
	{
		public: 

			/*
			* @brief
			*/
			EntityArchiver( ) = default;

			/*
			* @brief
			*/
			~EntityArchiver( ) = default;

			/*
			* @brief
			*/
			Result Serialize( const EntityHandle& entity );

			/*
			* @brief
			*/
			Result Deserialize( const String& filePath, Vector< EntityHandle >& out );

			/*
			* @brief
			*/
			EntityHandle Deserialize( const String& filePath );

			/*
			* @brief
			*/
			EntityHandle Deserialize( ByteBuffer* buffer );

		protected:

		private: 
	};

}

#endif
