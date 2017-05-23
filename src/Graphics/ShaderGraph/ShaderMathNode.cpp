// @file ShaderMathNode.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/ShaderGraph/ShaderMathNode.h"

namespace Enjon
{ 
	//================================================================================================================

	ShaderMultiplyNode::ShaderMultiplyNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{
	}

	//================================================================================================================

	ShaderMultiplyNode::~ShaderMultiplyNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderMultiplyNode::EvaluateOutputType( u32 portID )
	{
		if ( mInputs.size( ) < 2 )
		{
			return ShaderOutputType::Float;
		}

		Connection a_conn = mInputs.at( 0 );
		Connection b_conn = mInputs.at( 1 );

		// Look at inputs to determine output type of this node
		ShaderGraphNode* a = const_cast<ShaderGraphNode*>( mInputs.at( 0 ).mOwner );
		ShaderGraphNode* b = const_cast<ShaderGraphNode*>( mInputs.at( 1 ).mOwner );

		// Evaluate output type
		ShaderOutputType aType = a->EvaluateOutputType( a_conn.mOutputPortID );
		ShaderOutputType bType = b->EvaluateOutputType( b_conn.mOutputPortID ); 

		switch ( aType )
		{
			case ShaderOutputType::Float:
			{
				switch ( bType )
				{
					case ShaderOutputType::Float: mOutputType = ShaderOutputType::Float; break;
					case ShaderOutputType::Vec2: mOutputType = ShaderOutputType::Vec2; break;
					case ShaderOutputType::Vec3: mOutputType = ShaderOutputType::Vec3; break;
					case ShaderOutputType::Vec4: mOutputType = ShaderOutputType::Vec4; break;
				}

			} break;

			case ShaderOutputType::Vec2:
			{
				if ( bType == ShaderOutputType::Vec2 || bType == ShaderOutputType::Float )
				{
					mOutputType = ShaderOutputType::Vec2;
				}
				else
				{
					// Throw error here!
				}

			} break;

			case ShaderOutputType::Vec3:
			{
				if ( bType == ShaderOutputType::Vec3 || bType == ShaderOutputType::Float )
				{
					mOutputType = ShaderOutputType::Vec3;
				}
				else
				{
					// Throw error
				}
			} break;

			case ShaderOutputType::Vec4:
			{
				if ( bType == ShaderOutputType::Vec4 || bType == ShaderOutputType::Float )
				{
					mOutputType = ShaderOutputType::Vec4; 
				}
				else
				{
					// Throw error
				}
			} break; 
		}

		return mOutputType;
	}

	//================================================================================================================

	Enjon::String ShaderMultiplyNode::EvaluateToGLSL( )
	{
		Enjon::String finalEvaluation = "";

		// Evaluate inputs
		for ( auto& c : mInputs )
		{
			ShaderGraphNode* owner = const_cast<ShaderGraphNode*>( c.mOwner );
			finalEvaluation += owner->Evaluate( ) + "\n"; 
		}

		// Get connections
		Connection a_conn = mInputs.at( 0 );
		Connection b_conn = mInputs.at( 1 );

		// Get shader graph nodes
		ShaderGraphNode* a = const_cast<ShaderGraphNode*>( a_conn.mOwner );
		ShaderGraphNode* b = const_cast<ShaderGraphNode*>( b_conn.mOwner );

		// Evaluate final line of code
		finalEvaluation += GetQualifiedID( ) + " = " + a->EvaluateAtPort( a_conn.mOutputPortID ) + " * " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ";";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderMultiplyNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderMultiplyNode::GetDeclaration( )
	{
		switch ( EvaluateOutputType( ) )
		{
			case ShaderOutputType::Float: return "float " + mID + ";"; break;
			case ShaderOutputType::Vec2: return "vec2 " + mID + ";"; break;
			case ShaderOutputType::Vec3: return "vec3 " + mID + ";"; break;
			case ShaderOutputType::Vec4: return "vec4 " + mID + ";"; break;
			default: return "";
		}
	} 

	//================================================================================================================
			
	ShaderDotProductNode::ShaderDotProductNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{ 
	}

	//================================================================================================================

	ShaderDotProductNode::~ShaderDotProductNode( )
	{ 
	}

	//================================================================================================================

	ShaderOutputType ShaderDotProductNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Float; 
	}

