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
}













