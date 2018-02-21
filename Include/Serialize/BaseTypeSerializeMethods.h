// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: BaseTypeSerializeMethods.h

#pragma once 
#ifndef ENJON_PROPERTY_SERIALIZER_ARCHIVER_H
#define ENJON_PROPERTY_SERIALIZER_ARCHIVER_H 

#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"

namespace Enjon
{ 
	class PropertyArchiver
	{
		public: 
			/**
			* @brief
			*/
			static void Serialize( const Object* object, const MetaProperty* property, ByteBuffer* buffer ); 

			/**
			* @brief
			*/
			static void Deserialize( const Object* object, ByteBuffer* buffer );
	};

}

#endif

