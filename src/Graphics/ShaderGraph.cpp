#include "Graphics/ShaderGraph.h"
#include "Graphics/Shader.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <fstream>
#include <sstream> 
#include <algorithm>
#include <iostream>

#define STRSIZE( string ) static_cast< SizeType >( string.length() ) 

using namespace rapidjson;

namespace Enjon
{ 
	std::unordered_map< std::string, Enjon::ShaderGraphNodeTemplate > Enjon::ShaderGraph::mTemplates;

	std::string ParseFromTo( const std::string& begin, const std::string& end, const std::string& src )
	{
		std::string returnStr = "";

		// Search for line to match
		std::size_t foundBegin = src.find( begin );
		std::size_t foundEnd = src.find( end );

		// Found
		if ( foundBegin != std::string::npos && foundEnd != std::string::npos )
		{
			// Substring length
			std::size_t length = foundEnd - foundBegin;

			// Return that substring
			return src.substr( foundBegin, length );
		}

		// Didn't find anything, so return empty string
		return returnStr;
	}

	//=========================================================================================================================

	std::string OutputLine( const std::string& line )
	{
		return line + "\n";
	}

	//=========================================================================================================================

	std::string OutputTabbedLine( const std::string& line )
	{
		return "\t" + line + "\n";
	}

	std::string OutputErrorBlock( const std::string& line )
	{
		std::string code = "";
		code += OutputLine( "///////////////////////////////////////////////////////////" );
		code += OutputLine( "// ERROR //////////////////////////////////////////////////" );
		code += OutputLine( line );
		code += OutputLine( "// ERROR //////////////////////////////////////////////////" );
		code += OutputLine( "///////////////////////////////////////////////////////////" );

		return code;
	}

	void ShaderGraphNode::AddOverride( const InputConnection& connection )
	{
		if ( !HasOverride( connection.mName ) )
		{
			mDefaultOverrides[ connection.mName ] = connection;
		}
	}

	//=========================================================================================================================

	bool ShaderGraphNode::HasOverride( const std::string& inputName )
	{
		return ( mDefaultOverrides.find( inputName ) != mDefaultOverrides.end( ) );
	}

	//=========================================================================================================================

	const InputConnection* ShaderGraphNode::GetOverride( const std::string& inputName )
	{
		if ( HasOverride( inputName ) )
		{
			return &mDefaultOverrides[ inputName ];
		}

		return nullptr;
	}

	//=========================================================================================================================

	void ShaderGraphNode::AddLink( const NodeLink& link )
	{
		// Make sure link doesn't exist before adding it
		bool bIsFound = false;
		for ( auto& l : mLinks )
		{
			// If the same input connection is used, then we're already in use for that link
			if ( l.mTo == link.mTo )
			{
				bIsFound = true;
				break;
			}
		}

		// If not in use, push back link
		if ( !bIsFound )
		{
			mLinks.push_back( link );
		}
	}

	//=========================================================================================================================

	void ShaderGraphNodeTemplate::AddParameterLayout( const ParameterLayout& layout )
	{
		// Build key for layout based on inputs
		std::string inputKey = "";
		for ( u32 i = 0; i < layout.mInputParameters.size( ); ++i )
		{
			inputKey += layout.mInputParameters.at( i );
		}

		auto query = mParameterLayouts.find( inputKey );
		if ( query == mParameterLayouts.end( ) )
		{
			mParameterLayouts[ inputKey ] = layout;
		}
	}

	//=========================================================================================================================

	const ParameterLayout* ShaderGraphNodeTemplate::GetLayout( const std::string& inputKey )
	{
		// Find and return parameter layout based on input key
		auto query = mParameterLayouts.find( inputKey );
		if ( query != mParameterLayouts.end( ) )
		{
			return &query->second;
		}

		return nullptr;
	}

	//=========================================================================================================================

	const NodeLink* ShaderGraphNode::GetLink( const std::string inputName )
	{
		// Search for link based on input name
		for ( auto& n : mLinks )
		{
			if ( n.mTo->mName.compare( inputName ) == 0 )
			{
				return &n;
			}
		}

		return nullptr;
	}

	//=========================================================================================================================

	bool ShaderGraphNodeTemplate::IsFunction( ) const
	{
		return ( mType == ShaderGraphNodeType::Function );
	}

	//=========================================================================================================================

	ShaderGraph::ShaderGraph( )
	{
	}

	//=========================================================================================================================

	ShaderGraph::~ShaderGraph( )
	{
	}

	//=========================================================================================================================

	void ShaderGraph::AddNode( const ShaderGraphNode& node )
	{
		auto query = mNodes.find( node.mName );
		if ( query == mNodes.end( ) )
		{
			mNodes[ node.mName ] = node;
		}
	}

	//=========================================================================================================================

	const ShaderGraphNode* ShaderGraph::GetNode( const std::string& nodeName )
	{
		auto query = mNodes.find( nodeName );
		if ( query != mNodes.end( ) )
		{
			return &mNodes[ nodeName ];
		}

		return nullptr;
	}
	const ShaderGraphNodeTemplate* ShaderGraph::GetTemplate( const std::string& name )
	{
		auto query = mTemplates.find( name );
		if ( query != mTemplates.end( ) )
		{
			return &mTemplates[ name ];
		}

		return nullptr;
	}

	//==========================================================================================

	std::string ShaderGraph::ShaderPassToString( ShaderPassType type )
	{
		switch ( type )
		{
		case ShaderPassType::Surface_StaticGeom:
		{
			return "Surface_StaticGeom";
		}; break;
		default:
		{
			return "";
		} break;
		}
	}

	//==========================================================================================

	Document ShaderGraph::GetJSONDocumentFromFilePath( const std::string& filePath, s32* status )
	{
		// Load file
		std::ifstream f;
		f.open( filePath );
		std::stringstream buffer;
		buffer << f.rdbuf( );

		std::string str = buffer.str( );

		Document document;
		if ( document.Parse( str.c_str( ), STRSIZE( str ) ).HasParseError( ) )
		{
			auto parseError = document.Parse( str.c_str( ), STRSIZE( str ) ).GetParseError( );
			std::cout << parseError << "\n";
			*status = -1;
		}

		return document;
	}

	//=========================================================================================================================

