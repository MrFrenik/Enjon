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

	// These do not necessary need to evaluate to executable code, 
	// they just need to evaluate to glsl

	class ShaderGraphNode
	{
		public:
			/**
			* @brief Constructor
			*/
			ShaderGraphNode( ) {}

			/**
			* @brief Destructor
			*/
			~ShaderGraphNode( ) {}

			/**
			* @brief Destructor
			*/
			void Execute( )
			{

			}

			virtual Enjon::String EvaluateToGLSL( )
			{
				return "";
			}

			void AddInput( ShaderGraphNode* node )
			{
				auto query = std::find( mInputs.begin(), mInputs.end(), node );
				if ( query == mInputs.end( ) && mInputs.size() < mMaxNumberInputs )
				{
					mInputs.push_back( node );
				}
			}

			void RemoveInput( ShaderGraphNode* node );

		protected:
			u32 mMaxNumberInputs = 1;
			std::vector< ShaderGraphNode* > mInputs; 
	};

	class ShaderGraphFunctionNode : public ShaderGraphNode
	{
		public: 
			/**
			* @brief Constructor
			*/
			ShaderGraphFunctionNode( ) {}

			/**
			* @brief Destructor
			*/
			~ShaderGraphFunctionNode( ) {}

		protected:
	};

	class BinaryFunctionNode : public ShaderGraphFunctionNode
	{
		public:

			BinaryFunctionNode( )
			{
				mMaxNumberInputs = 2;
			}

			~BinaryFunctionNode( )
			{

			}

		protected:
	};
	
	class UnaryFunctionNode : public ShaderGraphFunctionNode
	{
		public:

			UnaryFunctionNode( )
			{
				mMaxNumberInputs = 1;
			}

			~UnaryFunctionNode( )
			{

			}

		protected:
	};

	class ConstantNode : public ShaderGraphNode 
	{
		public:

			ConstantNode( )
			{
				mMaxNumberInputs = 0;
			}

			ConstantNode( f32 value )
			{
				mValue = value;
				mMaxNumberInputs = 0;
			}

			~ConstantNode( ) { }

			virtual Enjon::String EvaluateToGLSL( ) override
			{
				return std::to_string( mValue );
			}

		protected:
			f32 mValue = 0.0f;
	};

	template < typename T >
	class UniformNode : public ShaderGraphNode
	{ 
		public: 
			UniformNode( )
			{

			}

			~UniformNode( )
			{

			}

		protected: 
			T mData;
	};

	class UniformFloatNode : public UniformNode< f32 > 
	{
		public:
			UniformFloatNode( )
			{
				mMaxNumberInputs = 1;
			}
			
			UniformFloatNode( f32 value )
			{
				mData = value;
				mMaxNumberInputs = 1;
			}

			~UniformFloatNode( ) { }

			Enjon::String EvaluateToGLSL( ) override
			{
				return std::to_string( mData );
			}

		protected: 
	};

	class UniformVec4Node : public UniformNode< Vec4 >
	{
		UniformVec4Node( )
		{
			mMaxNumberInputs = 4;
		}

		UniformVec4Node( const Vec4& vec )
		{
			mData = vec;
			mMaxNumberInputs = 4;
		}

		~UniformVec4Node( ) { }

		Enjon::String EvaluateToGLSL( ) override
		{
			Enjon::String x = std::to_string( mData.x );
			Enjon::String y = std::to_string( mData.y );
			Enjon::String z = std::to_string( mData.z );
			Enjon::String w = std::to_string( mData.w );

			return "vec4(" + x + ", " + y + ", " + z + ", " + w + ")";
		}
	};

	class MultiplyNode : public BinaryFunctionNode
	{
		public:
			virtual Enjon::String EvaluateToGLSL( ) override
			{ 
				Enjon::String inputA = mInputs.at( 0 )->EvaluateToGLSL( );
				Enjon::String inputB = mInputs.at( 1 )->EvaluateToGLSL( );

				return ( inputA + " * " + inputB );
			} 

		protected: 

	};

	class Texture2DNode : public ShaderGraphNode
	{

		public: 
			/**
			* @brief Constructor
			*/
			Texture2DNode( const Enjon::String& textureName )
			{
				mName = textureName;
			}
			
			/**
			* @brief Destructor
			*/
			~Texture2DNode( )
			{

			}

			Enjon::String EvaluateToGLSL( ) override
			{
				return "texture(" + mName + ", texCoords)";
			}

		private: 
			Enjon::String mName; 
	};

	/*
		Really not sure how to structure this...

		Texture2DNode tex1( "tex1" );				// Textures are uniforms, therefore thier names must be unique
		Texture2DNode tex2( "tex2" );
		MultiplyNode mult1;
		AddNode add1;
		UniformFloatNode fl1( "uniformName" );		// Uniform names MUST be unique
		UniformFloatNode fl2( "uniformName2" );

		// This will happen when the nodes are hooked up
		mult1.AddInput( tex1 );
		mult1.AddInput( fl1 );

		add1.AddInput( fl1 );
		add1.AddInput( mult1 );
 
		// Have to hook up to the master node
		BaseColor.SetInput( add1 );
		Normal.SetInput( tex2 );
		

	*/
}

#endif










