// @file ShaderMathNode.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SHADER_MATH_NODE_H
#define ENJON_SHADER_MATH_NODE_H 

#include "Graphics/Shadergraph.h"

namespace Enjon
{
	class ShaderLerpNode : public BinaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderLerpNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderLerpNode( );

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
	
	class ShaderDivideNode : public BinaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderDivideNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderDivideNode( ); 

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
	
	class ShaderAddNode : public BinaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderAddNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderAddNode( ); 

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
	
	class ShaderSubtractNode : public BinaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderSubtractNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderSubtractNode( ); 

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

	class ShaderTimeNode : public ShaderGraphNode 
	{
		public:
			/*
			* @brief
			*/ 
			ShaderTimeNode( const Enjon::String& id );

			/*
			* @brief
			*/ 
			~ShaderTimeNode( );

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
	
	class ShaderSinNode : public UnaryFunctionNode
	{
	public:
		/*
		* @brief Constructor
		*/
		ShaderSinNode( const Enjon::String& id );

		/*
		* @brief Destructor
		*/
		~ShaderSinNode( );

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

	class ShaderOneMinusNode : public UnaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderOneMinusNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderOneMinusNode( );

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

	class ShaderPowerNode : public BinaryFunctionNode
	{
		public:
			/*
			* @brief Constructor
			*/
			ShaderPowerNode( const Enjon::String& id );

			/*
			* @brief Destructor
			*/
			~ShaderPowerNode( );

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