	//================================================================================================================

	Enjon::String ShaderDotProductNode::EvaluateToGLSL( )
	{
		Enjon::String finalEvaluation = "";

		// Evaluate inputs
		for ( auto& c : mInputs )
		{
			ShaderGraphNode* owner = const_cast< ShaderGraphNode* >( c.mOwner );
			finalEvaluation += owner->Evaluate( ) + "\n";
		}

		// Get connections
		Connection a_conn = mInputs.at( 0 );
		Connection b_conn = mInputs.at( 1 );

		// Get shader graph nodes
		ShaderGraphNode* a = const_cast< ShaderGraphNode* >( a_conn.mOwner );
		ShaderGraphNode* b = const_cast< ShaderGraphNode* >( b_conn.mOwner );

		// Evaluate final line of code
		finalEvaluation += GetQualifiedID( ) + " = dot(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ", " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderDotProductNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderDotProductNode::GetDeclaration( )
	{
		return "float " + mID + ";";
	}

	//================================================================================================================
		
	ShaderNormalizeNode::ShaderNormalizeNode( const Enjon::String& id )
		: UnaryFunctionNode( id )
	{ 
	}

	//================================================================================================================

	ShaderNormalizeNode::~ShaderNormalizeNode( )
	{ 
	}

	//================================================================================================================

	ShaderOutputType ShaderNormalizeNode::EvaluateOutputType( u32 portID )
	{
		if ( mInputs.size( ) < 1 )
		{
			return ShaderOutputType::Float;
		}

		Connection a_conn = mInputs.at( 0 );

		// Look at inputs to determine output type of this node
		ShaderGraphNode* a = const_cast< ShaderGraphNode* >( mInputs.at( 0 ).mOwner );

		// Evaluate output type
		ShaderOutputType aType = a->EvaluateOutputType( a_conn.mOutputPortID );

		switch ( aType )
		{
			case ShaderOutputType::Float:
			{ 
				mOutputType = ShaderOutputType::Float;

			} break;

			case ShaderOutputType::Vec2:
			{
				mOutputType = ShaderOutputType::Vec2; 
			} break;

			case ShaderOutputType::Vec3:
			{
				mOutputType = ShaderOutputType::Vec3;
			} break;

			case ShaderOutputType::Vec4:
			{
				mOutputType = ShaderOutputType::Vec4;
			} break;
			default:
			{
				mOutputType = ShaderOutputType::Float;
			} break;
		}

		return mOutputType;
	}

	//================================================================================================================

	Enjon::String ShaderNormalizeNode::EvaluateToGLSL( )
	{ 
		Enjon::String finalEvaluation = "";

		// Evaluate inputs
		for ( auto& c : mInputs )
		{
			ShaderGraphNode* owner = const_cast< ShaderGraphNode* >( c.mOwner );
			finalEvaluation += owner->Evaluate( ) + "\n";
		}

		// Get connections
		Connection a_conn = mInputs.at( 0 );

		// Get shader graph nodes
		ShaderGraphNode* a = const_cast< ShaderGraphNode* >( a_conn.mOwner );

		// Evaluate final line of code
		finalEvaluation += GetQualifiedID( ) + " = normalize(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderNormalizeNode::EvaluateAtPort( u32 portID )
	{
		// Only one output, so just get qualified id
		return GetQualifiedID( ); 
	}

	//================================================================================================================

	Enjon::String ShaderNormalizeNode::GetDeclaration( )
	{
		switch ( EvaluateOutputType( ) )
		{
			case ShaderOutputType::Float: return "float " + mID + ";"; break;
			case ShaderOutputType::Vec2: return "vec2 " + mID + ";"; break;
			case ShaderOutputType::Vec3: return "vec3 " + mID + ";"; break;
			case ShaderOutputType::Vec4: return "vec4 " + mID + ";"; break;
			default: return "";
		}
	}

	//================================================================================================================
}
