// @file ShaderMathNode.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SHADER_MATH_NODE_H
#define ENJON_SHADER_MATH_NODE_H 

#include "Graphics/Shadergraph.h"

namespace Enjon
{
	class ShaderMultiplyNode : public BinaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderMultiplyNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderMultiplyNode( ); 

			/*
			* @brief
			*/
			virtual ShaderOutputType EvaluateOutputType( u32 portID = 0 ) override;

			/*
			* @brief
			*/
			virtual Enjon::String EvaluateToGLSL( ) override;

			/*
			* @brief
			*/
			virtual Enjon::String EvaluateAtPort( u32 portID ) override;

			/*
			* @brief
			*/
			virtual Enjon::String GetDeclaration( ) override;

		protected:

	};

	class ShaderDotProductNode : public BinaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderDotProductNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderDotProductNode( ); 

			/*
			* @brief
			*/
			virtual ShaderOutputType EvaluateOutputType( u32 portID = 0 ) override;

			/*
			* @brief
			*/
			virtual Enjon::String EvaluateToGLSL( ) override;

			/*
			* @brief
			*/
			virtual Enjon::String EvaluateAtPort( u32 portID ) override;

			/*
			* @brief
			*/
			virtual Enjon::String GetDeclaration( ) override;

		protected:

	};

	class ShaderNormalizeNode : public UnaryFunctionNode
	{
	public:
		/*
		* @brief Constructor
		*/
		ShaderNormalizeNode( const Enjon::String& id );

		/*
		* @brief Destructor
		*/
		~ShaderNormalizeNode( );

		/*
		* @brief
		*/
		virtual ShaderOutputType EvaluateOutputType( u32 portID = 0 ) override;

		/*
		* @brief
		*/
		virtual Enjon::String EvaluateToGLSL( ) override;

		/*
		* @brief
		*/
		virtual Enjon::String EvaluateAtPort( u32 portID ) override;

		/*
		* @brief
		*/
		virtual Enjon::String GetDeclaration( ) override;

	protected:

	};
	
}

#endif
