// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: ObjectBinarySerializer.h

#pragma once 
#ifndef ENJON_OBJECT_BINARY_SERIALIZER_H
#define ENJON_OBJECT_BINARY_SERIALIZER_H 

#include "Base/Object.h"
#include "Serialize/ByteBuffer.h"

namespace Enjon
{
	class ObjectBinarySerializer
	{
		public:

			/*
			*@brief
			*/
			ObjectBinarySerializer( );

			/*
			*@brief
			*/
			~ObjectBinarySerializer( );

			/*
			*@brief
			*/
			Result Serialize( const Object* object ); 

			/*
			*@brief
			*/
			Result WriteToFile( const String& filePath );

		private:
			ByteBuffer mBuffer;
	};

	class ObjectBinaryDeserializer
	{
		public:

			/*
			* @brief
			*/
			ObjectBinaryDeserializer( ); 

			/*
			*@brief
			*/
			~ObjectBinaryDeserializer( );

			/*
			*@brief
			*/
			Result Deserialize( const String& filePath, Vector< Object* >& objectsOut ); 

			/*
			*@brief
			*/
			template <typename T>
			void GetObjectsOfType( Vector<T*>& objects )
			{
				// Cannot operate on type that does not derive from object
				Object::AssertIsObject<T>( );

				// Get class to search for
				const MetaClass* cls = Object::GetClass<T>( );
				if ( cls && Exists( cls ) )
				{
					// Copy over objects
					objects = mObjects[cls];
				}
			}

		private:
			/*
			*@brief
			*/
			bool Exists( const MetaClass* cls )
			{
				return ( mObjects.find( cls ) != mObjects.end( ) );
			}
			
			/*
			*@brief
			*/
			void Reset( );

			/*
			*@brief
			*/
			void ReleaseData( );

		private:
			HashMap< const MetaClass*, Vector<Object*>> mObjects;
			ByteBuffer mBuffer;
	};
}

#endif
