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

	void ShaderGraph::Compile( )
	{
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

	void ShaderGraph::BeginFragmentMain( )
	{
		mShaderCodeOutput += "\n// Fragment Main\n";
		mShaderCodeOutput += "void main() \n{\n";
	}

	//===============================================================================================

	void ShaderGraph::EndFragmentMain( )
	{
		mShaderCodeOutput += "\n}\n";
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













