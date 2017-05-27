// @file ShaderGraph.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/ShaderGraph.h"

namespace Enjon
{
	ShaderGraph::ShaderGraph( )
	{
		// Add main node by default
		AddNode( &mMainNode );

		// Set shader graph output to empty string
		mShaderCodeOutput = "";
	}

	//===============================================================================================

	ShaderGraph::~ShaderGraph( )
	{
	}

	//===============================================================================================

	ShaderGraphNode* ShaderGraph::AddNode( ShaderGraphNode* node )
	{
		auto query = mNodes.find( node );
		if ( query == mNodes.end( ) )
		{
			mNodes.insert( node );
		}

		return node;
	}

	//===============================================================================================

	void ShaderGraph::RemoveNode( ShaderGraphNode* node )
	{
		auto query = mNodes.find( node );
		if ( query != mNodes.end( ) )
		{
			mNodes.erase( node );
			delete node;
			node = nullptr;
		}
	}

	//===============================================================================================

	bool ShaderGraph::VariableExists( const Enjon::String& var )
	{
		auto query = mRegisteredVariables.find( var );
		return ( query != mRegisteredVariables.end( ) );
	}

	//===============================================================================================
		
	void ShaderGraph::OutputShaderGraphHeaderInfo( )
	{
		mShaderCodeOutput += "/*\n";
		mShaderCodeOutput += "\tShaderGraph\n";
		mShaderCodeOutput += " \tThis file has been generated. All modifications will be lost.\n";
		mShaderCodeOutput += "*/\n\n"; 
	}

	//===============================================================================================

	void ShaderGraph::Compile( )
	{
		// Shader graph header info
		OutputShaderGraphHeaderInfo( );

		// Vertex Shader
		OutputVertexHeader( );

		// Vertex main
		BeginVertexMain( );
		{

		}
		EndVertexMain( );

		// Output fragment header code
		OutputFragmentHeader( );

		// Has to iterate through all nodes and build map of leaf nodes
		for ( auto& n : mNodes )
		{
			// Recursively run through children
			RecurseThroughChildrenAndBuildVariables( n );
		}

		// Begin declarations
		mShaderCodeOutput += "// Declarations and Uniforms \n";

		// Output declarations
		for ( auto& var : mDeclarations )
		{
			 // Outputting variables
			mShaderCodeOutput +=  var + "\n";
		} 

		// Output fragment main
		BeginFragmentMain( ); 
		{ 
			// Defines on start
			for ( auto& n : mDefinesOnStart )
			{
				mShaderCodeOutput += const_cast< ShaderGraphNode* >( n )->GetDefinition( ) + "\n";
			} 

			// Albedo
			mShaderCodeOutput += mMainNode.Evaluate( );
		} 
		EndFragmentMain( );
	}

	//===============================================================================================
		
	void ShaderGraph::DeleteGraph( )
	{ 
		std::set< ShaderGraphNode* > nodes = mNodes;
		for ( auto& n : nodes )
		{
			// Skip main node
			if ( n == &mMainNode )
			{
				continue;
			}

			RemoveNode( n );
		} 
	}

	//===============================================================================================
		
