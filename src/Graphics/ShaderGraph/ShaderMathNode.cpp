// @file ShaderMathNode.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/ShaderGraph/ShaderMathNode.h"

namespace Enjon
{ 
	//================================================================================================================

	ShaderLerpNode::ShaderLerpNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{
	}

	//================================================================================================================

	ShaderLerpNode::~ShaderLerpNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderLerpNode::EvaluateOutputType( u32 portID )
	{
		if ( mInputs.size( ) < 2 )
		{
			return ShaderOutputType::Float;
		}

		Connection a_conn = mInputs.at( 0 );
		Connection b_conn = mInputs.at( 1 );

		// Look at inputs to determine output type of this node
		ShaderGraphNode* a = const_cast< ShaderGraphNode* >( mInputs.at( 0 ).mOwner );
		ShaderGraphNode* b = const_cast< ShaderGraphNode* >( mInputs.at( 1 ).mOwner );

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
			mOutputType = ShaderOutputType::Vec3;
			//if ( bType == ShaderOutputType::Vec3 || bType == ShaderOutputType::Float )
			//{
			//	mOutputType = ShaderOutputType::Vec3;
			//}
			//else
			//{
			//	// Throw error
			//}
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

	Enjon::String ShaderLerpNode::EvaluateToGLSL( )
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
		Connection c_conn = mInputs.at( 2 );

		// Get shader graph nodes
		ShaderGraphNode* a = const_cast< ShaderGraphNode* >( a_conn.mOwner );
		ShaderGraphNode* b = const_cast< ShaderGraphNode* >( b_conn.mOwner );
		ShaderGraphNode* c = const_cast< ShaderGraphNode* >( c_conn.mOwner );

		// Evaluate final line of code
		finalEvaluation += GetQualifiedID( ) + " = mix(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ", " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ", " + c->EvaluateAtPort( c_conn.mOutputPortID ) + ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderLerpNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderLerpNode::GetDeclaration( )
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
				mOutputType = ShaderOutputType::Vec3;
				//if ( bType == ShaderOutputType::Vec3 || bType == ShaderOutputType::Float )
				//{
				//	mOutputType = ShaderOutputType::Vec3;
				//}
				//else
				//{
				//	// Throw error
				//}
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

	ShaderDivideNode::ShaderDivideNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{
	}

	//================================================================================================================

	ShaderDivideNode::~ShaderDivideNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderDivideNode::EvaluateOutputType( u32 portID )
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
				mOutputType = ShaderOutputType::Vec3;
				//if ( bType == ShaderOutputType::Vec3 || bType == ShaderOutputType::Float )
				//{
				//	mOutputType = ShaderOutputType::Vec3;
				//}
				//else
				//{
				//	// Throw error
				//}
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

	Enjon::String ShaderDivideNode::EvaluateToGLSL( )
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
		finalEvaluation += GetQualifiedID( ) + " = " + a->EvaluateAtPort( a_conn.mOutputPortID ) + " / " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ";";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderDivideNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderDivideNode::GetDeclaration( )
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

	ShaderSubtractNode::ShaderSubtractNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{ 
	}

	//================================================================================================================

	ShaderSubtractNode::~ShaderSubtractNode( )
	{ 
	}

	//================================================================================================================

	ShaderOutputType ShaderSubtractNode::EvaluateOutputType( u32 portID )
	{ 
		if ( mInputs.size( ) < 2 )
		{
			return ShaderOutputType::Float;
		}

		Connection a_conn = mInputs.at( 0 );
		Connection b_conn = mInputs.at( 1 );

		// Look at inputs to determine output type of this node
		ShaderGraphNode* a = const_cast< ShaderGraphNode* >( mInputs.at( 0 ).mOwner );
		ShaderGraphNode* b = const_cast< ShaderGraphNode* >( mInputs.at( 1 ).mOwner );

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
			mOutputType = ShaderOutputType::Vec3;
			//if ( bType == ShaderOutputType::Vec3 || bType == ShaderOutputType::Float )
			//{
			//	mOutputType = ShaderOutputType::Vec3;
			//}
			//else
			//{
			//	// Throw error
			//}
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

	Enjon::String ShaderSubtractNode::EvaluateToGLSL( )
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
		finalEvaluation += GetQualifiedID( ) + " = " + a->EvaluateAtPort( a_conn.mOutputPortID ) + " - " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ";";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderSubtractNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderSubtractNode::GetDeclaration( )
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

	ShaderAddNode::ShaderAddNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{ 
	}

	//================================================================================================================

	ShaderAddNode::~ShaderAddNode( )
	{ 
	}

	//================================================================================================================

	ShaderOutputType ShaderAddNode::EvaluateOutputType( u32 portID )
	{ 
		if ( mInputs.size( ) < 2 )
		{
			return ShaderOutputType::Float;
		}

		Connection a_conn = mInputs.at( 0 );
		Connection b_conn = mInputs.at( 1 );

		// Look at inputs to determine output type of this node
		ShaderGraphNode* a = const_cast< ShaderGraphNode* >( mInputs.at( 0 ).mOwner );
		ShaderGraphNode* b = const_cast< ShaderGraphNode* >( mInputs.at( 1 ).mOwner );

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
			mOutputType = ShaderOutputType::Vec3;
			//if ( bType == ShaderOutputType::Vec3 || bType == ShaderOutputType::Float )
			//{
			//	mOutputType = ShaderOutputType::Vec3;
			//}
			//else
			//{
			//	// Throw error
			//}
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

	Enjon::String ShaderAddNode::EvaluateToGLSL( )
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
		finalEvaluation += GetQualifiedID( ) + " = " + a->EvaluateAtPort( a_conn.mOutputPortID ) + " + " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ";";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderAddNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderAddNode::GetDeclaration( )
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
		
	ShaderAbsoluteValueNode::ShaderAbsoluteValueNode( const Enjon::String& id )
		: UnaryFunctionNode( id )
	{ 
	}

	//================================================================================================================

	ShaderAbsoluteValueNode::~ShaderAbsoluteValueNode( )
	{ 
	}

	//================================================================================================================

	ShaderOutputType ShaderAbsoluteValueNode::EvaluateOutputType( u32 portID )
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

	Enjon::String ShaderAbsoluteValueNode::EvaluateToGLSL( )
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
		finalEvaluation += GetQualifiedID( ) + " = abs(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderAbsoluteValueNode::EvaluateAtPort( u32 portID )
	{
		// Only one output, so just get qualified id
		return GetQualifiedID( ); 
	}

	//================================================================================================================
	
	Enjon::String ShaderAbsoluteValueNode::GetDeclaration( )
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

	ShaderTimeNode::ShaderTimeNode( const Enjon::String& id ) 
		: ShaderGraphNode( id )
	{ 
		mMaxNumberInputs = 0;
		mMaxNumberOutputs = 1;
		mPrimitiveType = ShaderPrimitiveType::Float;
		mOutputType = ShaderOutputType::Float;
	}

	//================================================================================================================

	ShaderTimeNode::~ShaderTimeNode( )
	{ 
	}

	//================================================================================================================

	ShaderOutputType ShaderTimeNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Float;
	}

	//================================================================================================================

	Enjon::String ShaderTimeNode::EvaluateToGLSL( )
	{
		// Don't need to evaluate, as it has no inputs
		return "";
	}

	//================================================================================================================

	Enjon::String ShaderTimeNode::EvaluateAtPort( u32 portID )
	{
		// Only one output, so just get qualified id
		return "uWorldTime"; 
	}

	//================================================================================================================

	Enjon::String ShaderTimeNode::GetDeclaration( )
	{
		// Already declared as uniform, so no need
		return "";
	}

	//================================================================================================================

	ShaderSinNode::ShaderSinNode( const Enjon::String& id ) 
		: UnaryFunctionNode( id )
	{ 
	}

	//================================================================================================================

	ShaderSinNode::~ShaderSinNode( )
	{ 
	}

	//================================================================================================================

	ShaderOutputType ShaderSinNode::EvaluateOutputType( u32 portID )
	{ 
		// Only one output type possible
		return ShaderOutputType::Float;
	}

	//================================================================================================================

	Enjon::String ShaderSinNode::EvaluateToGLSL( )
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
		finalEvaluation += GetQualifiedID( ) + " = sin(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderSinNode::EvaluateAtPort( u32 portID )
	{
		// Only one output
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderSinNode::GetDeclaration( )
	{
		return "float " + GetQualifiedID( ) + ";";
	}

	//================================================================================================================

	ShaderOneMinusNode::ShaderOneMinusNode( const Enjon::String& id )
		: UnaryFunctionNode( id )
	{ 
	}

	//================================================================================================================

	ShaderOneMinusNode::~ShaderOneMinusNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderOneMinusNode::EvaluateOutputType( u32 portID )
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

	Enjon::String ShaderOneMinusNode::EvaluateToGLSL( )
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

		Enjon::String oneVector = "";

		switch ( EvaluateOutputType( ) )
		{
			case ShaderOutputType::Float:
			{
				oneVector = "1.0";
			} break;
			
			case ShaderOutputType::Vec2:
			{
				oneVector = "vec2( 1.0 )";
			} break;
			
			case ShaderOutputType::Vec3:
			{
				oneVector = "vec3( 1.0 )"; 
			} break;

			case ShaderOutputType::Vec4:
			{
				oneVector = "vec4( 1.0 )"; 
			} break;

			default:
			{ 
			} break;
		}

		// Evaluate final line of code
		finalEvaluation += GetQualifiedID( ) + " = " + oneVector + " - " + a->EvaluateAtPort( a_conn.mOutputPortID ) + ";";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderOneMinusNode::EvaluateAtPort( u32 portID )
	{
		// Only one output, so just get qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderOneMinusNode::GetDeclaration( )
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

	ShaderPowerNode::ShaderPowerNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{
		mOutputType = ShaderOutputType::Float;
		mPrimitiveType = ShaderPrimitiveType::Float;
	}

	//================================================================================================================

	ShaderPowerNode::~ShaderPowerNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderPowerNode::EvaluateOutputType( u32 portID )
	{ 
		// Returns float
		return mOutputType;
	}

	//================================================================================================================

	Enjon::String ShaderPowerNode::EvaluateToGLSL( )
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
		finalEvaluation += GetQualifiedID( ) + " = pow(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ", " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderPowerNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderPowerNode::GetDeclaration( )
	{
		// Always returns float
		return "float " + mID + ";";
	}


	//================================================================================================================

	ShaderLengthNode::ShaderLengthNode( const Enjon::String& id )
		: UnaryFunctionNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Float;
		mOutputType = ShaderOutputType::Float;
	}

	//================================================================================================================

	ShaderLengthNode::~ShaderLengthNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderLengthNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Float; 
	}

	//================================================================================================================

	Enjon::String ShaderLengthNode::EvaluateToGLSL( )
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
		finalEvaluation += GetQualifiedID( ) + " = length(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderLengthNode::EvaluateAtPort( u32 portID )
	{
		// Only one output, so just get qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderLengthNode::GetDeclaration( )
	{
		return "float " + mID + ";";
	}

	//================================================================================================================
}