	s32 ShaderGraph::DeserializeTemplate( const std::string& filePath )
	{
		s32 status = 1;

		// Get document
		Document document = ShaderGraph::GetJSONDocumentFromFilePath( filePath, &status );

		if ( status != 1 )
		{
			// Log error and return status
			return status;
		}

		// Check to make sure that templates is found
		if ( document.HasMember( "Templates" ) )
		{
			auto templatesItr = document.FindMember( "Templates" );
			if ( templatesItr->value.IsObject( ) )
			{
				// Get templates object
				auto templates = templatesItr->value.GetObject( );
				for ( Value::ConstMemberIterator itr = templates.MemberBegin( ); itr != templates.MemberEnd( ); ++itr )
				{
					// If this happens, we won't add the new template
					bool bErrorOccured = false;

					// Create new template to fill out
					ShaderGraphNodeTemplate newTemplate;

					// Get template name
					std::string templateName = itr->name.GetString( );

					// Set name
					newTemplate.mName = templateName;

					// Get node type
					if ( itr->value.HasMember( "Type" ) )
					{
						std::string templateStr = itr->value.FindMember( "Type" )->value.GetString( );

						// If function
						if ( templateStr.compare( "Function" ) == 0 )
						{
							newTemplate.mType = ShaderGraphNodeType::Function;
						}
						// If variable
						else if ( templateStr.compare( "Variable" ) == 0 )
						{
							newTemplate.mType = ShaderGraphNodeType::Variable;
						}
						// Else is 
						else if ( templateStr.compare( "Component" ) == 0 )
						{
							newTemplate.mType = ShaderGraphNodeType::Component;
						}
						else
						{
							// Error
						}
					}

					// Get uniform type
					if ( itr->value.HasMember( "UniformType" ) )
					{
						std::string uniformStr = itr->value.FindMember( "UniformType" )->value.GetString( );
					std:transform( uniformStr.begin( ), uniformStr.end( ), uniformStr.begin( ), ::tolower );

						// Find uniform type
						if ( uniformStr.compare( "texturesampler2d" ) == 0 )
						{
							newTemplate.mUniformType = UniformType::TextureSampler2D;
						}
						else if ( uniformStr.compare( "float" ) == 0 )
						{
							newTemplate.mUniformType = UniformType::Float;
						}
						else if ( uniformStr.compare( "vec2" ) == 0 )
						{
							newTemplate.mUniformType = UniformType::Vec2;
						}
						else if ( uniformStr.compare( "vec3" ) == 0 )
						{
							newTemplate.mUniformType = UniformType::Vec3;
						}
						else if ( uniformStr.compare( "vec4" ) == 0 )
						{
							newTemplate.mUniformType = UniformType::Vec4;
						}
						else if ( uniformStr.compare( "mat4" ) == 0 )
						{
							newTemplate.mUniformType = UniformType::Mat4;
						}
						else
						{
							// Invalid
							newTemplate.mUniformType = UniformType::Invalid;
						}
					}

					// Number of inputs
					if ( itr->value.HasMember( "NumberInputs" ) )
					{
						newTemplate.mNumberInputs = itr->value.FindMember( "NumberInputs" )->value.GetUint( );
					}

					// Number of outputs
					if ( itr->value.HasMember( "NumberOutputs" ) )
					{
						newTemplate.mNumberOutputs = itr->value.FindMember( "NumberOutputs" )->value.GetUint( );
					}

					// If function, need to get paramater layouts
					if ( newTemplate.IsFunction( ) )
					{
						if ( itr->value.HasMember( "ParameterLayouts" ) )
						{
							auto plArray = itr->value.FindMember( "ParameterLayouts" )->value.GetArray( );
							for ( auto& v : plArray )
							{
								auto p_array = v.GetArray( );
								u32 numberInputs = newTemplate.mNumberInputs;
								u32 numberOutputs = newTemplate.mNumberOutputs;

								// If #inputs + #outputs != arraySize, then there's an error
								if ( numberInputs + numberOutputs != p_array.Size( ) )
								{
									// Throw error
								}

								ParameterLayout newLayout;

								// Push back input parameters
								std::string inputParameterKey = "";
								for ( u32 i = 0; i < numberInputs; ++i )
								{
									newLayout.mInputParameters.push_back( p_array[ i ].GetString( ) );
								}

								// Push back output parameters
								for ( u32 i = numberInputs; i < p_array.Size( ); ++i )
								{
									//newLayout.mOutputParameters.push_back( p_array[i].GetString( ) );
									newLayout.mOutputParameter = p_array[ i ].GetString( );
								}

								// Push back new layout into template
								newTemplate.AddParameterLayout( newLayout );
							}
						}
					}

					// Variable Declaration
					if ( itr->value.HasMember( "VariableDeclaration" ) )
					{
						newTemplate.mVariableDeclaration = itr->value.FindMember( "VariableDeclaration" )->value.GetString( );
					}

					// Variable Definition
					if ( itr->value.HasMember( "VariableDefinition" ) )
					{
						newTemplate.mVariableDefinition = itr->value.FindMember( "VariableDefinition" )->value.GetString( );
					}

					// Inputs
					if ( itr->value.HasMember( "Inputs" ) )
					{
						auto inputObj = itr->value.FindMember( "Inputs" )->value.GetObject( );
						for ( Value::ConstMemberIterator inputItr = inputObj.MemberBegin( ); inputItr != inputObj.MemberEnd( ); ++inputItr )
						{
							InputConnection input;
							input.mName = inputItr->name.GetString( );

							// Get primitive type
							if ( inputItr->value.HasMember( "PrimitiveType" ) )
							{
								input.mPrimitiveType = inputItr->value.FindMember( "PrimitiveType" )->value.GetString( );
							}

							// Get default value
							if ( inputItr->value.HasMember( "Default" ) )
							{
								input.mDefaultValue = inputItr->value.FindMember( "Default" )->value.GetString( );
							}

							// Add new input
							newTemplate.mInputs[ input.mName ] = input;
						}
					}

					// Outputs
					if ( itr->value.HasMember( "Outputs" ) )
					{
						auto outputObj = itr->value.FindMember( "Outputs" )->value.GetObject( );
						for ( Value::ConstMemberIterator outputItr = outputObj.MemberBegin( ); outputItr != outputObj.MemberEnd( ); ++outputItr )
						{
							OutputConnection output;
							output.mName = outputItr->name.GetString( );

							// Get primitive type
							if ( outputItr->value.HasMember( "PrimitiveType" ) )
							{
								output.mPrimitiveType = outputItr->value.FindMember( "PrimitiveType" )->value.GetString( );
							}

							// Get code template
							if ( outputItr->value.HasMember( "CodeTemplate" ) )
							{
								output.mCodeTemplate = outputItr->value.FindMember( "CodeTemplate" )->value.GetString( );
							}

							// Add new output
							newTemplate.mOutputs[ output.mName ] = output;
						}
					}

					// If all is well, add the template
					if ( !bErrorOccured )
					{
						mTemplates[ templateName ] = newTemplate;
					}
				}
			}
		}

		return 1;
	}

	//=========================================================================================================================

	void ShaderGraphNode::Clear( )
	{
		// Clear all links
		mLinks.clear( );
		mName = "INVALID";
		mIsUniform = false;
		mIsVariableDeclared = false;
		mIsVariableDefined = false;
		mUniformLocation = 0;
		mDefaultOverrides.clear( );
		mTemplate = nullptr;
	}

	//=========================================================================================================================

	void ShaderGraph::ClearGraph( )
	{
		if ( !mNodes.empty( ) )
		{
			mNodes.clear( );
		}

		// Clear the main surface node
		mMainSurfaceNode.Clear( );
		mTextureSamplerLocation = 0;
	}

	//=========================================================================================================================

