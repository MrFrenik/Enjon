// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ObjectArchiver.h

#pragma once 
#ifndef ENJON_OBJECT_ARCHIVER_H
#define ENJON_OBJECT_ARCHIVER_H 

#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"

namespace Enjon
{
	class ObjectArchiver
	{
		public:

			/*
			* @brief
			*/
			ObjectArchiver( );

			/*
			* @brief
			*/
			~ObjectArchiver( );

			/*
			* @brief
			*/
			Result Serialize( const Object* object );

			/*
			* @brief
			*/
			Result Deserialize( const String& filePath, Vector< Object* >& out );

			/*
			* @brief
			*/
			Result Deserialize( const String& filePath, HashMap< const MetaClass*, Object* >& out );

			/*
			*@brief
			*/
			Result WriteToFile( const String& filePath );

		private:

			/*
			*@brief
			*/
			void Reset( );

		private:
			ByteBuffer mBuffer;
	};
	
}

#endif
