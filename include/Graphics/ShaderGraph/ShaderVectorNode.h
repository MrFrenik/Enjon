// @file ShaderVectorNode.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SHADER_VECTOR_NODE_H
#define ENJON_SHADER_VECTOR_NODE_H 

#include "Graphics/ShaderGraph.h" 
#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Vec4.h"

namespace Enjon
{
	class ShaderVec2Node : public ShaderPrimitiveNode< Vec2 >
	{
	public:
		/*
		* @brief Default Constructor
		*/
		ShaderVec2Node( const Enjon::String& id );

		/*
		* @brief Constructor
		*/
		ShaderVec2Node( const Enjon::String& id, const Vec2& vec );

		/*
		* @brief Destructor
		*/
		~ShaderVec2Node( );

		/*
		* @brief
		*/
		Enjon::String EvaluateToGLSL( ) override;

		/*
		* @brief
		*/
		virtual Enjon::String GetDeclaration( ) override;
	};
	
	class ShaderVec3Node : public ShaderPrimitiveNode< Vec3 >
	{
	public:
		/*
		* @brief Default Constructor
		*/
		ShaderVec3Node( const Enjon::String& id );

		/*
		* @brief Constructor
		*/
		ShaderVec3Node( const Enjon::String& id, const Vec3& vec );

		/*
		* @brief Destructor
		*/
		~ShaderVec3Node( );

		/*
		* @brief
		*/
		Enjon::String EvaluateToGLSL( ) override;

		/*
		* @brief
		*/
		virtual Enjon::String GetDeclaration( ) override;
	};
	
	class ShaderVec4Node : public ShaderPrimitiveNode< Vec4 >
	{
	public:
		/*
		* @brief Default Constructor
		*/
		ShaderVec4Node( const Enjon::String& id );

		/*
		* @brief Constructor
		*/
		ShaderVec4Node( const Enjon::String& id, const Vec4& vec );

		/*
		* @brief Destructor
		*/
		~ShaderVec4Node( );

		/*
		* @brief
		*/
		Enjon::String EvaluateToGLSL( ) override;

		/*
		* @brief
		*/
		virtual Enjon::String GetDeclaration( ) override;
	};
}

#endif
