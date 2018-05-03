// @file ImportOptions.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_IMPORT_OPTIONS_H
#define ENJON_IMPORT_OPTIONS_H

#include "Base/Object.h"
#include "System/Types.h"
#include "Defines.h"

namespace Enjon
{
	class ImportOptions
	{
		public:

			/*
			* @brief
			*/
			ImportOptions( ) = default;

			/*
			* @brief
			*/
			~ImportOptions( ) = default;

			/*
			* @brief
			*/
			virtual Result OnEditorView( ) = 0; 

			/*
			* @brief
			*/
			bool IsImporting( ) const;

			/*
			* @brief
			*/
			String GetResourceFilePath( ) const;

			/*
			* @brief
			*/
			String GetDestinationAssetDirectory( ) const;

		protected: 
			String mResourceFilePath;
			String mDestinationAssetDirectory; 
			u32 mIsImporting : 1;
	}; 
}

#endif
