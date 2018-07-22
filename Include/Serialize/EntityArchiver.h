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
	class World;

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

		public:

			/*
			* @brief Static method which serializes entity data using an existing ByteBuffer
			*/
			static Result Serialize( const EntityHandle& entity, ByteBuffer* buffer );

			/*
			* @brief Static method which deserializes entity data using an existing ByteBuffer
			*/
			static EntityHandle Deserialize( ByteBuffer* buffer );

			/*
			* @brief Static method which deserializes entity data using an existing ByteBuffer and places into specific world when allocating
			*/
			static EntityHandle Deserialize( ByteBuffer* buffer, World* world );

		protected:

			/*
			* @brief
			*/
			static EntityHandle DeserializeInternal( const EntityHandle& entiy, ByteBuffer* buffer );

		private: 
	};

}

#endif
