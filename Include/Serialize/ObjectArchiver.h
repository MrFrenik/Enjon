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
			static Result Serialize( const Object* object, ByteBuffer* buffer ); 

			/*
			* @brief
			*/
			Result Deserialize( const String& filePath, Vector< Object* >& out );

			/*
			* @brief
			*/
			Object* Deserialize( const String& filePath );

			/*
			* @brief
			*/
			Result Deserialize( const String& filePath, Object* object );

			/*
			* @brief Takes in a HashMap< const MetaClass*, Vector< Object* > > and a file path to a .easset file to deserialize.
			*			Fills the map out with vectors of objects indexed by class
			*/
			Result Deserialize( const String& filePath, HashMap< const MetaClass*, Vector< Object* > >& out );

			/*
			*@brief
			*/
			virtual Result WriteToFile( const String& filePath );

			/*
			* @brief
			* @note Not a huge fun of this being exposed. Otherwise, why not just use a simple byte buffer?
			*/
			template <typename T>
			void WriteToBuffer( const T& value )
			{
				mBuffer.Write< T >( value ); 
			} 

			/*
			* @brief
			* @note Not a huge fun of this being exposed. Otherwise, why not just use a simple byte buffer?
			*/
			template <typename T>
			T ReadFromBuffer( )
			{
				return mBuffer.Read< T >( ); 
			} 

			/*
			*@brief Takes an existing byte buffer and parses for an object - WILL NOT CALL RESET 
			*/ 
			static Object* Deserialize( ByteBuffer* buffer );

			/*
			*@brief Takes an existing byte buffer, parses the buffer and then fills out the object passed in using that buffer
			*/ 
			static Result Deserialize( ByteBuffer* bufer, Object* object );

		protected:

			/*
			*@brief
			*/ 
			void Reset( ); 

		public:

			/*
			*@brief
			*/ 
			static Result MergeObjects( Object* source, Object* dest, MergeType mergeType );


			/*
			*@brief
			*/ 
			static Result MergeProperty( Object* source, Object* dest, const MetaProperty* prop, MergeType mergeType );

			/*
			*@brief
			*/ 
			static bool HasPropertyOverrides( const Object* obj );

			/*
			*@brief
			*/ 
			static bool HasPropertyOverridesDefault( const Object* obj );

			/*
			*@brief
			*/ 
			static Result RecordAllPropertyOverrides( Object* source, Object* dest ); 

			/*
			* @brief
			*/
			static Result ClearAllPropertyOverrides( Object* obj );

			/*
			* @brief
			*/
			static Result RevertProperty( Object* object, MetaProperty* prop );
			
		protected:

			/*
			*@brief
			*/ 
			Result SerializeObjectDataDefault( const Object* object, const MetaClass* cls );

			/*
			*@brief
			*/ 
			Result DeserializeObjectDataDefault( const Object* object, const MetaClass* cls );

			/*
			*@brief
			*/ 
			static Result SerializeObjectDataDefault( const Object* object, const MetaClass* cls, ByteBuffer* buffer );

			/*
			*@brief
			*/ 
			static Result DeserializeObjectDataDefault( const Object* object, const MetaClass* cls, ByteBuffer* buffer ); 

			/*
			*@brief
			*/ 
			static Result MergeObjectsDefault( Object* source, Object* dest, MergeType mergeType );

			/*
			*@brief
			*/ 
			static Result RecordAllPropertyOverridesDefault( Object* source, Object* dest ); 

			/*
			*@brief
			*/ 
			static Result ClearAllPropertyOverridesDefault( Object* obj );

		protected:
			ByteBuffer mBuffer;
	};
	
}

#endif
