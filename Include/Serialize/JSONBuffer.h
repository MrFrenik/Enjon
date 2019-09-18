#ifndef ENJON_JSON_BUFFER
#define ENJON_JSON_BUFFER

#include "System/Types.h" 
#include "Defines.h" 

// Want to be able to easily read members of json objects as well as serialize them

namespace Enjon
{
	class JSONBuffer
	{ 
		public: 

			/*
			* @brief Default constructor
			*/
			JSONBuffer();

			/*
			* @brief Constructor using file path to read from
			*/
			JSONBuffer( const String& filePath );

			/*
			* @brief Default destructor
			*/
			~JSONBuffer();

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

		protected:

		private:
	}; 
}

#endif