	s32 ShaderGraph::Deserialize( const std::string& filePath )
	{
		// This will need to happen only AFTER validation has been checked - otherwise could end up with empty graph
		ClearGraph( );

		s32 status = 1;

		// Get JSON doc
		Document document = ShaderGraph::GetJSONDocumentFromFilePath( filePath, &status );

		// Failed to serialize document
		if ( status != 1 )
		{
			// Log error

			// Return status
			return status;
		}

		// Parse shader name
		if ( document.HasMember( "ShaderGraphName" ) )
		{
			Value::MemberIterator name = document.FindMember( "ShaderGraphName" );
			if ( name->value.IsString( ) )
			{
				mName = name->value.GetString( );
			}
		}

		// Parse Nodes to place into appropriate maps
		if ( document.HasMember( "Nodes" ) )
		{
			Value::MemberIterator nodes = document.FindMember( "Nodes" );
			if ( nodes->value.IsObject( ) )
			{
				// Get nodes object
				auto nodesObj = nodes->value.GetObject( );

				// Iterate over members
				for ( Value::ConstMemberIterator itr = nodesObj.MemberBegin( ); itr != nodesObj.MemberEnd( ); ++itr )
				{
					bool bErrorOccured = false;

					if ( !itr->value.IsObject( ) )
					{
						bErrorOccured = true;
						continue;
					}

					// Have to link to templates somehow to reference these nodes
					auto nodeName = itr->name.GetString( );

					// Create new node to place back
					ShaderGraphNode node;

					// Set name
					node.mName = nodeName;

					// Get template name
					std::string nodeTemplateName;
					if ( itr->value.HasMember( "Template" ) )
					{
						// Get template name
						auto tmpItr = itr->value.FindMember( "Template" );
						nodeTemplateName = tmpItr->value.GetString( );

						// Get the template
						const ShaderGraphNodeTemplate* nodeTemplate = ShaderGraph::GetTemplate( nodeTemplateName );

						// If texture, then need to set uniform location and incrememnt
						if ( nodeTemplateName.compare( "Texture2DSamplerNode" ) == 0 )
						{
							node.mUniformLocation = mTextureSamplerLocation++;
						}

						// Set template
						if ( nodeTemplate )
						{
							node.mTemplate = nodeTemplate;
						}
						else
						{
							// Throw error
							bErrorOccured = true;
						}
					}

					if ( itr->value.HasMember( "IsUniform" ) )
					{
						node.mIsUniform = itr->value.FindMember( "IsUniform" )->value.GetBool( );
					}

					// Add new node if no error occured
					if ( !bErrorOccured )
					{
						AddNode( node );
					}
					else
					{
						// Let user know that node could not be added
					}
				}
			}
		}

		if ( document.HasMember( "NodeLinks" ) )
		{
			auto nodeLinksItr = document.FindMember( "NodeLinks" );
			if ( nodeLinksItr->value.IsObject( ) )
			{
				// Get node links object
				const Value& nodeLinksObj = nodeLinksItr->value.GetObject( );

				for ( Value::ConstMemberIterator itr = nodeLinksObj.MemberBegin( ); itr != nodeLinksObj.MemberEnd( ); ++itr )
				{
					bool bErrorOccured = false;

					if ( !itr->value.IsObject( ) )
					{
						bErrorOccured = true;
						continue;
					}

					// Get node object
					auto nodeObj = itr->value.GetObject( );

					// Get node from graph
					ShaderGraphNode* node = const_cast< ShaderGraphNode* >( GetNode( itr->name.GetString( ) ) );

					if ( node )
					{
						// Inputs
						if ( nodeObj.HasMember( "Inputs" ) )
						{
							// Get input object
							auto inputObj = nodeObj.FindMember( "Inputs" )->value.GetObject( );

							for ( Value::ConstMemberIterator iItr = inputObj.MemberBegin( ); iItr != inputObj.MemberEnd( ); ++iItr )
							{
								// If a link is defined
								if ( !iItr->value.ObjectEmpty( ) )
								{
									// New link made
									NodeLink newLink;

									// Get input object
									auto inputItr = iItr->value.GetObject( ).MemberBegin( );

									// Get input from template
									const InputConnection* input = const_cast< ShaderGraphNodeTemplate* >( node->mTemplate )->GetInput( iItr->name.GetString( ) );

									// Set shader node connector
									const ShaderGraphNode* connectingNode = GetNode( inputItr->name.GetString( ) );

									// Error check
									if ( !connectingNode )
									{
										bErrorOccured = true;
									}

									// Set node
									newLink.mConnectingNode = connectingNode;

									// Define new output connection
									const OutputConnection* output = nullptr;

									// If connecting node valid
									if ( connectingNode )
									{
										// Set output
										output = const_cast<ShaderGraphNodeTemplate*> ( connectingNode->mTemplate )->GetOutput( inputItr->value.GetString( ) );
									}

									// Set to connection
									newLink.mTo = input;

									// Set from connection
									newLink.mFrom = output;

									// Add new link
									if ( !bErrorOccured )
									{
										node->AddLink( newLink );
									}
									else
									{
										// Let user know error occurred
									}
								}
							}
						}

						if ( nodeObj.HasMember( "DefaultOverrides" ) )
						{
							// Get input object
							auto defaultObj = nodeObj.FindMember( "DefaultOverrides" )->value.GetObject( );

							for ( Value::ConstMemberIterator iItr = defaultObj.MemberBegin( ); iItr != defaultObj.MemberEnd( ); ++iItr )
							{
								// Get input from template
								const InputConnection* input = const_cast<ShaderGraphNodeTemplate*>( node->mTemplate )->GetInput( iItr->name.GetString( ) );

								// If input exists, then push back input 
								if ( input )
								{
									InputConnection newInputOverride = *input;

									// Put in new value
									newInputOverride.mDefaultValue = iItr->value.GetString( );

									// Push back override
									node->AddOverride( newInputOverride );
								}
							}
						}
					}
					else
					{
						// Throw error 
						bErrorOccured = true;
					}
				}
			}
		}

		if ( document.HasMember( "MainNode" ) )
		{
			// Set up main node template
			mMainSurfaceNode.mTemplate = GetTemplate( "MainNodeTemplate" );

			if ( mMainSurfaceNode.mTemplate == nullptr )
			{
				// Something terrible happened
			}

			auto mainNodeItr = document.FindMember( "MainNode" );
			if ( mainNodeItr->value.IsObject( ) )
			{
				// Get node links object
				const Value& mainNodeObj = mainNodeItr->value.GetObject( );

				for ( Value::ConstMemberIterator itr = mainNodeObj.MemberBegin( ); itr != mainNodeObj.MemberEnd( ); ++itr )
				{
					bool bErrorOccured = false;

					// Inputs
					if ( mainNodeObj.HasMember( "Inputs" ) )
					{
						// Get input object
						auto inputObj = mainNodeObj.FindMember( "Inputs" )->value.GetObject( );

						for ( Value::ConstMemberIterator iItr = inputObj.MemberBegin( ); iItr != inputObj.MemberEnd( ); ++iItr )
						{
							// If a link is defined
							if ( !iItr->value.ObjectEmpty( ) )
							{
								// New link made
								NodeLink newLink;

								// Get input object
								auto inputItr = iItr->value.GetObject( ).MemberBegin( );

								// Get input from template
								const InputConnection* input = const_cast<ShaderGraphNodeTemplate*>( mMainSurfaceNode.mTemplate )->GetInput( iItr->name.GetString( ) );

								// Set shader node connector
								const ShaderGraphNode* connectingNode = GetNode( inputItr->name.GetString( ) );

								// Error check
								if ( !connectingNode )
								{
									bErrorOccured = true;
								}

								// Set node
								newLink.mConnectingNode = connectingNode;

								// Define new output connection
								const OutputConnection* output = nullptr;

								// If connecting node valid
								if ( connectingNode )
								{
									// Set output
									if ( connectingNode->mTemplate )
									{
										output = const_cast<ShaderGraphNodeTemplate*> ( connectingNode->mTemplate )->GetOutput( inputItr->value.GetString( ) );
									}
								}

								// Set to connection
								newLink.mTo = input;

								// Set from connection
								newLink.mFrom = output;

								// Add new link
								if ( !bErrorOccured )
								{
									mMainSurfaceNode.AddLink( newLink );
								}
								else
								{
									// Let user know error occurred
								}
							}
						}
					}
				}
			}
		}

		return status;
	}

	//========================================================================================================================= 

	std::string ShaderGraph::OutputPassTypeMetaData( const ShaderPassType& pass, s32* status )
	{
		std::string code = "";
		switch ( pass )
		{
		case ShaderPassType::Surface_StaticGeom:
		{
			code += OutputLine( "/*" );
			code += OutputLine( "* @info: This file has been generated. All changes will be lost. Copyright NerdKingdom 2017." );
			code += OutputLine( "* @file: " + mName );
			code += OutputLine( "* @passType: " + ShaderPassToString( pass ) );
			code += OutputLine( "*/\n" );
		} break;
		}

		return code;
	}

	std::string ShaderGraph::OutputVertexIncludes( const ShaderPassType& pass, s32* status )
	{
		std::string code = "";

		switch ( pass )
		{
		case ShaderPassType::Surface_StaticGeom:
		{
			code += OutputLine( "$input a_position, a_normal, a_tangent, a_texcoord0" );
			code += OutputLine( "$output v_wpos, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0\n" );
			code += OutputLine( "#include \"../common/common.sh\"\n" );
		} break;
		}

		return code;
	}

	std::string ShaderGraph::OutputVertexHeaderBeginTag( )
	{
		std::string code = "";
		code += OutputLine( "///////////////////////////////////////////////////" );
		code += OutputLine( "// VERTEX_SHADER_BEGIN ////////////////////////////" );
		return code;
	}

	std::string ShaderGraph::OutputVertexHeaderEndTag( )
	{
		std::string code = "";
		code += OutputLine( "// VERTEX_SHADER_END //////////////////////////////" );
		code += OutputLine( "///////////////////////////////////////////////////\n" );
		return code;
	}

