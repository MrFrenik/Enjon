// Copyright 2016-2017 John Jackson. All Rights Reserved.
// File: CacheRegistryManifest.h

#pragma once
#ifndef ENJON_CACHE_REGISTRY_MANIFEST_H
#define ENJON_CACHE_REGISTRY_MANIFEST_H 

#include "System/Types.h"
#include "Defines.h"

namespace Enjon
{
	class CacheRegistryManifest
	{
		public:
			/*
			* @brief Default constructor
			*/
			CacheRegistryManifest( ) = default; 

			/*
			* @brief Default destructor
			*/
			~CacheRegistryManifest( ) = default; 

		protected:

			// Should there be a directory structure for this? Should I build a directory structure for the engine now and have this and the asset manager take
			// advantage of that?

			/*
				struct ManifestRecord
				{
					UUID mUUID;					// The uuid of the asset manifest in question
					ByteBuffer mDataBuffer;		// The data buffer used for the individual manifest record
				}
			*/

			/*
			Result Initialize( const String& manifestPath )
			{
				// Open a binary file to parse and do stuff with it
				ReadInManifest( manifestPath );
			}
			*/

			/*
				void ReadInManifest( const String& manifestPath )
				{
					// Should I keep a byte buffer around for this? 
					// This is binary data, after all...
					// Would be useful to be able to search for records in the buffer data and then do updates to it
					// Actually...that's what this manifest could be used for...
					// Keep around various byte buffers for each of the records needed?
				}
			*/

		private:
			String mManifestPath; 
	};
}

#endif
