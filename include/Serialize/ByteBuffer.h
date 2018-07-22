// @file ByteBuffer.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_BYTE_BUFFER_H
#define ENJON_BYTE_BUFFER_H

#include "System/Types.h"
#include "Defines.h"

#define ENJON_BYTE_BUFFER_DEFAULT_CAPACITY	1024

namespace Enjon
{
	enum class BufferStatus
	{
		Invalid,
		ReadyToRead,
		ReadyToWrite
	};

	class ByteBuffer
	{
		public:
			/*
			* @brief Constructor
			*/
			ByteBuffer( );

			/*
			* @brief Constructor
			*/
			ByteBuffer( const String& filePath );

			/*
			* @brief
			*/
			ByteBuffer( const ByteBuffer& other );
			
			/*
			* @brief Destructor
			*/
			~ByteBuffer( );

			/*
			* @brief
			*/
			void ReadFromFile( const Enjon::String& filePath );
			
			/*
			* @brief
			*/
			void WriteToFile( const Enjon::String& filePath );

			/*
			* @brief
			*/
			BufferStatus GetStatus( ) const;

			/*
			* @brief
			*/
			template <typename T>
			T Read( ); 

			/*
			* @brief
			*/
			template < typename T >
			void Write( const T& val );

			/*
			* @brief
			*/
			const u32 GetSize( ) const;

			/*
			* @brief
			*/
			void ReleaseData( );

			/*
			* @brief
			*/
			void Reset( );

			/*
			* @brief
			*/
			void AdvanceReadPosition( const usize& amount );

			/*
			* @brief
			*/
			const u8* GetData( ) const;

			/*
			* @brief
			*/
			void AppendBuffer( const ByteBuffer& other );
 
			/*
			* @brief Is destructive in that it advances the read position of the buffer parameter
			*/
			void AppendBufferFromReadPosition( ByteBuffer* other );

			/*
			* @brief
			*/
			void CopyFromOther( const ByteBuffer& other );

			/*
			* @brief
			*/
			void SetReadPosition( const u32& position );

		private:
			/*
			* @brief Destructor
			*/
			void Resize( usize size );

		private:
			u32 mReadPosition		= 0;
			u32 mWritePosition		= 0;
			u32 mSize				= 0;
			u32 mCapacity			= ENJON_BYTE_BUFFER_DEFAULT_CAPACITY;
			u8* mBuffer				= nullptr;
			BufferStatus mStatus	= BufferStatus::Invalid;
	};
}

#endif