	std::string ShaderGraph::OutputFragmentHeaderBeginTag( )
	{
		std::string code = "";
		code += OutputLine( "// FRAGMENT_SHADER_BEGIN ////////////////////////////" );
		code += OutputLine( "///////////////////////////////////////////////////\n" );
		return code;
	}

	std::string ShaderGraph::OutputFragmentHeaderEndTag( )
	{
		std::string code = "";
		code += OutputLine( "// FRAGMENT_SHADER_END //////////////////////////////" );
		code += OutputLine( "///////////////////////////////////////////////////\n" );
		return code;
	}

	std::string ShaderGraph::OutputVertexHeader( const ShaderPassType& pass, s32* status )
	{
		std::string code = "";

		// Header tag
		code += OutputVertexHeaderBeginTag( );

		// VertexHeaderInputData
		code += OutputVertexIncludes( pass, status );

		code += "/*\n";
		code += "* @info: This file has been generated. All changes will be lost. Copyright NerdKingdom 2017.\n";
		code += "* @file: " + mName + "." + ShaderPassToString( pass ) + ".VertexShader.glsl\n";
		code += "*/\n\n";

		// Get main node template
		ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast< ShaderGraphNodeTemplate* >( GetTemplate( "MainNodeTemplate" ) );

		// If not valid template
		if ( !surfaceNodeTemplate )
		{
			// Throw error 
			// Return empty code
			return code;
		}

		code += OutputLine( "// Gloabl Uniforms" );
		code += OutputLine( "uniform vec4 Global_Uniform_WORLD_TIME;" );
		code += OutputLine( "#define GLOBAL_WORLD_TIME Global_Uniform_WORLD_TIME.x" );

		// Comment for declarations
		code += OutputLine( "\n// Variable Declarations" );

		// Output all variables to be used for vertex shader pass
		switch ( pass )
		{
			case ShaderPassType::Surface_StaticGeom:
			{
				// Should only have one link
				NodeLink* link = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "WorldPositionOffset" ) );
				if ( link )
				{
					// Evaluate definitions for this node
					NodeLink* l = link;
					code += const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDeclaration( );
				}

			} break;
		}

		return code;
	}

	//==================================================================================================================

	/*
	void ShaderGraph::AddUniformsToShader( const NodeLink& link, Shader* shader )
	{
		if ( link.mConnectingNode == nullptr )
		{
			return;
		}

		// Iterate through all nodes in link and recursive add uniforms
		for ( auto& l : link.mConnectingNode->mLinks )
		{
			AddUniformsToShader( l, shader );
		}

		// Create new uniform if node is uniform
		if ( link.mConnectingNode->IsUniform( ) )
		{
			// Get uniform type from connecting node's template
			const ShaderGraphNodeTemplate* nodeTemplate = link.mConnectingNode->mTemplate;

			// Get uniform name
			std::string uniformName = const_cast< ShaderGraphNode* > ( link.mConnectingNode )->GetUniformName( );

			if ( nodeTemplate )
			{
			UniformType type = nodeTemplate->GetUniformType( );
			switch ( type )
			{
				case UniformType::TextureSampler2D:
				{
					// Texture samplers just use node name instead of uniform name
					std::string name = const_cast<ShaderGraphNode*>( link.mConnectingNode )->mName;

					UniformTextureSampler* uniform = new UniformTextureSampler( name );
					if ( !shader->AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}
				} break;

				case UniformType::Float:
				{
					UniformFloat* uniform = new UniformFloat( uniformName, 1.0f );
					if ( !shader->AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}
				} break;
				case UniformType::Vec2:
				{
					UniformVec2* uniform = new UniformVec2( uniformName, Vec2( 1.0f ) );
					if ( !shader->AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
				}

				} break;

				case UniformType::Vec3:
				{
					UniformVec3* uniform = new UniformVec3( uniformName, Vec3( 1.0f ) );
					if ( !shader->AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}

				} break;
				case UniformType::Vec4:
				{
					UniformVec4* uniform = new UniformVec4( uniformName, Vec4( 1.0f ) );
					if ( !shader->AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}

				} break;

				case UniformType::Mat4: 
				{
				// Do nothing for now
				} break;
				default:
				{
				// Error
				} break;
			}
			}
		}
	}
	*/

	//==================================================================================================================

	/*
	Shader* ShaderGraph::CreateShader( ShaderPassType pass )
	{
	Shader* newShader = new Shader( );
	ShaderParameters params;
	params.shaderName = mName + "." + ShaderPassToString( pass ) + ".shader";

	// Get main node template
	ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast<ShaderGraphNodeTemplate*>( GetTemplate( "MainNodeTemplate" ) );

	// If not valid template
	if ( !surfaceNodeTemplate )
	{
	// Throw error
	// Shader not valid
	}

	// Output all variables to be used for vertex shader pass
	switch ( pass )
	{
	case ShaderPassType::Surface_StaticGeom:
	{
	// Evaluate world position offsets for world position offets
	NodeLink* link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "BaseColor" ) );
	if ( link )
	{
	// Need to build uniforms from this node
	AddUniformsToShader( *link, newShader );
	}

	link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "Normal" ) );
	if ( link )
	{
	// Need to build uniforms from this node
	AddUniformsToShader( *link, newShader );
	}

	link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "Metallic" ) );
	if ( link )
	{
	// Need to build uniforms from this node
	AddUniformsToShader( *link, newShader );
	}

	link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "Roughness" ) );
	if ( link )
	{
	// Need to build uniforms from this node
	AddUniformsToShader( *link, newShader );
	}

	link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "Emissive" ) );
	if ( link )
	{
	// Need to build uniforms from this node
	AddUniformsToShader( *link, newShader );
	}

	link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "AO" ) );
	if ( link )
	{
	// Need to build uniforms from this node
	AddUniformsToShader( *link, newShader );
	}

	link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "WorldPositionOffset" ) );
	if ( link )
	{
	// Need to build uniforms from this node
	AddUniformsToShader( *link, newShader );
	}
	} break;
	}

	// Have to fill out shader parameters


	// Then have to iterate through main node links and all uniforms for shader to use

	//return newShader;
	//}
	*/

	//==================================================================================================================

	std::string ShaderGraph::BeginVertexMain( const ShaderPassType& pass, s32* status )
	{
		std::string code = "\n// Vertex Main\n";
		code += "void main()\n";
		code += "{\n";
		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::EndVertexMain( const ShaderPassType& pass, s32* status )
	{
		// Need to iterate through nodes and make sure all set back to not being declared or defined
		for ( auto& n : mNodes )
		{
			n.second.SetDeclared( false );
			n.second.SetDefined( false );
		}

		std::string code = "}\n";
		code += OutputVertexHeaderEndTag( );
		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::OutputVertexMain( const ShaderPassType& pass, s32* status )
	{
		std::string code = "";

		// Get main node template
		ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast<ShaderGraphNodeTemplate*>( GetTemplate( "MainNodeTemplate" ) );

		// If not valid template
		if ( !surfaceNodeTemplate )
		{
			// Throw error 
			// Return empty code
			return code;
		}

		// Output all variables to be used for vertex shader pass
		switch ( pass )
		{
		case ShaderPassType::Surface_StaticGeom:
		{
			// Output base suface static geom code

			// Evaluate world position offsets for world position offets
			NodeLink* link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "WorldPositionOffset" ) );

			// Need this for possible texcoords
			code += OutputTabbedLine( "v_texcoord0 = a_texcoord0;\n" );

			// Other defaults ( Not sure what should go here just yet )
			code += OutputTabbedLine( "vec4 normal = a_normal * 2.0 - 1.0;" );
			code += OutputTabbedLine( "vec3 wnormal = mul( u_model[0], vec4( normal.xyz, 0.0 ) ).xyz;\n" );
			code += OutputTabbedLine( "vec4 tangent = a_tangent * 2.0 - 1.0;" );
			code += OutputTabbedLine( "vec3 wtangent = mul( u_model[0], vec4( tangent.xyz, 0.0 ) ).xyz;\n" );
			code += OutputTabbedLine( "vec3 viewNormal = normalize( mul( u_view, vec4( wnormal, 0.0 ) ).xyz );" );
			code += OutputTabbedLine( "vec3 viewTangent = normalize( mul( u_view, vec4( wtangent, 0.0 ) ).xyz );" );
			code += OutputTabbedLine( "vec3 viewBitangent = cross( viewNormal, viewTangent ) * tangent.w;\n" );
			code += OutputTabbedLine( "mat3 tbn = mat3( viewTangent, viewBitangent, viewNormal );\n" );
			code += OutputTabbedLine( "v_normal = viewNormal;" );
			code += OutputTabbedLine( "v_tangent = viewTangent;" );
			code += OutputTabbedLine( "v_bitangent = viewBitangent;" );

			// If link exists
			if ( link )
			{
				// Evaluate definitions for this node
				NodeLink* l = link;
				if ( l->mConnectingNode->IsDeclared( ) )
				{
					code += OutputTabbedLine( const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateVariableDefinition( ) );
				}

				if ( l->mFrom )
				{
					// Get code evaluation at link output
					std::string linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );
					std::string evalType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );
					code += OutputTabbedLine( "vec3 wpos = mul( u_model[0], vec4( a_position, 1.0 ) ).xyz;" );
					code += OutputTabbedLine( "wpos += " + ShaderGraph::TransformOutputType( linkEval, evalType, +"vec3" ) + ";" );
				}
				else
				{
					code += OutputErrorBlock( "INVALID: WorldPositionOffset: Link" + l->mConnectingNode->mName + ";" );
				}
			}
			// Otherwise, vertex position doesn't get changed
			else
			{
				code += OutputTabbedLine( "vec3 wpos = mul( u_model[0], vec4( a_position, 1.0 ) ).xyz;" );
			}

			// Final position output 
			code += OutputTabbedLine( "vec3 view = mul( u_view, vec4( wpos, 0.0 ) ).xyz;" );
			code += OutputTabbedLine( "v_view = mul(view, tbn);\n" );
			code += OutputTabbedLine( "v_wpos = wpos;\n" );
			code += OutputTabbedLine( "gl_Position = mul( u_viewProj, vec4( wpos, 1.0 ) );\n" );


		} break;
		}

		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::OutputFragmentIncludes( const ShaderPassType& pass, s32* status )
	{
		std::string code = "";

		switch ( pass )
		{
		case ShaderPassType::Surface_StaticGeom:
		{
			code += OutputLine( "$input v_wpos, v_view, v_normal, v_tangent, v_bitangent, v_texcoord0\n" );
			code += OutputLine( "#include \"../common/common.sh\"\n" );
		} break;
		}

		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::OutputFragmentHeader( const ShaderPassType& pass, s32* status )
	{
		std::string code = "";

		// Header tag
		code += OutputFragmentHeaderBeginTag( );

		// Fragment header includes
		code += OutputFragmentIncludes( pass, status );

		code += "/*\n";
		code += "* @info: This file has been generated. All changes will be lost. Copyright NerdKingdom 2017.\n";
		code += "* @file: " + mName + "." + ShaderPassToString( pass ) + ".FragmentShader.glsl\n";
		code += "*/\n\n";

		// Get main node template
		ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast< ShaderGraphNodeTemplate* >( GetTemplate( "MainNodeTemplate" ) );

		// If not valid template
		if ( !surfaceNodeTemplate )
		{
			// Throw error 
			// Return empty code
			return code;
		}
		// Comment for declarations
		code += OutputLine( "// Gloabl Uniforms" );
		code += OutputLine( "uniform vec4 Global_Uniform_WORLD_TIME;" );
		code += OutputLine( "#define GLOBAL_WORLD_TIME Global_Uniform_WORLD_TIME.x" );

		// Comment for declarations
		code += OutputLine( "\n// Variable Declarations" );

		// Output all variables to be used for vertex shader pass
		switch ( pass )
		{
		case ShaderPassType::Surface_StaticGeom:
		{
			// BaseColor link
			NodeLink* baseColorLink = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "BaseColor" ) );
			if ( baseColorLink )
			{
				// Evaluate definitions for this node
				NodeLink* l = baseColorLink;
				code += const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDeclaration( );
			}

			// Normal link
			NodeLink* normalLink = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "Normal" ) );
			if ( normalLink )
			{
				// Evaluate definitions for this node
				NodeLink* l = normalLink;
				code += const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDeclaration( );
			}

			// Metallic link
			NodeLink* metallicLink = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "Metallic" ) );
			if ( metallicLink )
			{
				// Evaluate definitions for this node
				NodeLink* l = metallicLink;
				code += const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDeclaration( );
			}

			// Roughness link
			NodeLink* roughnessLink = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "Roughness" ) );
			if ( roughnessLink )
			{
				// Evaluate definitions for this node
				NodeLink* l = roughnessLink;
				code += const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDeclaration( );
			}

			// Emissive link
			NodeLink* emissiveLink = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "Emissive" ) );
			if ( emissiveLink )
			{
				// Evaluate definitions for this node
				NodeLink* l = emissiveLink;
				code += const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDeclaration( );
			}

			// AO link
			NodeLink* aoLink = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "AO" ) );
			if ( aoLink )
			{
				// Evaluate definitions for this node
				NodeLink* l = aoLink;
				code += const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDeclaration( );
			}

		} break;
		}

		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::BeginFragmentMain( const ShaderPassType& pass, s32* status )
	{
		std::string code = "\n// Fragment Main\n";
		code += "void main()\n";
		code += "{\n";
		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::OutputFragmentMain( const ShaderPassType& pass, s32* status )
	{
		std::string code = "";

		// Get main node template
		ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast<ShaderGraphNodeTemplate*>( GetTemplate( "MainNodeTemplate" ) );

		// If not valid template
		if ( !surfaceNodeTemplate )
		{
			// Throw error 
			// Return empty code
			return code;
		}

		if ( !mMainSurfaceNode.mTemplate )
		{
			return code;
		}

		// Output all variables to be used for vertex shader pass
		switch ( pass )
		{
		case ShaderPassType::Surface_StaticGeom:
		{
			// Output base suface static geom code
			code += OutputTabbedLine( "mat3 tbn = mat3( normalize( v_tangent ), normalize( v_bitangent ), normalize( v_normal ) );\n" );
			code += OutputLine( "#if BGFX_SHADER_LANGUAGE_HLSL" );
			code += OutputTabbedLine( "tbn = transpose( tbn );" );
			code += OutputLine( "#endif\n" );

			// Evaluate world position offsets for world position offets
			NodeLink* link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "BaseColor" ) );

			code += OutputTabbedLine( "// Base Color" );
			// If link exists
			if ( link )
			{
				// Evaluate definitions for this node
				NodeLink* l = link;
				if ( !l->mConnectingNode->IsDefined( ) )
				{
					std::string appendedCode = OutputLine( const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDefinition( ) );
					code += appendedCode;
				}

				// Get code evaluation at link output

				if ( l->mFrom )
				{
					std::string linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

					// Get output type so we can transform correctly
					std::string fromOutputType = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

					// Transform this type to required type for output ( could make this more specific to get away from branches )
					linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "vec4" );

					code += OutputTabbedLine( "gl_FragData[0] = " + linkEval + ";\n" );
				}
				else
				{
					code += OutputErrorBlock( "INVALID: BaseColor: Link" + l->mConnectingNode->mName + ";" );
				}

			}
			// Default base color
			else
			{
				std::string baseColor = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "BaseColor" )->mDefaultValue;
				code += OutputTabbedLine( "gl_FragData[0] = " + baseColor + ";\n" );
			}

			// Normals
			code += OutputTabbedLine( "// Normal" );
			link = const_cast<NodeLink*> ( mMainSurfaceNode.GetLink( "Normal" ) );
			if ( link )
			{
				// Evaluate definitions for this node
				NodeLink* l = link;
				if ( !l->mConnectingNode->IsDefined( ) )
				{
					code += OutputTabbedLine( const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateVariableDefinition( ) );
				}

				if ( l->mFrom )
				{
					// Get code evaluation at link output
					std::string linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

					// Get output type so we can transform correctly
					std::string fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

					// Transform this type to required type for output ( could make this more specific to get away from branches )
					linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "vec3" );

					code += OutputTabbedLine( "vec3 normal = vec3( 0, 0, 1 );" );
					code += OutputTabbedLine( "normal.xyz = " + linkEval + " * 2.0 - 1.0;" );
					code += OutputTabbedLine( "normal = normalize( mul( tbn, normal ) );" );
					code += OutputTabbedLine( "vec3 wnormal = normalize( mul( u_invView, vec4( normal, 0.0 ) ).xyz );" );
					code += OutputTabbedLine( "gl_FragData[1] = vec4( encodeNormalUint( wnormal ), 1.0 );\n" );
				}
				else
				{
					code += OutputErrorBlock( "INVALID: Normal: Link" + l->mConnectingNode->mName + ";" );
				}
			}
			// Default normals
			else
			{
				std::string normal = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Normal" )->mDefaultValue;
				normal = ShaderGraph::FindReplaceAllMetaTag( normal, "#V_NORMAL", "v_normal" );
				code += OutputTabbedLine( "gl_FragData[1] = vec4( encodeNormalUint( " + normal + " ), 1.0 );\n" );
			}

			code += OutputTabbedLine( "// Material Properties" );
			std::string metallic, roughness, ao;

			// Metallic
			link = const_cast<NodeLink*> ( mMainSurfaceNode.GetLink( "Metallic" ) );
			if ( link )
			{
				// Evaluate definitions for this node
				NodeLink* l = link;
				if ( !l->mConnectingNode->IsDefined( ) )
				{
					code += OutputTabbedLine( const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateVariableDefinition( ) );
				}

				if ( l->mFrom )
				{
					// Get code evaluation at link output
					std::string linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

					// Strip away global defines
					linkEval = ShaderGraph::ReplaceAllGlobalMetaTags( linkEval );

					// Get output type so we can transform correctly
					std::string fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

					// Transform to float
					linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "float" );

					metallic = linkEval;
				}
				else
				{
					code += OutputErrorBlock( "INVALID: Metallic: Link" + l->mConnectingNode->mName + ";" );
				}
			}
			// Default metallic
			else
			{
				metallic = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Metallic" )->mDefaultValue;
			}

			// Roughness
			link = const_cast<NodeLink*> ( mMainSurfaceNode.GetLink( "Roughness" ) );
			if ( link )
			{
				// Evaluate definitions for this node
				NodeLink* l = link;
				if ( !l->mConnectingNode->IsDefined( ) )
				{
					code += OutputTabbedLine( const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateVariableDefinition( ) );
				}

				if ( l->mFrom )
				{
					// Get code evaluation at link output
					std::string linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

					// Get output type so we can transform correctly
					std::string fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

					// Transform to float
					linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "float" );

					roughness = linkEval;
				}
				else
				{
					code += OutputErrorBlock( "INVALID: Metallic: Link" + l->mConnectingNode->mName + ";" );
				}
			}
			// Default roughness
			else
			{
				roughness = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Roughness" )->mDefaultValue;
			}

			// AO
			link = const_cast<NodeLink*> ( mMainSurfaceNode.GetLink( "AO" ) );
			if ( link )
			{
				// Evaluate definitions for this node
				NodeLink* l = link;
				if ( !l->mConnectingNode->IsDefined( ) )
				{
					code += OutputTabbedLine( const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateVariableDefinition( ) );
				}

				if ( l->mFrom )
				{
					// Get code evaluation at link output
					std::string linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

					// Get output type so we can transform correctly
					std::string fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

					// Transform to float
					linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "float" );

					ao = linkEval;
				}
				else
				{
					code += OutputErrorBlock( "INVALID: AO: Link" + l->mConnectingNode->mName + ";" );
				}
			}
			// Default ao
			else
			{
				ao = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "AO" )->mDefaultValue;
			}

			// Output Material Properties
			code += OutputTabbedLine( "gl_FragData[2] = vec4( " + metallic + ", " + roughness + ", " + ao + ", 1.0);\n" );

			// Emissive
			code += OutputTabbedLine( "// Emissive" );
			link = const_cast<NodeLink*> ( mMainSurfaceNode.GetLink( "Emissive" ) );
			if ( link )
			{
				// Evaluate definitions for this node
				NodeLink* l = link;
				if ( !l->mConnectingNode->IsDefined( ) )
				{
					code += OutputTabbedLine( const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateVariableDefinition( ) );
				}

				if ( l->mFrom )
				{
					// Get code evaluation at link output
					std::string linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

					// Get output type so we can transform correctly
					std::string fromOutputType = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

					// Transform this type to required type for output ( could make this more specific to get away from branches )
					linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "vec4" );

					code += OutputTabbedLine( "gl_FragData[3] = " + linkEval + ";\n" );
				}
				else
				{
					code += OutputErrorBlock( "INVALID: Emissive: Link" + l->mConnectingNode->mName + ";" );
				}
			}
			// Default emissive
			else
			{
				std::string emissive = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Emissive" )->mDefaultValue;
				code += OutputTabbedLine( "gl_FragData[3] = " + emissive + ";\n" );
			}

		} break;
		}

		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::EndFragmentMain( const ShaderPassType& pass, s32* status )
	{
		// Need to iterate through nodes and make sure all set back to not being declared or defined
		for ( auto& n : mNodes )
		{
			n.second.SetDeclared( false );
			n.second.SetDefined( false );
		}

		std::string code = "}\n";
		code += OutputFragmentHeaderEndTag( );

		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::TransformOutputType( const std::string& code, const std::string& type, const std::string& requiredType )
	{
		// If the same, then return
		if ( type.compare( requiredType ) == 0 )
		{
			return code;
		}
		else if ( requiredType.compare( "vec3" ) == 0 )
		{
			if ( type.compare( "float" ) == 0 )
			{
				return "vec3(" + code + ", " + code + ", " + code + ")";
			}
			else if ( type.compare( "vec2" ) == 0 )
			{
				return "vec3(" + code + ", 1.0)";
			}
			else if ( type.compare( "vec4" ) == 0 )
			{
				return code + ".xyz";
			}
			else
			{
				return code;
			}
		}
		else if ( requiredType.compare( "vec4" ) == 0 )
		{
			if ( type.compare( "float" ) == 0 )
			{
				return "vec4(vec3(" + code + ", " + code + ", " + code + "), 1.0 )";
			}
			else if ( type.compare( "vec2" ) == 0 )
			{
				return "vec4(" + code + ", 0.0, 1.0)";
			}
			else if ( type.compare( "vec3" ) == 0 )
			{
				return "vec4(" + code + ", 1.0)";
			}
			else
			{
				return code;
			}
		}
		else if ( requiredType.compare( "float" ) == 0 )
		{
			if ( type.compare( "vec2" ) == 0 )
			{
				return code + ".x";
			}
			else if ( type.compare( "vec3" ) == 0 )
			{
				return code + ".x";
			}
			else if ( type.compare( "vec4" ) == 0 )
			{
				return code + ".x";
			}
			else
			{
				return code;
			}
		}

		return code;
	}

	//==================================================================================================================

	std::string ShaderGraph::ReplaceAllGlobalMetaTags( const std::string& code )
	{
		std::string returnCode = code;

		// Replace V_TEXCOORD0 with appropriate tag
		for ( u32 i = 0; i < ShaderGraph::TagCount( returnCode, "#V_TEXCOORD0" ); ++i )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( returnCode, "#V_TEXCOORD0", "v_texcoord0" );
		}

		// Replace G_WORLD_TIME with appropriate tag
		for ( u32 i = 0; i < ShaderGraph::TagCount( returnCode, "#G_WORLD_TIME" ); ++i )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( returnCode, "#G_WORLD_TIME", "GLOBAL_WORLD_TIME" );
		}

		// Replace V_NORMAL with appropriate tag
		for ( u32 i = 0; i < ShaderGraph::TagCount( returnCode, "#V_NORMAL" ); ++i )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( returnCode, "#V_NORMAL", "v_normal" );
		}

		// Replace V_TANGENT with appropriate tag
		for ( u32 i = 0; i < ShaderGraph::TagCount( returnCode, "#V_TANGENT" ); ++i )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( returnCode, "#V_TANGENT", "v_tangent" );
		}

		// Replace V_BITANGENT with appropriate tag
		for ( u32 i = 0; i < ShaderGraph::TagCount( returnCode, "#V_BITANGENT" ); ++i )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( returnCode, "#V_BITANGENT", "v_bitangent" );
		}

		// Replace V_WPOS with appropriate tag
		for ( u32 i = 0; i < ShaderGraph::TagCount( returnCode, "#V_WPOS" ); ++i )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( returnCode, "#V_WPOS", "v_wpos" );
		}

		// Replace V_VIEW with appropriate tag
		for ( u32 i = 0; i < ShaderGraph::TagCount( returnCode, "#V_VIEW" ); ++i )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( returnCode, "#V_VIEW", "v_view" );
		}

		return returnCode;
	}

	//==================================================================================================================

	std::string ShaderGraph::FindReplaceAllMetaTag( const std::string& code, const std::string& toFind, const std::string& replaceWith )
	{
		std::string retCode = code;

		// Search for begin
		std::size_t pos = retCode.find( toFind );
		while ( pos != std::string::npos )
		{
			retCode = FindReplaceMetaTag( retCode, toFind, replaceWith );
			pos = retCode.find( toFind, pos + 1 );
		}

		return retCode;
	}

	//==================================================================================================================

	std::string ShaderGraph::FindReplaceMetaTag( const std::string& code, const std::string& toFind, const std::string& replaceWith )
	{
		std::string returnStr = "";

		// Search for begin
		std::size_t foundBegin = code.find( toFind );
		std::size_t findSize = toFind.length( );

		/*
		example:
		#OUTPUT_TYPE #NODE_NAME;

		Replace "#OUTPUT_TYPE" with "float"

		subStrBefore = "float"
		subStrAfter = " #NODE_NAME;"

		finalString = subStrBefore + subStrAfter;
		*/

		std::string subStrBefore = "";
		std::string subStrAfter = "";

		// If found, then replace and return
		if ( foundBegin != std::string::npos )
		{
			subStrBefore = code.substr( 0, foundBegin );
			subStrAfter = code.substr( foundBegin + findSize );
			return ( subStrBefore + replaceWith + subStrAfter );
		}
		// Else just return the original string
		else
		{
			return code;
		}
	}

	std::string ShaderGraphNode::BuildInputKeyFromLinks( )
	{
		std::string inputKey = "";
		for ( auto& l : mLinks )
		{
			std::string fromName = l.mFrom->mName;
			std::string outputType = const_cast<ShaderGraphNode*> ( l.mConnectingNode )->EvaluateOutputTypeAt( l.mFrom->mName );
			std::string input = ShaderGraph::FindReplaceAllMetaTag( l.mFrom->mPrimitiveType, "#OUTPUT_TYPE", const_cast<ShaderGraphNode*>( l.mConnectingNode )->EvaluateOutputTypeAt( l.mFrom->mName ) );
			inputKey += input;
		}

		if ( inputKey.empty( ) )
		{
			if ( mTemplate->IsFunction( ) )
			{
				// Look for empty key
				inputKey = "";
				//inputKey = const_cast< ShaderGraphNodeTemplate* > ( mTemplate )->GetOutput( "Value" )->mPrimitiveType;
			}
		}

		return inputKey;
	}

	std::string ShaderGraphNode::EvaluateOutputTypeAt( const std::string& outputName )
	{
		std::string returnOutputType = "";

		// Handle differently, of course
		if ( mTemplate->IsFunction( ) )
		{
			// Build input key from links ( could be empty )
			std::string inputKey = BuildInputKeyFromLinks( );

			// Need to code replace to make sure that empties are handled correctly
			inputKey = ShaderGraph::FindReplaceAllMetaTag( inputKey, "#TYPE(", "" );
			inputKey = ShaderGraph::FindReplaceAllMetaTag( inputKey, ")", "" );

			// Get paramterlayout from template
			const ParameterLayout* layout = const_cast< ShaderGraphNodeTemplate* >( mTemplate )->GetLayout( inputKey );

			// Get output from output parameter
			if ( layout )
			{
				return layout->mOutputParameter;
			}
		}

		else
		{
			// Get output connection
			const OutputConnection* output = const_cast< ShaderGraphNodeTemplate* > ( mTemplate )->GetOutput( outputName );

			// Otherwise return primitive type of output connection
			returnOutputType = output->mPrimitiveType;
		}

		return returnOutputType;
	}

	std::string ShaderGraphNode::EvaluateOutputCodeAt( const std::string& name )
	{
		std::string code = "";

		// Get code template from template
		if ( mTemplate == nullptr )
		{
			return "ERROR: EvalutateOutputCodeAt: " + mName + ": template is null!";
		}

		const OutputConnection* output = const_cast< ShaderGraphNodeTemplate* > ( mTemplate )->GetOutput( name );
		if ( output )
		{
			code = output->mCodeTemplate;

			// Do code replacement for output
			code = ShaderGraph::FindReplaceMetaTag( code, "#NODE_NAME", mName );
		}

		// Return code
		return code;
	}

	//===========================================================================================================================

	std::string ShaderGraphNode::EvaluateVariableDefinition( )
	{
		std::string finalOutput = "";

		// At this point, ALL necessary nodes should be evaluated for their variable declarations
		// Have to evaluate for their actual definitions now
		if ( IsDefined( ) )
		{
			return finalOutput;
		}

		// Set Defined
		SetDefined( true );

		// Error
		if ( mTemplate == nullptr )
		{
			return finalOutput;
		}

		// Make sure that all variabled in links are defined
		for ( auto& l : mLinks )
		{
			if ( !const_cast<ShaderGraphNode*>( l.mConnectingNode )->IsDefined( ) )
			{
				finalOutput += OutputLine( const_cast<ShaderGraphNode*> ( l.mConnectingNode )->EvaluateVariableDefinition( ) );
			}
		}

		// If is a uniform, don't need to redefine the variable
		if ( !IsUniform( ) )
		{
			// Get variable decl template code
			std::string variableDefTemplate = mTemplate->GetVariableDefinitionTemplate( );

			// Replace with output from links if available, otherwise we'll resort to defaults
			for ( auto& l : mLinks )
			{
				if ( l.mFrom && l.mTo )
				{
					std::string codeEval = const_cast< ShaderGraphNode* > ( l.mConnectingNode )->EvaluateOutputCodeAt( l.mFrom->mName );

					// Get input name from link
					std::string inputName = "#INPUT(" + l.mTo->mName + ")";

					// Replace this input name in definition code with code evaluation
					variableDefTemplate = ShaderGraph::FindReplaceAllMetaTag( variableDefTemplate, inputName, codeEval );
				}
				else
				{
					variableDefTemplate = OutputErrorBlock( "INVALID: Node: " + mName + ": Incorrect output channel from input node: " + l.mConnectingNode->mName );
				}
			}

			// Get inputs from template and search for remaining areas to replace
			const std::unordered_map< std::string, InputConnection >& inpts = mTemplate->GetInputs( );
			for ( auto& i : inpts )
			{
				std::string inputName = "#INPUT(" + i.second.mName + ")";
				std::string defaultValue = i.second.mDefaultValue;

				// Need to get default value from either template inputs or from default overrides
				if ( HasOverride( i.second.mName ) )
				{
					defaultValue = GetOverride( i.second.mName )->mDefaultValue;
				}

				variableDefTemplate = ShaderGraph::FindReplaceAllMetaTag( variableDefTemplate, inputName, defaultValue );
			}

			// Need to find and replace for name
			variableDefTemplate = ShaderGraph::FindReplaceAllMetaTag( variableDefTemplate, "#NODE_NAME", mName );

			// Replace all global tags
			variableDefTemplate = ShaderGraph::ReplaceAllGlobalMetaTags( variableDefTemplate );

			if ( mTemplate->IsFunction( ) )
			{
				// Replace with Output Type
				for ( u32 i = 0; i < ShaderGraph::TagCount( variableDefTemplate, "#OUTPUT_TYPE" ); ++i )
				{
					variableDefTemplate = ShaderGraph::FindReplaceMetaTag( variableDefTemplate, "#OUTPUT_TYPE", EvaluateOutputTypeAt( "Value" ) );
				}
			}

			return ( finalOutput + variableDefTemplate );
		}

		else
		{
			return finalOutput;
		}
	}

	//===========================================================================================================================

	std::string ShaderGraphNode::EvaluateVariableDeclaration( )
	{
		// Final output
		std::string finalOutput = "";

		// Early out if already declared
		if ( IsDeclared( ) )
		{
			return finalOutput;
		}

		// Error
		if ( mTemplate == nullptr )
		{
			return finalOutput;
		}

		// Make sure that all variables in links are declared
		for ( auto& l : mLinks )
		{
			if ( !const_cast<ShaderGraphNode*> ( l.mConnectingNode )->IsDeclared( ) )
			{
				finalOutput += OutputLine( ( const_cast<ShaderGraphNode*> ( l.mConnectingNode )->EvaluateVariableDeclaration( ) ) );
			}
		}

		// Get variable decl template code
		std::string variableDeclTemplate = OutputLine( IsUniform( ) ? "uniform " + mTemplate->GetVariableDeclarationTemplate( ) : mTemplate->GetVariableDeclarationTemplate( ) );

		// Need to find and replace for certain things
		variableDeclTemplate = ( ShaderGraph::FindReplaceMetaTag( variableDeclTemplate, "#NODE_NAME", IsUniform( ) ? GetUniformName( ) : mName ) );

		// Replace uniform location tag
		variableDeclTemplate = ( ShaderGraph::FindReplaceMetaTag( variableDeclTemplate, "#UNIFORM_LOCATION", std::to_string( mUniformLocation ) ) );

		// Evalute output type if uniform or function
		if ( mTemplate->IsFunction( ) )
		{
			std::string outputTypeString = EvaluateOutputTypeAt( "Value" );

			// If empty string, then need backup...
			if ( outputTypeString.empty( ) )
			{
				// If no links at all, default to empty
				if ( mLinks.empty( ) )
				{

				}
				// Need to make sure at least one is set after this...I guess, not sure what to expect here
			}

			variableDeclTemplate = ( ShaderGraph::FindReplaceMetaTag( variableDeclTemplate, "#OUTPUT_TYPE", outputTypeString ) );
		}
		else if ( IsUniform( ) )
		{
			std::string actualVariable = "";

			if ( ShaderGraph::HasTag( variableDeclTemplate, "float" ) )
			{
				actualVariable = "#define " + mName + " " + GetUniformName( ) + ".x";
			}
			else if ( ShaderGraph::HasTag( variableDeclTemplate, "vec2" ) )
			{
				actualVariable = "#define " + mName + " " + GetUniformName( ) + ".xy";
			}
			else if ( ShaderGraph::HasTag( variableDeclTemplate, "vec3" ) )
			{
				actualVariable = "#define " + mName + " " + GetUniformName( ) + ".xyz";
			}

			// Need to replace the uniform with a vec3, so have to make account for that; 
			variableDeclTemplate = ShaderGraph::ReplaceTypeWithAppropriateUniformType( variableDeclTemplate );

			// Add actual variable declaration
			variableDeclTemplate += OutputLine( actualVariable );
		}

		// Set variable to being declared
		SetDeclared( true );

		// Final output return
		return ( finalOutput + variableDeclTemplate );
	}
	//========================================================================================================================= 

	u32 ShaderGraph::TagCount( const std::string& code, const std::string& tag )
	{
		u32 count = 0;

		// Search for begin
		std::size_t pos = code.find( tag );
		while ( pos != std::string::npos )
		{
			count++;
			pos = code.find( tag, pos + 1 );
		}

		return count;
	}

	//========================================================================================================================= 

	bool ShaderGraph::HasTag( const std::string& code, const std::string& tag )
	{
		// Search for begin
		std::size_t foundBegin = code.find( tag );

		// If found, then replace and return
		if ( foundBegin != std::string::npos )
		{
			return true;
		}

		return false;
	}

	//========================================================================================================================= 

	std::string ShaderGraph::ReplaceTypeWithAppropriateUniformType( const std::string& code )
	{
		std::string returnCode = "";

		// Replace float with vec4
		if ( HasTag( code, "float" ) )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( code, "float", "vec4" );
		}
		else if ( HasTag( code, "vec2" ) )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( code, "vec2", "vec4" );
		}
		else if ( HasTag( code, "vec3" ) )
		{
			returnCode = ShaderGraph::FindReplaceMetaTag( code, "vec3", "vec4" );
		}
		else
		{
			return code;
		}

		return returnCode;
	}

	//========================================================================================================================= 

	std::string ShaderGraph::GetCode( ShaderPassType type, ShaderType shaderType )
	{
		auto query = mShaderPassCode.find( type );
		if ( query != mShaderPassCode.end( ) )
		{
			switch ( shaderType )
			{
			case ShaderType::Vertex:
			{
				// Get pass code
				std::string passCode = mShaderPassCode[ type ];

				// Parse and return vertex shader code 
				passCode = ParseFromTo( OutputVertexHeaderBeginTag( ), OutputVertexHeaderEndTag( ), passCode );
				passCode = ShaderGraph::FindReplaceMetaTag( passCode, OutputVertexHeaderBeginTag( ), "" );

				// Return
				return passCode;

			} break;
			case ShaderType::Fragment:
			{
				// Get pass code
				std::string passCode = mShaderPassCode[ type ];

				// Parse and return fragment shader code 
				passCode = ParseFromTo( OutputFragmentHeaderBeginTag( ), OutputFragmentHeaderEndTag( ), passCode );
				passCode = ShaderGraph::FindReplaceMetaTag( passCode, OutputFragmentHeaderBeginTag( ), "" );

				// Return
				return passCode;

			} break;
			case ShaderType::Compute:
			{
				// Get pass code
				std::string passCode = mShaderPassCode[ type ];

				// Return compute shader code
				return passCode;

			} break;
			default:
			{
				// Just return all of it
				return mShaderPassCode[ type ];
			} break;
			}
		}

		// Node pass found that matches
		return "";
	}

	//========================================================================================================================= 

	// Start here tomorrow...
	s32 ShaderGraph::Compile( )
	{
		s32 status = 1;

		// For each pass, output shader code based on graph
		for ( u32 i = 0; i < ( u32 )ShaderPassType::Count; ++i )
		{
			std::string code = "";

			// Meta data for pass
			code += OutputPassTypeMetaData( ShaderPassType( i ), &status );

			// Vertex Shader Output
			code += OutputVertexHeader( ShaderPassType( i ), &status );
			code += BeginVertexMain( ShaderPassType( i ), &status );
			code += OutputVertexMain( ShaderPassType( i ), &status );
			code += EndVertexMain( ShaderPassType( i ), &status );

			// Fragment Shader Output
			code += OutputFragmentHeader( ShaderPassType( i ), &status );
			code += BeginFragmentMain( ShaderPassType( i ), &status );
			code += OutputFragmentMain( ShaderPassType( i ), &status );
			code += EndFragmentMain( ShaderPassType( i ), &status );

			// Geometry Shader Output 
			// Compute Shader Output
			// Tesselelation Shader Output
			// Whatever Other Crazy Shit We Want To Output

			if ( status > 0 )
			{
				mShaderPassCode[ ShaderPassType::Surface_StaticGeom ] = code;
			}
		}

		return status;
	}

}







