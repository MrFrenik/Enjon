// @file Shader.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SHADER_H
#define ENJON_SHADER_H 

#include "Asset/Asset.h"
#include "System/Types.h"
#include "Math/Vec4.h"
#include "Math/Mat4.h"
#include "Defines.h"

#include <vector>

namespace Enjon
{ 
	template <typename T>
	class ShaderUniform
	{
		public:
			ShaderUniform( ) {}
			~ShaderUniform( ) {}

		private: 
			T mValue;
	};

	class Shader : public Asset
	{
		public:
			/**
			* @brief Constructor
			*/
			Shader( ) {}

			/**
			* @brief Destructor
			*/
			~Shader( ) {}

		private: 
			u32 mProgramID; 
			u32 mVertexShaderID;
			u32 mFragmentShaderID;
	}; 
}

#endif










