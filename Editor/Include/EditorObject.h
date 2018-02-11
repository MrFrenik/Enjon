// File: EditorObject.h
// Copyright 2016-2018 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_EDITOR_OBJECT_H
#define ENJON_EDITOR_OBJECT_H 

#include <Base/Object.h>

namespace Enjon
{
	class EditorObject : public Enjon::Object
	{ 
		public:

			/**
			* @brief
			*/
			EditorObject( ) = default;

			/**
			* @brief
			*/
			~EditorObject( ) = default;

			/**
			* @brief
			*/
			virtual void Update( ) = 0;

		private: 

			/**
			* @brief
			*/
			virtual void ProcessInput( ) = 0;

		protected: 
	};
};

#endif