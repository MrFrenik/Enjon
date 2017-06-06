// @file ShaderGraph.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/ShaderGraph.h"
#include "Utils/FileUtils.h"

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

	Enjon::Result ShaderGraph::Compile( )
	{
		// Shader graph header info
		OutputShaderGraphHeaderInfo( );

		// Vertex Shader
		OutputVertexHeader( );

		// Vertex main
		BeginVertexMain( );
		{
			VertexMain( );
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

			// Default values for outputs on start
			mShaderCodeOutput += "// Default Values\n\n";
			mShaderCodeOutput += "AlbedoOut = vec4( 1.0, 1.0, 1.0, 1.0 );\n";
			mShaderCodeOutput += "NormalsOut = vec4( fs_in.tbn[2], 1.0 );\n";
			mShaderCodeOutput += "EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );\n";
			mShaderCodeOutput += "float metallic = 0.0;\n";
			mShaderCodeOutput += "float roughness = 1.0;\n";
			mShaderCodeOutput += "float ao = 0.0;\n";
			mShaderCodeOutput += "MatPropsOut = vec4( metallic, roughness, ao, 1.0 );\n";
			mShaderCodeOutput += "PositionOut = vec4(fs_in.fragPos, 1.0 );\n\n";

			// Formatting
			mShaderCodeOutput += "\n";

			// Albedo
			mShaderCodeOutput += mMainNode.Evaluate( );
		} 
		EndFragmentMain( );

		// Return success
		return Enjon::Result::SUCCESS;
	}

	//===============================================================================================
		
	Enjon::String ShaderGraph::Parse( const ShaderType type )
	{
		switch ( type )
		{
			case ShaderType::Vertex:
			{
				// Need to grab all lines from VERTEX_BEGIN to VERTEX_END
				return Utils::ParseFromTo( "// VERTEX_SHADER_BEGIN\n", "// VERTEX_SHADER_END\n", mShaderCodeOutput );
			} break;

			case ShaderType::Fragment:
			{
				// Need to grab all lines from VERTEX_BEGIN to VERTEX_END 
				return Utils::ParseFromTo( "// FRAGMENT_SHADER_BEGIN\n", "// FRAGMENT_SHADER_END\n", mShaderCodeOutput );
			} break;

			default: 
			{
				// Shouldn't get here!
				assert( false ); 
				return "";
			} break; 
		}
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

		mShaderCodeOutput += "#version 330 core\n\n";
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
		mShaderCodeOutput += "\tvec3 camViewDir;\n";
		mShaderCodeOutput += "} fs_in;\n\n"; 

		// Fragment shader uniforms
		mShaderCodeOutput += "uniform float uWorldTime = 1.0;\n"; 
		mShaderCodeOutput += "uniform vec3 uCameraWorldPosition = vec3( 0.0, 0.0, 0.0 );\n"; 
	}

	//===============================================================================================

	void ShaderGraph::OutputVertexHeader( )
	{
		mShaderCodeOutput += "// VERTEX_SHADER_BEGIN\n\n";

		mShaderCodeOutput += "#version 330 core\n\n";
		mShaderCodeOutput += "layout (location = 0) in vec3 vertexPosition;\n";
		mShaderCodeOutput += "layout (location = 1) in vec3 vertexNormal;\n";
		mShaderCodeOutput += "layout (location = 2) in vec3 vertexTangent;\n";
		mShaderCodeOutput += "layout (location = 3) in vec2 vertexUV;\n\n";

		mShaderCodeOutput += "out VS_OUT\n";
		mShaderCodeOutput += "{\n";
		mShaderCodeOutput += "\tvec3 fragPos;\n";
		mShaderCodeOutput += "\tvec2 texCoords;\n";
		mShaderCodeOutput += "\tmat3 tbn;\n";
		mShaderCodeOutput += "\tvec3 camViewDir;\n"; 
		mShaderCodeOutput += "} vs_out;\n\n"; 

		mShaderCodeOutput += "uniform mat4 uCamera = mat4(1.0);\n";
		mShaderCodeOutput += "uniform vec3 uCameraWorldPosition = vec3( 0, 0, 0 );\n\n"; 
		mShaderCodeOutput += "uniform vec3 uCameraForwardDirection = vec3( 0, 0, 0 );\n\n"; 
		mShaderCodeOutput += "uniform mat4 uModel = mat4(1.0);\n\n"; 
	}

	//===============================================================================================

	void ShaderGraph::VertexMain( )
	{
		mShaderCodeOutput += "mat4 MVP = uCamera * uModel;\n\n";

		mShaderCodeOutput += "vec3 pos = vec3(uModel * vec4(vertexPosition, 1.0));\n\n"; 

		mShaderCodeOutput += "gl_Position = uCamera * uModel * vec4(vertexPosition, 1.0);\n\n";

		mShaderCodeOutput += "vec3 N = normalize((uModel * vec4(vertexNormal, 0.0)).xyz);\n";
		mShaderCodeOutput += "vec3 T = normalize((uModel * vec4(vertexTangent, 0.0)).xyz);\n\n";
		mShaderCodeOutput += "T = normalize(T - dot(T, N) * N);\n\n";
		//mShaderCodeOutput += "vec3 B = normalize((uModel * vec4((cross(vertexNormal, vertexTangent.xyz) * 1.0), 0.0)).xyz);\n";
		mShaderCodeOutput += "vec3 B = cross(N, T);\n";
		mShaderCodeOutput += "mat3 TBN = mat3(T, B, N);\n\n";

		mShaderCodeOutput += "vec3 viewDir = normalize(uCameraWorldPosition - pos) ;\n\n";

		mShaderCodeOutput += "vs_out.fragPos = vec3(uModel * vec4(vertexPosition, 1.0));\n";
		mShaderCodeOutput += "vs_out.texCoords = vec2(vertexUV.x, -vertexUV.y);\n";
		mShaderCodeOutput += "vs_out.tbn = TBN;\n";
		mShaderCodeOutput += "vs_out.camViewDir = viewDir;\n"; 
	}
	
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
		
	const std::vector< UniformReference >& ShaderGraph::GetUniforms( ) const
	{
		return mUniforms;
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

			// Store uniforms and texture samples to send to shader
			if ( node->GetVariableType( ) == ShaderGraphNodeVariableType::UniformVariable || node->GetPrimitiveType() == ShaderPrimitiveType::Texture2D )
			{
				// Store location
				u32 location = 0;
				if ( node->GetPrimitiveType( ) == ShaderPrimitiveType::Texture2D )
				{
					location = mLastTextureLocation++;
				}

				UniformReference ref = { const_cast< ShaderGraphNode* >( node )->GetID( ), node->GetPrimitiveType( ), location };
				mUniforms.push_back( ref );
			}
		}
	}

	//=================================================================

	ShaderPannerNode::ShaderPannerNode( const Enjon::String& id, const Enjon::Vec2& speed )
		: ShaderGraphNode( id )
	{ 
		mPrimitiveType = ShaderPrimitiveType::Vec2;
		mOutputType = ShaderOutputType::Vec2; 
		mSpeed = speed;
	}

	//=================================================================

	ShaderPannerNode::~ShaderPannerNode( )
	{ 
	}

	//=================================================================

	ShaderOutputType ShaderPannerNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec2;
	}

	//=================================================================

	Enjon::String ShaderPannerNode::EvaluateToGLSL( )
	{ 
		Enjon::String finalEvaluation = "";

		// Evaluate inputs
		for ( auto& c : mInputs )
		{
			ShaderGraphNode* owner = const_cast<ShaderGraphNode*>( c.mOwner );
			finalEvaluation += owner->Evaluate( ) + "\n"; 
		} 

		if ( !mInputs.empty( ) )
		{
			// Get connections
			Connection a_conn = mInputs.at( 0 );

			// Get shader graph nodes
			ShaderGraphNode* a = const_cast<ShaderGraphNode*>( a_conn.mOwner );

			Enjon::String time;

			// Has time input
			if ( mInputs.size( ) > 1 )
			{
				// Get connections
				Connection b_conn = mInputs.at( 1 );

				// Get shader graph nodes
				ShaderGraphNode* b = const_cast< ShaderGraphNode* >( b_conn.mOwner );

				// Evaluate final line of code
				time = b->EvaluateAtPort( b_conn.mOutputPortID );
			}
			else
			{
				time = "uWorldTime";
			}

			// Evaluate final line of code
			Enjon::String aEval = a->EvaluateAtPort( a_conn.mOutputPortID );
			finalEvaluation += GetQualifiedID( ) + " = vec2( fs_in.texCoords.x + " + aEval + ".x * " + time + ", " + " fs_in.texCoords.y + " + aEval + ".y * " + time + ");"; 
		}

		else
		{
			Enjon::String time = "uWorldTime";

			// Evaluate final line of code
			Enjon::String xSpeed = std::to_string( mSpeed.x );
			Enjon::String ySpeed = std::to_string( mSpeed.y );
			finalEvaluation += GetQualifiedID( ) + " = vec2( fs_in.texCoords.x + " + xSpeed + " * " + time + ", " + " fs_in.texCoords.y + " + ySpeed + " * " + time + ");"; 
		}
		
		// Return
		return finalEvaluation; 
	}

	//=================================================================

	Enjon::String ShaderPannerNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderPannerNode::GetDeclaration( )
	{
		return "vec2 " + GetQualifiedID( ) + ";";
	}
 
	//=================================================================

	//=================================================================

	ShaderTextureCoordinatesNode::ShaderTextureCoordinatesNode( const Enjon::String& id, const Enjon::Vec2& tiling )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec2;
		mOutputType = ShaderOutputType::Vec2;
		mTiling = tiling;
	}

	//=================================================================

	ShaderTextureCoordinatesNode::~ShaderTextureCoordinatesNode( )
	{
	}

	//=================================================================

	ShaderOutputType ShaderTextureCoordinatesNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec2;
	}

	//=================================================================

	Enjon::String ShaderTextureCoordinatesNode::EvaluateToGLSL( )
	{
		Enjon::String finalEvaluation = "";

		// Evaluate inputs
		for ( auto& c : mInputs )
		{
			ShaderGraphNode* owner = const_cast< ShaderGraphNode* >( c.mOwner );
			finalEvaluation += owner->Evaluate( ) + "\n";
		}

		if ( !mInputs.empty( ) )
		{
			// Get connections
			Connection a_conn = mInputs.at( 0 );

			// Get shader graph nodes
			ShaderGraphNode* a = const_cast< ShaderGraphNode* >( a_conn.mOwner );

			// Evaluate final line of code
			Enjon::String aEval = a->EvaluateAtPort( a_conn.mOutputPortID );
			finalEvaluation += GetQualifiedID( ) + " = vec2( fs_in.texCoords.x * " + aEval + ".x, " + " fs_in.texCoords.y * " + aEval + ".y);";
		}

		else
		{
			// Evaluate final line of code
			Enjon::String xTiling = std::to_string( mTiling.x );
			Enjon::String yTiling = std::to_string( mTiling.y );
			finalEvaluation += GetQualifiedID( ) + " = vec2( fs_in.texCoords.x * " + xTiling + ", " + " fs_in.texCoords.y * " + yTiling + ");";
		}

		// Return
		return finalEvaluation;
	}

	//=================================================================

	Enjon::String ShaderTextureCoordinatesNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderTextureCoordinatesNode::GetDeclaration( )
	{
		return "vec2 " + GetQualifiedID( ) + ";";
	}

	//=================================================================

	ShaderVertexNormalWSNode::ShaderVertexNormalWSNode( const Enjon::String& id )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
	}

	//=================================================================

	ShaderVertexNormalWSNode::~ShaderVertexNormalWSNode( )
	{
	}

	//=================================================================

	ShaderOutputType ShaderVertexNormalWSNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec3;
	}

	//=================================================================

	Enjon::String ShaderVertexNormalWSNode::EvaluateToGLSL( )
	{ 
		return GetQualifiedID( ) + " = " + VERTEX_NORMAL_WORLD_SPACE_FRAG + ";"; 
	}

	//=================================================================

	Enjon::String ShaderVertexNormalWSNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderVertexNormalWSNode::GetDeclaration( )
	{
		return "vec3 " + GetQualifiedID( ) + ";";
	} 

	//=================================================================

	ShaderPixelNormalWSNode::ShaderPixelNormalWSNode( const Enjon::String& id )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
	}

	//=================================================================

	ShaderPixelNormalWSNode::~ShaderPixelNormalWSNode( )
	{
	}

	//=================================================================

	ShaderOutputType ShaderPixelNormalWSNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec3;
	}

	//=================================================================

	Enjon::String ShaderPixelNormalWSNode::EvaluateToGLSL( )
	{
		return GetQualifiedID( ) + " = " + FRAGMENT_NORMAL_WORLD_SPACE_FRAG + ";";
	}

	//=================================================================

	Enjon::String ShaderPixelNormalWSNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderPixelNormalWSNode::GetDeclaration( )
	{
		return "vec3 " + GetQualifiedID( ) + ";";
	}

	//=================================================================

	ShaderCameraWorldPositionNode::ShaderCameraWorldPositionNode( const Enjon::String& id )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
	}

	//=================================================================

	ShaderCameraWorldPositionNode::~ShaderCameraWorldPositionNode( )
	{
	}

	//=================================================================

	ShaderOutputType ShaderCameraWorldPositionNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec3;
	}

	//=================================================================

	Enjon::String ShaderCameraWorldPositionNode::EvaluateToGLSL( )
	{
		return GetQualifiedID( ) + " = " + CAMERA_WORLD_POSITION + ";";
	}

	//=================================================================

	Enjon::String ShaderCameraWorldPositionNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderCameraWorldPositionNode::GetDeclaration( )
	{
		return "vec3 " + GetQualifiedID( ) + ";";
	}

	//=================================================================

	//=================================================================

	ShaderCameraViewDirectionNode::ShaderCameraViewDirectionNode( const Enjon::String& id )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
	}

	//=================================================================

	ShaderCameraViewDirectionNode::~ShaderCameraViewDirectionNode( )
	{
	}

	//=================================================================

	ShaderOutputType ShaderCameraViewDirectionNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec3;
	}

	//=================================================================

	Enjon::String ShaderCameraViewDirectionNode::EvaluateToGLSL( )
	{
		return GetQualifiedID( ) + " = normalize( uCameraWorldPosition - fs_in.fragPos );";
	}

	//=================================================================

	Enjon::String ShaderCameraViewDirectionNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderCameraViewDirectionNode::GetDeclaration( )
	{
		return "vec3 " + GetQualifiedID( ) + ";";
	}

	//=================================================================

	ShaderVertexNormalDirectionNode::ShaderVertexNormalDirectionNode( const Enjon::String& id )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
	}

	//=================================================================

	ShaderVertexNormalDirectionNode::~ShaderVertexNormalDirectionNode( )
	{
	}

	//=================================================================

	ShaderOutputType ShaderVertexNormalDirectionNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec3;
	}

	//=================================================================

	Enjon::String ShaderVertexNormalDirectionNode::EvaluateToGLSL( )
	{
		return GetQualifiedID( ) + " = " + VERTEX_NORMAL_DIR_UNPERTURBED_FRAG + ";";
	}

	//=================================================================

	Enjon::String ShaderVertexNormalDirectionNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderVertexNormalDirectionNode::GetDeclaration( )
	{
		return "vec3 " + GetQualifiedID( ) + ";";
	} 
	
	//=================================================================

	ShaderVertexWorldPositionNode::ShaderVertexWorldPositionNode( const Enjon::String& id )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Vec3;
		mOutputType = ShaderOutputType::Vec3;
	}

	//=================================================================

	ShaderVertexWorldPositionNode::~ShaderVertexWorldPositionNode( )
	{
	}

	//=================================================================

	ShaderOutputType ShaderVertexWorldPositionNode::EvaluateOutputType( u32 portID )
	{
		return ShaderOutputType::Vec3;
	}

	//=================================================================

	Enjon::String ShaderVertexWorldPositionNode::EvaluateToGLSL( )
	{
		return GetQualifiedID( ) + " = " + VERTEX_WORLD_POSITION_FRAG + ";";
	}

	//=================================================================

	Enjon::String ShaderVertexWorldPositionNode::EvaluateAtPort( u32 portID )
	{
		return GetQualifiedID( );
	}

	//=================================================================

	Enjon::String ShaderVertexWorldPositionNode::GetDeclaration( )
	{
		return "vec3 " + GetQualifiedID( ) + ";";
	}

	//================================================================================================================

	ShaderBranchIfElseNode::ShaderBranchIfElseNode( const Enjon::String& id )
		: ShaderGraphNode( id )
	{
		mPrimitiveType = ShaderPrimitiveType::Float;
		mOutputType = ShaderOutputType::Float;
	}

	//================================================================================================================

	ShaderBranchIfElseNode::~ShaderBranchIfElseNode( )
	{
	}

	//================================================================================================================

	ShaderOutputType ShaderBranchIfElseNode::EvaluateOutputType( u32 portID )
	{
		// For now, return float 
		return ShaderOutputType::Float; 
	}

	//================================================================================================================

	Enjon::String ShaderBranchIfElseNode::EvaluateToGLSL( )
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
		Connection agteb_conn;
		Connection aeb_conn;
		Connection altb_conn;

		// Get shader graph nodes
		ShaderGraphNode* a = const_cast<ShaderGraphNode*>( a_conn.mOwner );
		ShaderGraphNode* b = const_cast<ShaderGraphNode*>( b_conn.mOwner );
		ShaderGraphNode* aGTEb = nullptr;
		ShaderGraphNode* aEb = nullptr;
		ShaderGraphNode* aLTb = nullptr;

		// aGTEb
		if ( mInputs.size( ) >= 2 )
		{
			agteb_conn = mInputs.at( 2 );
			aGTEb = const_cast< ShaderGraphNode* >( agteb_conn.mOwner );
		}
		
		// aLTb
		if ( mInputs.size( ) >= 3 )
		{
			altb_conn = mInputs.at( 3 );
			aLTb = const_cast< ShaderGraphNode* >( altb_conn.mOwner );
		}
		
		// aEb
		//if ( mInputs.size( ) >= 4 )
		//{
		//	aeb_conn = mInputs.at( 4 );
		//	aEb = const_cast< ShaderGraphNode* >( aeb_conn.mOwner );
		//} 
		
		Enjon::String aEval = a->EvaluateAtPort( a_conn.mOutputPortID );
		Enjon::String bEval = b->EvaluateAtPort( b_conn.mOutputPortID ); 

		//if ( aEb )
		//{
		//	Enjon::String aebeval = aEb->EvaluateAtPort( aeb_conn.mOutputPortID ); 
		//	finalEvaluation += "if ( " + aEval + " == " + bEval + " )\n";
		//	finalEvaluation += "{\n";
		//	finalEvaluation += "\t" + GetQualifiedID( ) + " = " + aebeval + ";\n";
		//	finalEvaluation += "}\n";
		//}

		if ( aGTEb )
		{ 
			Enjon::String agtebeval = aGTEb->EvaluateAtPort( agteb_conn.mOutputPortID ); 
			finalEvaluation += "if ( " + aEval + " >= " + bEval + " )\n";
			finalEvaluation += "{\n";
			finalEvaluation += "\t" + GetQualifiedID( ) + " = " + agtebeval + ";\n";
			finalEvaluation += "}\n"; 
		}

		if ( aLTb )
		{
			Enjon::String altbeval = aLTb->EvaluateAtPort( altb_conn.mOutputPortID ); 
			finalEvaluation += "if ( " + aEval + " < " + bEval + " )\n";
			finalEvaluation += "{\n";
			finalEvaluation += "\t" + GetQualifiedID( ) + " = " + altbeval + ";\n";
			finalEvaluation += "}\n"; 
		} 

		// Return
		return finalEvaluation;
	}

	//================================================================================================================

	Enjon::String ShaderBranchIfElseNode::EvaluateAtPort( u32 portID )
	{
		// Multiply node only has one output, so just return its qualified id
		return GetQualifiedID( );
	}

	//================================================================================================================

	Enjon::String ShaderBranchIfElseNode::GetDeclaration( )
	{
		return "float " + mID + ";"; 
	} 
	
	//================================================================================================================
}













