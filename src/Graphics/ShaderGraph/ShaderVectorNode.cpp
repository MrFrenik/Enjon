// @file ShaderVectorNode.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/ShaderGraph/ShaderVectorNode.h"

namespace Enjon
{ 
	//=============================================================================================================

	ShaderVec2Node::ShaderVec2Node( const Enjon::String& id )
		: ShaderPrimitiveNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec2;
		mOutputType = ShaderOutputType::Vec2;
		mMaxNumberInputs = 2;
	}

	//=============================================================================================================

	ShaderVec2Node::ShaderVec2Node( const Enjon::String& id, const Vec2& vec )
		: ShaderPrimitiveNode( id )
	{
		mData = vec;
		mPrimitiveType = ShaderPrimitiveType::Vec2;
		mOutputType = ShaderOutputType::Vec2;
		mMaxNumberInputs = 2;
	}

	//=============================================================================================================

	ShaderVec2Node::~ShaderVec2Node( ) { }

	//=============================================================================================================

	Enjon::String ShaderVec2Node::EvaluateToGLSL( )
	{
		Enjon::String x = std::to_string( mData.x );
		Enjon::String y = std::to_string( mData.y );

		Enjon::String def = "vec2 " + GetQualifiedID( ) + " = vec2(" + x + ", " + y + ");";

		switch ( mVariableType )
		{
		case ShaderGraphNodeVariableType::LocalVariable:
		{
			return def;
		}
		break;

		case ShaderGraphNodeVariableType::UniformVariable:
		{
			return "";
		}
		break;

		default:
		{
			return "";
		}
		break;
		}
	}

	//=============================================================================================================

	Enjon::String ShaderVec2Node::GetDeclaration( )
	{
		switch ( mVariableType )
		{ 
			case ShaderGraphNodeVariableType::UniformVariable:
			{
				return ( "uniform vec2 " + mID + ";" ); 
			} break;
			
			default:
			case ShaderGraphNodeVariableType::LocalVariable:
			{
				return ( "vec2 " + mID + ";" ); 
			} break;
		}
	} 

	//=============================================================================================================
	
	ShaderVec3Node::ShaderVec3Node( const Enjon::String& id )
		: ShaderPrimitiveNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
		mMaxNumberInputs = 3;
	}

	//=============================================================================================================

	ShaderVec3Node::ShaderVec3Node( const Enjon::String& id, const Vec3& vec )
		: ShaderPrimitiveNode( id )
	{
		mData = vec;
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
		mMaxNumberInputs = 3;
	}

	//=============================================================================================================

	ShaderVec3Node::~ShaderVec3Node( ) 
	{ 
	}

	//=============================================================================================================

	Enjon::String ShaderVec3Node::EvaluateToGLSL( )
	{
		Enjon::String x = std::to_string( mData.x );
		Enjon::String y = std::to_string( mData.y );
		Enjon::String z = std::to_string( mData.z );

		Enjon::String def = "vec3 " + GetQualifiedID( ) + " = vec3(" + x + ", " + y + ", " + z + ");";

		switch ( mVariableType )
		{
		case ShaderGraphNodeVariableType::LocalVariable:
		{
			return def;
		}
		break;

		case ShaderGraphNodeVariableType::UniformVariable:
		{
			return "";
		}
		break;

		default:
		{
			return "";
		}
		break;
		}
	} 

	//=============================================================================================================

	Enjon::String ShaderVec3Node::GetDeclaration( )
	{
		switch ( mVariableType )
		{
			case ShaderGraphNodeVariableType::UniformVariable:
			{
				return ( "uniform vec3 " + mID + ";" ); 
			} break;
			
			default:
			case ShaderGraphNodeVariableType::LocalVariable:
			{
				return ( "vec3 " + mID + ";" ); 
			} break; 
		}
	}
 
	//=============================================================================================================
	
	ShaderVec4Node::ShaderVec4Node( const Enjon::String& id )
		: ShaderPrimitiveNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec4;
		mOutputType = ShaderOutputType::Vec4;
		mMaxNumberInputs = 4;
	}

	//=============================================================================================================

	ShaderVec4Node::ShaderVec4Node( const Enjon::String& id, const Vec4& vec )
		: ShaderPrimitiveNode( id )
	{
		mData = vec;
		mPrimitiveType = ShaderPrimitiveType::Vec4;
		mOutputType = ShaderOutputType::Vec4;
		mMaxNumberInputs = 4;
	}

	//=============================================================================================================

	ShaderVec4Node::~ShaderVec4Node( ) 
	{ 
	}

	//=============================================================================================================

	Enjon::String ShaderVec4Node::EvaluateToGLSL( )
	{
		Enjon::String x = std::to_string( mData.x );
		Enjon::String y = std::to_string( mData.y );
		Enjon::String z = std::to_string( mData.z );
		Enjon::String w = std::to_string( mData.w );

		Enjon::String def = "vec4 " + GetQualifiedID( ) + " = vec4(" + x + ", " + y + ", " + z + ", " + w + ");";

		switch ( mVariableType )
		{
		case ShaderGraphNodeVariableType::LocalVariable:
		{
			return def;
		}
		break;

		case ShaderGraphNodeVariableType::UniformVariable:
		{
			return "";
		}
		break;

		default:
		{
			return "";
		}
		break;
		}
	} 

	//=============================================================================================================

	Enjon::String ShaderVec4Node::GetDeclaration( )
	{
		switch ( mVariableType )
		{
			case ShaderGraphNodeVariableType::UniformVariable:
			{
				return ( "uniform vec4 " + mID + ";" ); 
			} break;
			
			default:
			case ShaderGraphNodeVariableType::LocalVariable:
			{
				return ( "vec4 " + mID + ";" ); 
			} break; 
		}
	}

	//=============================================================================================================

	ShaderClampNode::ShaderClampNode( const Enjon::String& id, f32 min, f32 max )
		: UnaryFunctionNode( id ), mMin( min ), mMax( max )
	{
	}

	//================================================================================================================

	ShaderClampNode::~ShaderClampNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderClampNode::EvaluateOutputType( u32 portID )
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

	Enjon::String ShaderClampNode::EvaluateToGLSL( )
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

		// Need to make these inputs eventually
		Enjon::String min = std::to_string( mMin );
		Enjon::String max = std::to_string( mMax );

		// Evaluate final line of code
		finalEvaluation += GetQualifiedID( ) + " = clamp(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ", " + min + ", " + max +  ");";

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderClampNode::EvaluateAtPort( u32 portID )
	{
		// Only one output, so just get qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderClampNode::GetDeclaration( )
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

	ShaderMaskNode::ShaderMaskNode( const Enjon::String& id, MaskFlags flags )
		: UnaryFunctionNode( id ), mFlags( flags )
	{
	}

	//================================================================================================================

	ShaderMaskNode::~ShaderMaskNode( )
	{
	}

	//================================================================================================================

	bool ShaderMaskNode::HasMask( MaskFlags flags )
	{
		return flags == mFlags;
	}

	//================================================================================================================

	ShaderOutputType ShaderMaskNode::EvaluateOutputType( u32 portID )
	{ 
		Connection a_conn = mInputs.at( 0 );

		// Look at inputs to determine output type of this node
		ShaderGraphNode* anode = const_cast< ShaderGraphNode* >( mInputs.at( 0 ).mOwner );

		// Evaluate output type
		ShaderOutputType aType = anode->EvaluateOutputType( a_conn.mOutputPortID );

		bool r = HasMask( MaskFlags::R );
		bool g = HasMask( MaskFlags::G );
		bool b = HasMask( MaskFlags::B );
		bool a = HasMask( MaskFlags::A );
		bool rg = HasMask( MaskFlags::RG );
		bool rb = HasMask( MaskFlags::RB );
		bool ra = HasMask( MaskFlags::RA );
		bool rgba = HasMask( MaskFlags::RGBA );
		bool rgb = HasMask( MaskFlags::RGB );
		bool gba = HasMask( MaskFlags::GBA );

		if ( rgba )
		{
			mOutputType = ShaderOutputType::Vec4;
		} 
		
		else if ( rgb || gba )
		{
			mOutputType = ShaderOutputType::Vec3;
		} 

		else if ( rg || rb || ra )
		{
			mOutputType = ShaderOutputType::Vec2;
		} 

		else
		{
			mOutputType = ShaderOutputType::Float;
		}

		return mOutputType;
	}

	//================================================================================================================

	Enjon::String ShaderMaskNode::EvaluateToGLSL( )
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
		ShaderGraphNode* anode = const_cast< ShaderGraphNode* >( a_conn.mOwner );

		bool r = HasMask( MaskFlags::R );
		bool g = HasMask( MaskFlags::G );
		bool b = HasMask( MaskFlags::B );
		bool a = HasMask( MaskFlags::A );
		bool rg = HasMask( MaskFlags::RG );
		bool rb = HasMask( MaskFlags::RB );
		bool ra = HasMask( MaskFlags::RA );
		bool rgba = HasMask( MaskFlags::RGBA );
		bool rgb = HasMask( MaskFlags::RGB );
		bool gba = HasMask( MaskFlags::GBA );

		switch ( EvaluateOutputType( ) )
		{
			default:
			case ShaderOutputType::Float:
			{
				ShaderOutputType aType = anode->EvaluateOutputType( );
				if ( aType != ShaderOutputType::Float )
				{
					if ( r )		finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".r;"; 
					else if ( g )	finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".g;"; 
					else if ( b )	finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".b;"; 
					else			finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".a;"; 
				}
			} break;
			
			case ShaderOutputType::Vec2:
			{ 
				if ( rg )
				{
					finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".rg;"; 
				}
				else if ( rb )
				{ 
					finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".rb;"; 
				}
				else
				{
					finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".ra;"; 
				}

			} break;
			
			case ShaderOutputType::Vec3:
			{
				if ( rgb )
				{
					finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".rgb;"; 
				}
				else
				{
					finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ".gba;"; 
				}
				
			} break;
			
			case ShaderOutputType::Vec4:
			{
					finalEvaluation += GetQualifiedID( ) + " = " + anode->EvaluateAtPort( a_conn.mOutputPortID ) + ";"; 
			} break;
		}

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderMaskNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderMaskNode::GetDeclaration( )
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

	ShaderAppendNode::ShaderAppendNode( const Enjon::String& id )
		: BinaryFunctionNode( id )
	{
	}

	//================================================================================================================

	ShaderAppendNode::~ShaderAppendNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderAppendNode::EvaluateOutputType( u32 portID )
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
			default:
			case ShaderOutputType::Float:
			{
				if ( bType == ShaderOutputType::Float )
				{
					mOutputType = ShaderOutputType::Vec2;
				} 
			} break;

			case ShaderOutputType::Vec2:
			{
				if ( bType == ShaderOutputType::Float )
				{
					mOutputType = ShaderOutputType::Vec3;
				}
				else
				{
					// Throw error here!
				}

			} break;

			case ShaderOutputType::Vec3:
			{
				if ( bType == ShaderOutputType::Float )
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

	Enjon::String ShaderAppendNode::EvaluateToGLSL( )
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
		switch ( EvaluateOutputType( ) )
		{
			default:
			case ShaderOutputType::Vec2:
			{
				finalEvaluation += GetQualifiedID( ) + " = vec2(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ", " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ");"; 
			} break;
			
			case ShaderOutputType::Vec3:
			{
				finalEvaluation += GetQualifiedID( ) + " = vec3(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ", " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ");"; 
			} break;
			
			case ShaderOutputType::Vec4:
			{
				finalEvaluation += GetQualifiedID( ) + " = vec4(" + a->EvaluateAtPort( a_conn.mOutputPortID ) + ", " + b->EvaluateAtPort( b_conn.mOutputPortID ) + ");"; 
			} break;
		} 

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderAppendNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderAppendNode::GetDeclaration( )
	{
		switch ( EvaluateOutputType( ) )
		{
			case ShaderOutputType::Vec2: return "vec2 " + mID + ";"; break;
			case ShaderOutputType::Vec3: return "vec3 " + mID + ";"; break;
			case ShaderOutputType::Vec4: return "vec4 " + mID + ";"; break;
			default: return "";
		}
	}

	//================================================================================================================
}













