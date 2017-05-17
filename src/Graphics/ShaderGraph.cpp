// @file ShaderGraph.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/ShaderGraph.h"

namespace Enjon
{
	ShaderGraph::ShaderGraph( ) 
	{
		// Add main node by default
		AddNode( &mMainNode );
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

		// Formatting
		std::cout << "\n";

		// Defines on start
		for ( auto& n : mDefinesOnStart )
		{
			Enjon::String def = n->GetDefinition( );
			std::cout << def << "\n";
		}
	} 

	//===============================================================================================
			
	void ShaderGraph::RegisterVariable( const Enjon::String& var )
	{
		mRegisteredVariables.insert( var );
	} 

	//===============================================================================================
			
	void ShaderGraph::UnregisterVariable( const Enjon::String& var )
	{
		mRegisteredVariables.erase( var );
	}

	//===============================================================================================
			
	void ShaderGraph::RegisterRequiredDefinitions( ShaderGraphNode* node )
	{
		auto query = mDefinesOnStart.find( node );
		if ( query == mDefinesOnStart.end( ) )
		{
			mDefinesOnStart.insert( node );
		}
	}

	//===============================================================================================

	void ShaderGraph::RecurseThroughChildrenAndBuildVariables( ShaderGraphNode* node )
	{
		// Make sure node is valid
		if ( node == nullptr )
		{
			return;
		}

		// Recurse through children
		for ( auto& n : *node->GetInputs( ) )
		{
			RecurseThroughChildrenAndBuildVariables( n );
		}

		// Register variable name if not already used 
		if ( !VariableExists( node->GetID( ) ) )
		{ 
			RegisterVariable( node->GetID( ) );
			std::cout << node->GetDeclaration( ) << "\n"; 

			// If is of appropriate type, then push into defines on main begin
			if ( node->GetPrimitiveType( ) == ShaderPrimitiveType::Texture2D )
			{
				RegisterRequiredDefinitions( node );
			}
		} 
	} 
}