	void ShaderGraph::OutputFragmentHeader( )
	{
		mShaderCodeOutput += "// FRAGMENT_SHADER_BEGIN\n\n";

		mShaderCodeOutput += "#version 330 core\n";
		mShaderCodeOutput += "layout (location = 0) out vec4 AlbedoOut;\n";
		mShaderCodeOutput += "layout (location = 1) out vec4 NormalsOut;\n";
		mShaderCodeOutput += "layout (location = 2) out vec4 PositionOut;\n";
		mShaderCodeOutput += "layout (location = 3) out vec4 EmissiveOut;\n";
		mShaderCodeOutput += "layout (location = 4) out vec4 MatPropsOut;\n\n";

		mShaderCodeOutput += "in VS_OUT\n";
		mShaderCodeOutput += "{\n";
		mShaderCodeOutput += "\tvec3 fragPos;\n";
		mShaderCodeOutput += "\tvec2 texCoords;\n";
		mShaderCodeOutput += "\tmat3 tbn;\n";
		mShaderCodeOutput += "} fs_in;\n\n"; 

		// Default values for outputs
		mShaderCodeOutput += "// Default Values\n\n";
		mShaderCodeOutput += "AlbedoOut = vec4( 1.0, 1.0, 1.0, 1.0 );\n";
		mShaderCodeOutput += "NormalsOut = vec4( 0.0, 1.0, 0.0, 1.0 );\n";
		mShaderCodeOutput += "EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );\n";
		mShaderCodeOutput += "float metallic = 0.0;\n";
		mShaderCodeOutput += "float roughness = 1.0;\n";
		mShaderCodeOutput += "float ao = 0.0;\n";
		mShaderCodeOutput += "MatPropsOut = vec4( metallic, roughness, ao, 1.0 );\n\n";
	}

	//===============================================================================================

	void ShaderGraph::OutputVertexHeader( )
	{
		mShaderCodeOutput += "// VERTEX_SHADER_BEGIN\n\n";
	}

	//===============================================================================================
	
	void ShaderGraph::BeginVertexMain( )
	{
		// For now, just output default code 
		mShaderCodeOutput += "\n// Vertex main\n";
		mShaderCodeOutput += "void main()\n";
		mShaderCodeOutput += "{\n";
	}

	//===============================================================================================
	
	void ShaderGraph::EndVertexMain( )
	{ 
		mShaderCodeOutput += "}\n\n";
		mShaderCodeOutput += "// VERTEX_SHADER_END\n\n";
	}

	//===============================================================================================

	void ShaderGraph::BeginFragmentMain( )
	{
		mShaderCodeOutput += "\n// Fragment Main\n";
		mShaderCodeOutput += "void main() \n{\n";
	}

	//===============================================================================================

	void ShaderGraph::EndFragmentMain( )
	{
		mShaderCodeOutput += "\n}\n";
		mShaderCodeOutput += "\n// FRAGMENT_SHADER_END\n";
	}

	//===============================================================================================

	void ShaderGraph::RegisterVariable( const Enjon::String& var )
	{
		mRegisteredVariables.insert( var );
	}

	//===============================================================================================
		
	void ShaderGraph::RegisterDeclaration( const Enjon::String& decl )
	{
		mDeclarations.insert( decl );
	}

	//===============================================================================================

	void ShaderGraph::UnregisterVariable( const Enjon::String& var )
	{
		mRegisteredVariables.erase( var );
	}

	//===============================================================================================

	void ShaderGraph::RegisterRequiredDefinitions( const ShaderGraphNode* node )
	{
		auto query = mDefinesOnStart.find( node );
		if ( query == mDefinesOnStart.end( ) )
		{
			mDefinesOnStart.insert( node );
		}
	}

	//===============================================================================================

	void ShaderGraph::Connect( const ShaderGraphNode::Connection& connection )
	{
		mMainNode.AddInput( connection );
	}

	//===============================================================================================

	void ShaderGraph::RecurseThroughChildrenAndBuildVariables( const ShaderGraphNode* node )
	{
		// Make sure node is valid
		if ( node == nullptr )
		{
			return;
		}

		// Recurse through children
		for ( auto& n : *node->GetInputs( ) )
		{
			RecurseThroughChildrenAndBuildVariables( n.mOwner );
		}

		// Register variable name if not already used 
		if ( !VariableExists( node->GetID( ) ) )
		{ 
			// Register variable with graph
			RegisterVariable( node->GetID( ) ); 

			// Register variable declaration
			RegisterDeclaration( const_cast< ShaderGraphNode* >( node )->GetDeclaration( ) ); 

			// If is of appropriate type, then push into defines on main begin
			if ( node->GetPrimitiveType( ) == ShaderPrimitiveType::Texture2D )
			{
				RegisterRequiredDefinitions( node );
			}
		}
	}
}













