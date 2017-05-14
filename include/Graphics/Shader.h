// @file Shader.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SHADER_H
#define ENJON_SHADER_H 

#include "Asset/Asset.h"
#include "System/Types.h"
#include "Defines.h"

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
			Shader( ) {}
			~Shader( ) {}

		private: 
			u32 mProgramID; 
			u32 mVertexShaderID;
			u32 mFragmentShaderID;
	};
}


#endif
