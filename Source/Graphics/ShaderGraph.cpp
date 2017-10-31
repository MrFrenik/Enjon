#include "Graphics/ShaderGraph.h"
#include "Graphics/Shader.h"
#include "Asset/AssetManager.h"
#include "Asset/ShaderGraphAssetLoader.h"
#include "Engine.h"

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
	std::unordered_map< Enjon::String, Enjon::ShaderGraphNodeTemplate > Enjon::ShaderGraph::mTemplates;

	Enjon::String ParseFromTo( const Enjon::String& begin, const Enjon::String& end, const Enjon::String& src )
	{
		Enjon::String returnStr = "";

		// Search for line to match
		std::size_t foundBegin = src.find( begin );
		std::size_t foundEnd = src.find( end );

		// Found
		if ( foundBegin != Enjon::String::npos && foundEnd != Enjon::String::npos )
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

	Enjon::String OutputLine( const Enjon::String& line )
	{
		return line + "\n";
	}

	//=========================================================================================================================

	Enjon::String OutputTabbedLine( const Enjon::String& line )
	{
		return "\t" + line + "\n";
	}

	Enjon::String OutputErrorBlock( const Enjon::String& line )
	{
		Enjon::String code = "";
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

	bool ShaderGraphNode::HasOverride( const Enjon::String& inputName )
	{
		return ( mDefaultOverrides.find( inputName ) != mDefaultOverrides.end( ) );
	}

	//=========================================================================================================================

	const InputConnection* ShaderGraphNode::GetOverride( const Enjon::String& inputName )
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
		Enjon::String inputKey = "";
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

	const ParameterLayout* ShaderGraphNodeTemplate::GetLayout( const Enjon::String& inputKey )
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

	const NodeLink* ShaderGraphNode::GetLink( const Enjon::String inputName )
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

	const ShaderGraphNode* ShaderGraph::GetNode( const Enjon::String& nodeName )
	{
		auto query = mNodes.find( nodeName );
		if ( query != mNodes.end( ) )
		{
			return &mNodes[ nodeName ];
		}

		return nullptr;
	}
	const ShaderGraphNodeTemplate* ShaderGraph::GetTemplate( const Enjon::String& name )
	{
		auto query = mTemplates.find( name );
		if ( query != mTemplates.end( ) )
		{
			return &mTemplates[ name ];
		}

		return nullptr;
	}

	//==========================================================================================

	Enjon::String ShaderGraph::ShaderPassToString( ShaderPassType type )
	{
		switch ( type )
		{
			case ShaderPassType::StaticGeom:
			{
				return "StaticGeom";
			}; break;

			case ShaderPassType::InstancedGeom:
			{
				return "InstancedGeom";
			}; break;
			
			case ShaderPassType::Forward_StaticGeom:
			{
				return "Forward_StaticGeom";
			}; break;

			default:
			{
				return "";
			} break;
		}
	}

	//==========================================================================================

	Document ShaderGraph::GetJSONDocumentFromFilePath( const Enjon::String& filePath, s32* status )
	{
		// Load file
		std::ifstream f;
		f.open( filePath );
		std::stringstream buffer;
		buffer << f.rdbuf( );

		Enjon::String str = buffer.str( );

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
		
	Result ShaderGraph::Serialize( Enjon::ByteBuffer& buffer )
	{
		return Result::SUCCESS;
	}

	//=========================================================================================================================

	Result ShaderGraph::Deserialize( Enjon::ByteBuffer& buffer )
	{
		return Result::SUCCESS;
	}

	//=========================================================================================================================

	s32 ShaderGraph::DeserializeTemplate( const Enjon::String& filePath )
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
					Enjon::String templateName = itr->name.GetString( );

					// Set name
					newTemplate.mName = templateName;

					// Get node type
					if ( itr->value.HasMember( "Type" ) )
					{
						Enjon::String templateStr = itr->value.FindMember( "Type" )->value.GetString( );

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
						Enjon::String uniformStr = itr->value.FindMember( "UniformType" )->value.GetString( );
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
								Enjon::String inputParameterKey = "";
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
						Enjon::String definitionCode = "";

						auto variableDef = itr->value.FindMember( "VariableDefinition" ); 

						if ( variableDef->value.IsArray( ) )
						{
							auto arr = variableDef->value.GetArray( );
							for ( auto& s : arr )
							{
								definitionCode += s.GetString( );
							}
						}
						else
						{
							definitionCode = variableDef->value.GetString( );
						}

						newTemplate.mVariableDefinition = definitionCode;
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

		return status;
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

		for ( auto& u : mUniforms )
		{
			delete u.second;
			u.second = nullptr;
		}

		// Delete shaders
		for ( auto& s : mShaders )
		{
			delete s.second;
			s.second = nullptr;
		}

		mShaders.clear( );
		mUniforms.clear( );
	}

	//=========================================================================================================================

	s32 ShaderGraph::Create( const Enjon::String& filePath )
	{ 
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
		
		ClearGraph( );

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

					if ( itr->value.HasMember( "IsUniform" ) )
					{
						node.mIsUniform = itr->value.FindMember( "IsUniform" )->value.GetBool( );
					}

					// Get template name
					Enjon::String nodeTemplateName;
					if ( itr->value.HasMember( "Template" ) )
					{
						// Get template name
						auto tmpItr = itr->value.FindMember( "Template" );
						nodeTemplateName = tmpItr->value.GetString( );

						// Get the template
						const ShaderGraphNodeTemplate* nodeTemplate = ShaderGraph::GetTemplate( nodeTemplateName );

						if ( nodeTemplate != nullptr )
						{
							// If texture, then need to set uniform location and incrememnt
							if ( nodeTemplateName.compare( "Texture2DSamplerNode" ) == 0 || nodeTemplate->mUniformType == UniformType::TextureSampler2D )
							{
								node.mUniformLocation = mTextureSamplerLocation++;

								// If is texture, then is uniform by default
								node.mIsUniform = true;
							}

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

		// Compile after deserializing
		if ( status == 1 )
		{
			return Compile( );
		} 

		// If error, then clear graph and return fail status
		ClearGraph( ); 
		return status;
	}

	//========================================================================================================================= 
		
	Result ShaderGraph::Reload( )
	{
		// Call create with file path
		s32 status = Create( mFilePath ); 

		// Return failure if failed to create
		if ( status != 1 )
		{
			return Result::FAILURE;
		}

		return Result::SUCCESS;
	}

	//========================================================================================================================= 

	Enjon::String ShaderGraph::OutputPassTypeMetaData( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = "";
		code += OutputLine( "/*" );
		code += OutputLine( "* @info: This file has been generated. All changes will be lost." );
		code += OutputLine( "* @file: " + mName );
		code += OutputLine( "* @passType: " + ShaderPassToString( pass ) );
		code += OutputLine( "*/\n" );

		return code;
	}

	Enjon::String ShaderGraph::OutputVertexHeaderBeginTag( )
	{
		Enjon::String code = "";
		code += OutputLine( "///////////////////////////////////////////////////" );
		code += OutputLine( "// VERTEX_SHADER_BEGIN ////////////////////////////" );
		return code;
	}

	Enjon::String ShaderGraph::OutputVertexHeaderEndTag( )
	{
		Enjon::String code = "";
		code += OutputLine( "// VERTEX_SHADER_END //////////////////////////////" );
		code += OutputLine( "///////////////////////////////////////////////////\n" );
		return code;
	}

	Enjon::String ShaderGraph::OutputFragmentHeaderBeginTag( )
	{
		Enjon::String code = "";
		code += OutputLine( "// FRAGMENT_SHADER_BEGIN ////////////////////////////" );
		code += OutputLine( "///////////////////////////////////////////////////\n" );
		return code;
	}

	Enjon::String ShaderGraph::OutputFragmentHeaderEndTag( )
	{
		Enjon::String code = "";
		code += OutputLine( "// FRAGMENT_SHADER_END //////////////////////////////" );
		code += OutputLine( "///////////////////////////////////////////////////\n" );
		return code;
	}

	Enjon::String ShaderGraph::OutputVertexAttributes( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = "";

		switch ( pass )
		{
			case ShaderPassType::Forward_StaticGeom:
			case ShaderPassType::StaticGeom:
			{
				// Vertex Attribute Layouts
				code += OutputLine( "layout (location = 0) in vec3 aVertexPosition;" );
				code += OutputLine( "layout (location = 1) in vec3 aVertexNormal;" );
				code += OutputLine( "layout (location = 2) in vec3 aVertexTangent;" );
				code += OutputLine( "layout (location = 3) in vec3 aVertexUV;" );
			} break;

			case ShaderPassType::InstancedGeom:
			{
				// Vertex Attribute Layouts
				code += OutputLine( "layout (location = 0) in vec3 aVertexPosition;" );
				code += OutputLine( "layout (location = 1) in vec3 aVertexNormal;" );
				code += OutputLine( "layout (location = 2) in vec3 aVertexTangent;" );
				code += OutputLine( "layout (location = 3) in vec3 aVertexUV;" );
				code += OutputLine( "layout (location = 4) in mat4 aInstanceMatrix;" );
			} break; 
		}

		return code;
	}

	Enjon::String ShaderGraph::OutputVertexHeader( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = "";

		// Get main node template
		ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast< ShaderGraphNodeTemplate* >( GetTemplate( "MainNodeTemplate" ) );

		// If not valid template
		if ( !surfaceNodeTemplate )
		{
			// Throw error 
			// Return empty code
			return code;
		}

		// Header tag
		code += OutputVertexHeaderBeginTag( );

		// Version number
		code += OutputLine( "#version 330 core\n" );

		// Vertex Attribute Layouts
		code += OutputVertexAttributes( pass, status );

		// Vertex output struct
		code += OutputLine( "\nout VS_OUT" );
		code += OutputLine( "{" );
		code += OutputTabbedLine( "vec3 FragPositionWorldSpace;" );
		code += OutputTabbedLine( "vec2 TexCoords;" );
		code += OutputTabbedLine( "mat3 TBN;" );
		code += OutputTabbedLine( "mat3 TS_TBN;" );
		code += OutputTabbedLine( "vec3 ViewPositionTangentSpace;" );
		code += OutputTabbedLine( "vec3 FragPositionTangentSpace;" );
		code += OutputLine( "} vs_out;\n" );

		// Global uniforms
		code += OutputLine( "// Gloabl Uniforms" );
		code += OutputLine( "uniform float uWorldTime = 1.0f;" );
		code += OutputLine( "uniform mat4 uViewProjection;" );
		code += OutputLine( "uniform vec3 uViewPositionWorldSpace;" );
		code += OutputLine( "uniform mat4 uModel = mat4( 1.0f );" );

		// Comment for declarations
		code += OutputLine( "\n// Variable Declarations" );

		// Output all variables to be used for vertex shader pass
		switch ( pass )
		{
			case ShaderPassType::Forward_StaticGeom:
			case ShaderPassType::InstancedGeom:
			case ShaderPassType::StaticGeom:
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

	const std::unordered_map< Enjon::String, ShaderUniform* >* ShaderGraph::GetUniforms( )
	{
		return &mUniforms;
	}

	//==================================================================================================================

	bool ShaderGraph::HasShader( ShaderPassType pass )
	{
		return ( mShaders.find( pass ) != mShaders.end( ) );
	}

	//==================================================================================================================

	const Shader* ShaderGraph::GetShader( ShaderPassType pass )
	{
		if ( HasShader( pass ) )
		{
			return mShaders[ pass ];
		}

		return nullptr;
	}

	//==================================================================================================================

	const ShaderUniform* ShaderGraph::GetUniform( const Enjon::String& uniformName )
	{
		if ( HasUniform( uniformName ) )
		{
			return mUniforms[ uniformName ];
		}

		return nullptr;
	}

	//==================================================================================================================

	bool ShaderGraph::HasUniform( const Enjon::String& uniformName )
	{
		return ( mUniforms.find( uniformName ) != mUniforms.end( ) );
	}

	//==================================================================================================================

	bool ShaderGraph::AddUniform( ShaderUniform* uniform )
	{
		if ( uniform == nullptr )
		{
			return false;
		}

		if ( HasUniform( uniform->GetName( ) ) )
		{
			return false;
		}

		mUniforms[ uniform->GetName( ) ] = uniform;
		return true;
	}

	//==================================================================================================================

	void ShaderGraph::ConstructUniforms( const NodeLink& link )
	{
		if ( link.mConnectingNode == nullptr )
		{
			return;
		}

		// Iterate through all nodes in link and recursive add uniforms
		for ( auto& l : link.mConnectingNode->mLinks )
		{
			ConstructUniforms( l );
		}

		// Create new uniform if node is uniform
		if ( link.mConnectingNode->IsUniform( ) )
		{
			// Get uniform type from connecting node's template
			const ShaderGraphNodeTemplate* nodeTemplate = link.mConnectingNode->mTemplate;

			// Get uniform name
			//Enjon::String uniformName = const_cast< ShaderGraphNode* > ( link.mConnectingNode )->GetUniformName( );
			Enjon::String uniformName = const_cast< ShaderGraphNode* > ( link.mConnectingNode )->mName;

			// Get asset manager
			Enjon::AssetManager* am = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( );

			if ( nodeTemplate )
			{
				UniformType type = nodeTemplate->GetUniformType( );
				switch ( type )
				{
				case UniformType::TextureSampler2D:
				{
					// Texture samplers just use node name instead of uniform name
					Enjon::String name = const_cast<ShaderGraphNode*>( link.mConnectingNode )->mName;

					UniformTexture* uniform = new UniformTexture( name, am->GetDefaultAsset< Enjon::Texture >( ), link.mConnectingNode->mUniformLocation );
					if ( !AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}
				} break;

				case UniformType::Float:
				{
					UniformPrimitive< f32 >* uniform = new UniformPrimitive< f32 >( uniformName, 1.0f );
					if ( !AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}

				} break;

				case UniformType::Vec2:
				{
					UniformPrimitive< Vec2 >* uniform = new UniformPrimitive< Vec2 >( uniformName, Vec2( 1.0f ) );
					if ( !AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}

				} break;

				case UniformType::Vec3:
				{
					UniformPrimitive< Vec3 >* uniform = new UniformPrimitive< Vec3 >( uniformName, Vec3( 1.0f ) );
					if ( !AddUniform( uniform ) )
					{
						// Failed to add, so delete uniform
						delete uniform;
					}

				} break;

				case UniformType::Vec4:
				{
					UniformPrimitive< Vec4 >* uniform = new UniformPrimitive< Vec4 >( uniformName, Vec4( 1.0f ) );
					if ( !AddUniform( uniform ) )
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

	//==================================================================================================================

	Enjon::String ShaderGraph::BeginVertexMain( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = "\n// Vertex Main\n";
		code += "void main()\n";
		code += "{\n";
		return code;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::EndVertexMain( const ShaderPassType& pass, s32* status )
	{
		// Need to iterate through nodes and make sure all set back to not being declared or defined
		for ( auto& n : mNodes )
		{
			n.second.SetDeclared( false );
			n.second.SetDefined( false );
		}

		Enjon::String code = "}\n";
		code += OutputVertexHeaderEndTag( );
		return code;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::OutputVertexMain( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = "";

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
			case ShaderPassType::Forward_StaticGeom:
			case ShaderPassType::StaticGeom:
			{
				// Evaluate world position offsets for world position offets
				NodeLink* link = const_cast<NodeLink*>( mMainSurfaceNode.GetLink( "WorldPositionOffset" ) );

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
						Enjon::String linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );
						Enjon::String evalType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );
						code += OutputTabbedLine( "vec3 worldPosition = ( uModel * vec4( aVertexPosition, 1.0 ) ).xyz;" );
						code += OutputTabbedLine( "worldPosition += " + ShaderGraph::TransformOutputType( linkEval, evalType, +"vec3" ) + ";" );
					}
					else
					{
						code += OutputErrorBlock( "INVALID: WorldPositionOffset: Link" + l->mConnectingNode->mName + ";" );
					}
				}
				// Otherwise, vertex position doesn't get changed
				else
				{
					code += OutputTabbedLine( "vec3 worldPosition = ( uModel * vec4( aVertexPosition, 1.0 ) ).xyz;" );
				}

				// Final position output 
				code += OutputTabbedLine( "gl_Position = uViewProjection * vec4( worldPosition, 1.0 );\n" );
 
				//code += OutputTabbedLine( "vec3 N = normalize( ( uModel * vec4( aVertexNormal, 0.0 ) ).xyz );" );
				//code += OutputTabbedLine( "vec3 T = normalize( ( uModel * vec4( aVertexTangent, 0.0 ) ).xyz );\n\n" );
				
				code += OutputTabbedLine( "vec3 N = normalize( mat3(uModel) * aVertexNormal );" );
				code += OutputTabbedLine( "vec3 T = normalize( mat3(uModel) * aVertexTangent );" );
				
				code += OutputTabbedLine( "// Reorthogonalize with respect to N" );
				code += OutputTabbedLine( "T = normalize( T - dot(T, N) * N );\n\n" );

				code += OutputTabbedLine( "// Calculate Bitangent" );
				code += OutputTabbedLine( "vec3 B = cross( N, T );\n" );

				code += OutputTabbedLine( "// TBN" );
				code += OutputTabbedLine( "mat3 TBN = mat3( T, B, N );\n\n" );

				code += OutputTabbedLine( "// TS_TBN" );
				//code += OutputTabbedLine( "vec3 TS_T = normalize(mat3(uModel) * aVertexTangent);" );
				//code += OutputTabbedLine( "vec3 TS_N = normalize(mat3(uModel) * aVertexNormal);" );
				//code += OutputTabbedLine( "vec3 TS_T = normalize(aVertexTangent * mat3(uModel));" );
				//code += OutputTabbedLine( "vec3 TS_N = normalize(aVertexNormal * mat3(uModel));" );
				//code += OutputTabbedLine( "vec3 TS_B = normalize(cross(TS_N, TS_T));" );
				//code += OutputTabbedLine( "mat3 TS_TBN = transpose(mat3( TS_T, TS_B, TS_N ));\n" );
				code += OutputTabbedLine( "mat3 TS_TBN = transpose( TBN );\n" );

				code += OutputTabbedLine( "// Output Vertex Data" );
				code += OutputTabbedLine( "vs_out.FragPositionWorldSpace = worldPosition;" );
				code += OutputTabbedLine( "vs_out.TexCoords = vec2( aVertexUV.x, -aVertexUV.y );" );
				//code += OutputTabbedLine( "vs_out.ViewPositionTangentSpace = uViewPositionWorldSpace * TS_TBN;" );
				//code += OutputTabbedLine( "vs_out.FragPositionTangentSpace = vs_out.FragPositionWorldSpace * TS_TBN;" );
				code += OutputTabbedLine( "vs_out.ViewPositionTangentSpace = TS_TBN * uViewPositionWorldSpace;" );
				code += OutputTabbedLine( "vs_out.FragPositionTangentSpace = TS_TBN * vs_out.FragPositionWorldSpace;" );

				code += OutputTabbedLine( "vs_out.TBN = TBN;" );
				code += OutputTabbedLine( "vs_out.TS_TBN = TS_TBN;" );

			} break;

			case ShaderPassType::InstancedGeom:
			{
				// Evaluate world position offsets for world position offets
				NodeLink* link = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "WorldPositionOffset" ) );

				// If link exists
				if ( link )
				{
					// Evaluate definitions for this node
					NodeLink* l = link;
					if ( l->mConnectingNode->IsDeclared( ) )
					{
						code += OutputTabbedLine( const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDefinition( ) );
					}

					if ( l->mFrom )
					{
						// Get code evaluation at link output
						Enjon::String linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );
						Enjon::String evalType = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );
						code += OutputTabbedLine( "vec3 worldPosition = ( aInstanceMatrix * vec4( aVertexPosition, 1.0 ) ).xyz;" );
						code += OutputTabbedLine( "worldPosition += " + ShaderGraph::TransformOutputType( linkEval, evalType, +"vec3" ) + ";" );
					}
					else
					{
						code += OutputErrorBlock( "INVALID: WorldPositionOffset: Link" + l->mConnectingNode->mName + ";" );
					}
				}
				// Otherwise, vertex position doesn't get changed
				else
				{
					code += OutputTabbedLine( "vec3 worldPosition = ( aInstanceMatrix * vec4( aVertexPosition, 1.0 ) ).xyz;" );
				}

				// Final position output 
				code += OutputTabbedLine( "gl_Position = uViewProjection * vec4( worldPosition, 1.0 );\n" );

				code += OutputTabbedLine( "// Reorthogonalize with respect to N" );
				code += OutputTabbedLine( "vec3 N = normalize( ( aInstanceMatrix * vec4( aVertexNormal, 0.0 ) ).xyz );" );
				code += OutputTabbedLine( "vec3 T = normalize( ( aInstanceMatrix * vec4( aVertexTangent, 0.0 ) ).xyz );\n" );

				code += OutputTabbedLine( "// Calculate Bitangent" );
				code += OutputTabbedLine( "vec3 B = cross( N, T );\n" );

				code += OutputTabbedLine( "// TBN" );
				code += OutputTabbedLine( "mat3 TBN = mat3( T, B, N );\n" );

				code += OutputTabbedLine( "// TS_TBN" );
				code += OutputTabbedLine( "vec3 TS_T = normalize(mat3(aInstanceMatrix) * aVertexTangent);" );
				code += OutputTabbedLine( "vec3 TS_N = normalize(mat3(aInstanceMatrix) * aVertexNormal);" );
				code += OutputTabbedLine( "vec3 TS_B = normalize(cross(TS_N, TS_T));" );
				code += OutputTabbedLine( "mat3 TS_TBN = transpose(mat3( TS_T, TS_B, TS_N ));\n" );

				code += OutputTabbedLine( "// Output Vertex Data" );
				code += OutputTabbedLine( "vs_out.FragPositionWorldSpace = worldPosition;" );
				code += OutputTabbedLine( "vs_out.TexCoords = vec2( aVertexUV.x, -aVertexUV.y );" );
				code += OutputTabbedLine( "vs_out.ViewPositionTangentSpace = uViewPositionWorldSpace * TS_TBN;" );
				code += OutputTabbedLine( "vs_out.FragPositionTangentSpace = vs_out.FragPositionWorldSpace * TS_TBN;" );
				code += OutputTabbedLine( "vs_out.TBN = TBN;" );

			} break;
		}

		return code;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::OutputFragmentHeader( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = "";

		// Get main node template
		ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast< ShaderGraphNodeTemplate* >( GetTemplate( "MainNodeTemplate" ) );

		// If not valid template
		if ( !surfaceNodeTemplate )
		{
			// Throw error 
			// Return empty code
			return code;
		}

		// Header tag
		code += OutputFragmentHeaderBeginTag( );

		// Version number
		code += OutputLine( "#version 330 core\n" );

		// FragData Layout
		code += OutputLine( "layout (location = 0) out vec4 AlbedoOut;" );
		code += OutputLine( "layout (location = 1) out vec4 NormalsOut;" );
		code += OutputLine( "layout (location = 2) out vec4 PositionOut;" );
		code += OutputLine( "layout (location = 3) out vec4 EmissiveOut;" );
		code += OutputLine( "layout (location = 4) out vec4 MatPropsOut;\n" );

		// Fragment Data In 
		code += OutputLine( "\nin VS_OUT" );
		code += OutputLine( "{" );
		code += OutputTabbedLine( "vec3 FragPositionWorldSpace;" );
		code += OutputTabbedLine( "vec2 TexCoords;" );
		code += OutputTabbedLine( "mat3 TBN;" );
		code += OutputTabbedLine( "mat3 TS_TBN;" );
		code += OutputTabbedLine( "vec3 ViewPositionTangentSpace;" );
		code += OutputTabbedLine( "vec3 FragPositionTangentSpace;" );
		code += OutputLine( "} fs_in;\n" );

		// Comment for declarations
		code += OutputLine( "// Global Uniforms" );
		code += OutputLine( "uniform float uWorldTime = 1.0f;" );
		code += OutputLine( "uniform vec3 uViewPositionWorldSpace;" );

		// Comment for declarations
		code += OutputLine( "\n// Variable Declarations" );

		// Output all variables to be used for vertex shader pass
		switch ( pass )
		{
			case ShaderPassType::Forward_StaticGeom:
			case ShaderPassType::InstancedGeom:
			case ShaderPassType::StaticGeom:
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

	Enjon::String ShaderGraph::BeginFragmentMain( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = OutputLine( "\n// Fragment Main" );
		code += OutputLine( "void main()" );
		code += OutputLine( "{" );
		return code;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::OutputFragmentMain( const ShaderPassType& pass, s32* status )
	{
		Enjon::String code = "";

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
			case ShaderPassType::InstancedGeom:
			case ShaderPassType::StaticGeom:
			{
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
						Enjon::String appendedCode = OutputLine( const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDefinition( ) );
						code += appendedCode;
					}

					// Get code evaluation at link output

					if ( l->mFrom )
					{
						Enjon::String linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

						// Get output type so we can transform correctly
						Enjon::String fromOutputType = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

						// Transform this type to required type for output ( could make this more specific to get away from branches )
						linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "vec4" );

						code += OutputTabbedLine( "AlbedoOut = " + linkEval + ";\n" );
					}
					else
					{
						code += OutputErrorBlock( "INVALID: BaseColor: Incorrect Link: " + l->mConnectingNode->mName + ";" );
					}

				}
				// Default base color
				else
				{
					Enjon::String baseColor = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "BaseColor" )->mDefaultValue;
					code += OutputTabbedLine( "AlbedoOut = " + baseColor + ";\n" );
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
						Enjon::String linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

						// Get output type so we can transform correctly
						Enjon::String fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

						// Transform this type to required type for output ( could make this more specific to get away from branches )
						linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "vec3" );

						code += OutputTabbedLine( "vec3 normal = normalize( " + linkEval + " * 2.0 - 1.0 );" );
						code += OutputTabbedLine( "normal = normalize( fs_in.TBN * normal );" );
						code += OutputTabbedLine( "NormalsOut = vec4( normal, 1.0 );\n" );
					}
					else
					{
						code += OutputErrorBlock( "INVALID: Normal: Incorrect Link: " + l->mConnectingNode->mName + ";" );
					}
				}
				// Default normals
				else
				{
					Enjon::String normal = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Normal" )->mDefaultValue;
					normal = ShaderGraph::FindReplaceAllMetaTag( normal, "#V_NORMAL", "fs_in.TBN[2]" );
					code += OutputTabbedLine( "NormalsOut = vec4( " + normal + ", 1.0 );\n" );
				}

				code += OutputTabbedLine( "// Material Properties" );
				Enjon::String metallic, roughness, ao;

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
						Enjon::String linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

						// Strip away global defines
						linkEval = ShaderGraph::ReplaceAllGlobalMetaTags( linkEval );

						// Get output type so we can transform correctly
						Enjon::String fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

						// Transform to float
						linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "float" );

						metallic = "clamp( " + linkEval + ", 0.0, 1.0 )";
					}
					else
					{
						code += OutputErrorBlock( "INVALID: Metallic: Incorrect Link: " + l->mConnectingNode->mName + ";" );
					}
				}
				// Default metallic
				else
				{
					metallic = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Metallic" )->mDefaultValue;
					metallic = "clamp( " + metallic + ", 0.0, 1.0 )";
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
						Enjon::String linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

						// Get output type so we can transform correctly
						Enjon::String fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

						// Transform to float
						linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "float" );

						roughness = "clamp( " + linkEval + ", 0.0, 1.0 )";
					}
					else
					{
						code += OutputErrorBlock( "INVALID: Metallic: Incorrect Link: " + l->mConnectingNode->mName + ";" );
					}
				}
				// Default roughness
				else
				{
					roughness = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Roughness" )->mDefaultValue;
					roughness = "clamp( " + roughness + ", 0.0, 1.0 )";
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
						Enjon::String linkEval = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

						// Get output type so we can transform correctly
						Enjon::String fromOutputType = const_cast<ShaderGraphNode*>( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

						// Transform to float
						linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "float" );

						ao = "clamp( " + linkEval + ", 0.0, 1.0 )";
					}
					else
					{
						code += OutputErrorBlock( "INVALID: AO: Incorrect Link: " + l->mConnectingNode->mName + ";" );
					}
				}
				// Default ao
				else
				{
					ao = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "AO" )->mDefaultValue;
					ao = "clamp( " + ao + ", 0.0, 1.0 )";
				}

				// Output Material Properties
				code += OutputTabbedLine( "MatPropsOut = vec4( " + metallic + ", " + roughness + ", " + ao + ", 1.0);\n" );

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
						Enjon::String linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

						// Get output type so we can transform correctly
						Enjon::String fromOutputType = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

						// Transform this type to required type for output ( could make this more specific to get away from branches )
						linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "vec4" );

						code += OutputTabbedLine( "EmissiveOut = " + linkEval + ";\n" );
					}
					else
					{
						code += OutputErrorBlock( "INVALID: Emissive: Incorrect Link: " + l->mConnectingNode->mName + ";" );
					}
				}
				// Default emissive
				else
				{
					Enjon::String emissive = const_cast<ShaderGraphNodeTemplate*> ( mMainSurfaceNode.mTemplate )->GetInput( "Emissive" )->mDefaultValue;
					code += OutputTabbedLine( "EmissiveOut = " + emissive + ";\n" );
				}

				// Other default code
				code += OutputTabbedLine( "PositionOut = vec4( fs_in.FragPositionWorldSpace, 1.0 );" );

			} break;

			case ShaderPassType::Forward_StaticGeom:
			{
				// Evaluate world position offsets for world position offets
				NodeLink* link = const_cast< NodeLink* >( mMainSurfaceNode.GetLink( "BaseColor" ) );

				code += OutputTabbedLine( "// Base Color" );
				// If link exists
				if ( link )
				{
					// Evaluate definitions for this node
					NodeLink* l = link;
					if ( !l->mConnectingNode->IsDefined( ) )
					{
						Enjon::String appendedCode = OutputLine( const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateVariableDefinition( ) );
						code += appendedCode;
					}

					// Get code evaluation at link output

					if ( l->mFrom )
					{
						Enjon::String linkEval = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputCodeAt( l->mFrom->mName );

						// Get output type so we can transform correctly
						Enjon::String fromOutputType = const_cast< ShaderGraphNode* >( l->mConnectingNode )->EvaluateOutputTypeAt( l->mFrom->mName );

						// Transform this type to required type for output ( could make this more specific to get away from branches )
						linkEval = ShaderGraph::TransformOutputType( linkEval, fromOutputType, "vec4" );

						code += OutputTabbedLine( "AlbedoOut = " + linkEval + ";\n" );
					}
					else
					{
						code += OutputErrorBlock( "INVALID: BaseColor: Incorrect Link: " + l->mConnectingNode->mName + ";" );
					}

				}
				// Default base color
				else
				{
					Enjon::String baseColor = const_cast< ShaderGraphNodeTemplate* > ( mMainSurfaceNode.mTemplate )->GetInput( "BaseColor" )->mDefaultValue;
					code += OutputTabbedLine( "AlbedoOut = " + baseColor + ";\n" );
				}
			} break;
		}

		return code;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::EndFragmentMain( const ShaderPassType& pass, s32* status )
	{
		// Need to iterate through nodes and make sure all set back to not being declared or defined
		for ( auto& n : mNodes )
		{
			n.second.SetDeclared( false );
			n.second.SetDefined( false );
		}

		Enjon::String code = "}\n";
		code += OutputFragmentHeaderEndTag( );

		return code;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::TransformOutputType( const Enjon::String& code, const Enjon::String& type, const Enjon::String& requiredType )
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

#define REPLACE_META_TAG( code, find, replace )\
for ( u32 i = 0; i < ShaderGraph::TagCount( code, find ); ++i )\
{\
	code = ShaderGraph::FindReplaceMetaTag( code, find, replace );\
}

	Enjon::String ShaderGraph::ReplaceAllGlobalMetaTags( const Enjon::String& code )
	{
		Enjon::String returnCode = code;

		// Replace V_TEXCOORD0 with appropriate tag
		REPLACE_META_TAG( returnCode, "#TEXCOORDS", "fs_in.TexCoords" )
		REPLACE_META_TAG( returnCode, "#G_WORLD_TIME", "uWorldTime" )
		REPLACE_META_TAG( returnCode, "#V_NORMAL", "aVertexNormal" )
		REPLACE_META_TAG( returnCode, "#V_TANGENT", "aVertexTangent" )
		REPLACE_META_TAG( returnCode, "#WORLD_SPACE_VIEW_POS", "uViewPositionWorldSpace" )
		REPLACE_META_TAG( returnCode, "#WORLD_SPACE_FRAG_POS", "fs_in.FragPositionWorldSpace" )
		REPLACE_META_TAG( returnCode, "#TANGENT_SPACE_VIEW_POS", "fs_in.ViewPositionTangentSpace" )
		REPLACE_META_TAG( returnCode, "#TANGENT_SPACE_FRAG_POS", "fs_in.FragPositionTangentSpace" )
		REPLACE_META_TAG( returnCode, "#WORLD_SPACE_NORMALS", "NormalsOut.xyz" )

		return returnCode;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::FindReplaceAllMetaTag( const Enjon::String& code, const Enjon::String& toFind, const Enjon::String& replaceWith )
	{
		Enjon::String retCode = code;

		// Search for begin
		std::size_t pos = retCode.find( toFind );
		while ( pos != Enjon::String::npos )
		{
			retCode = FindReplaceMetaTag( retCode, toFind, replaceWith );
			pos = retCode.find( toFind, pos + 1 );
		}

		return retCode;
	}

	//==================================================================================================================

	Enjon::String ShaderGraph::FindReplaceMetaTag( const Enjon::String& code, const Enjon::String& toFind, const Enjon::String& replaceWith )
	{
		Enjon::String returnStr = "";

		// Search for begin
		std::size_t foundBegin = code.find( toFind );
		std::size_t findSize = toFind.length( );

		Enjon::String subStrBefore = "";
		Enjon::String subStrAfter = "";

		// If found, then replace and return
		if ( foundBegin != Enjon::String::npos )
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

	Enjon::String ShaderGraphNode::BuildInputKeyFromLinks( )
	{
		Enjon::String inputKey = "";
		for ( auto& l : mLinks )
		{
			Enjon::String fromName = l.mFrom->mName;
			Enjon::String outputType = const_cast<ShaderGraphNode*> ( l.mConnectingNode )->EvaluateOutputTypeAt( l.mFrom->mName );
			Enjon::String input = ShaderGraph::FindReplaceAllMetaTag( l.mFrom->mPrimitiveType, "#OUTPUT_TYPE", const_cast<ShaderGraphNode*>( l.mConnectingNode )->EvaluateOutputTypeAt( l.mFrom->mName ) );
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

	Enjon::String ShaderGraphNode::EvaluateOutputTypeAt( const Enjon::String& outputName )
	{
		Enjon::String returnOutputType = "";

		// Handle differently, of course
		if ( mTemplate->IsFunction( ) )
		{
			// Build input key from links ( could be empty )
			Enjon::String inputKey = BuildInputKeyFromLinks( );

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

	Enjon::String ShaderGraphNode::EvaluateOutputCodeAt( const Enjon::String& name )
	{
		Enjon::String code = "";

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

	Enjon::String ShaderGraphNode::EvaluateVariableDefinition( )
	{
		Enjon::String finalOutput = "";

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
		if ( !IsUniform( ) || mTemplate->GetUniformType( ) == UniformType::TextureSampler2D )
		{
			// Get variable decl template code
			Enjon::String variableDefTemplate = mTemplate->GetVariableDefinitionTemplate( );

			// Replace with output from links if available, otherwise we'll resort to defaults
			for ( auto& l : mLinks )
			{
				if ( l.mFrom && l.mTo )
				{
					Enjon::String codeEval = const_cast< ShaderGraphNode* > ( l.mConnectingNode )->EvaluateOutputCodeAt( l.mFrom->mName );

					// Get input name from link
					Enjon::String inputName = "#INPUT(" + l.mTo->mName + ")";

					// Replace this input name in definition code with code evaluation
					variableDefTemplate = ShaderGraph::FindReplaceAllMetaTag( variableDefTemplate, inputName, codeEval );
				}
				else
				{
					variableDefTemplate = OutputErrorBlock( "INVALID: Node: " + mName + ": Incorrect output channel from input node: " + l.mConnectingNode->mName );
				}
			}

			// Get inputs from template and search for remaining areas to replace
			const std::unordered_map< Enjon::String, InputConnection >& inpts = mTemplate->GetInputs( );
			for ( auto& i : inpts )
			{
				Enjon::String inputName = "#INPUT(" + i.second.mName + ")";
				Enjon::String defaultValue = i.second.mDefaultValue;

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

	Enjon::String ShaderGraphNode::EvaluateVariableDeclaration( )
	{
		// Final output
		Enjon::String finalOutput = "";

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
		Enjon::String variableDeclTemplate = OutputLine( IsUniform( ) ? "uniform " + mTemplate->GetVariableDeclarationTemplate( ) : mTemplate->GetVariableDeclarationTemplate( ) );

		// Need to find and replace for certain things
		//variableDeclTemplate = ( ShaderGraph::FindReplaceMetaTag( variableDeclTemplate, "#NODE_NAME", IsUniform( ) ? GetUniformName( ) : mName ) );
		variableDeclTemplate = ( ShaderGraph::FindReplaceMetaTag( variableDeclTemplate, "#NODE_NAME", mName ) );

		// Replace uniform location tag
		variableDeclTemplate = ( ShaderGraph::FindReplaceMetaTag( variableDeclTemplate, "#UNIFORM_LOCATION", std::to_string( mUniformLocation ) ) );

		// Evalute output type if uniform or function
		if ( mTemplate->IsFunction( ) )
		{
			Enjon::String outputTypeString = EvaluateOutputTypeAt( "Value" );

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

		// Set variable to being declared
		SetDeclared( true );

		// Final output return
		return ( finalOutput + variableDeclTemplate );
	}
	//========================================================================================================================= 

	u32 ShaderGraph::TagCount( const Enjon::String& code, const Enjon::String& tag )
	{
		u32 count = 0;

		// Search for begin
		std::size_t pos = code.find( tag );
		while ( pos != Enjon::String::npos )
		{
			count++;
			pos = code.find( tag, pos + 1 );
		}

		return count;
	}

	//========================================================================================================================= 

	bool ShaderGraph::HasTag( const Enjon::String& code, const Enjon::String& tag )
	{
		// Search for begin
		std::size_t foundBegin = code.find( tag );

		// If found, then replace and return
		if ( foundBegin != Enjon::String::npos )
		{
			return true;
		}

		return false;
	}

	//========================================================================================================================= 

	Enjon::String ShaderGraph::ReplaceTypeWithAppropriateUniformType( const Enjon::String& code )
	{
		Enjon::String returnCode = "";

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

	Enjon::String ShaderGraph::GetCode( ShaderPassType type, ShaderType shaderType )
	{
		auto query = mShaderPassCode.find( type );
		if ( query != mShaderPassCode.end( ) )
		{
			switch ( shaderType )
			{
			case ShaderType::Vertex:
			{
				// Get pass code
				Enjon::String passCode = mShaderPassCode[ type ];

				// Parse and return vertex shader code 
				passCode = ParseFromTo( OutputVertexHeaderBeginTag( ), OutputVertexHeaderEndTag( ), passCode );
				passCode = ShaderGraph::FindReplaceMetaTag( passCode, OutputVertexHeaderBeginTag( ), "" );

				// Return
				return passCode;

			} break;

			case ShaderType::Fragment:
			{
				// Get pass code
				Enjon::String passCode = mShaderPassCode[ type ];

				// Parse and return fragment shader code 
				passCode = ParseFromTo( OutputFragmentHeaderBeginTag( ), OutputFragmentHeaderEndTag( ), passCode );
				passCode = ShaderGraph::FindReplaceMetaTag( passCode, OutputFragmentHeaderBeginTag( ), "" );

				// Return
				return passCode;

			} break;

			case ShaderType::Compute:
			{
				// Get pass code
				Enjon::String passCode = mShaderPassCode[ type ];

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

	void ShaderGraph::WriteToFile( ShaderPassType pass )
	{
		Enjon::String shaderPath = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( )->GetAssetsPath( ) + "/Shaders";
		Enjon::String fragName = mName + "." + ShaderPassToString( pass ) + ".Fragment.glsl";
		Enjon::String vertName = mName + "." + ShaderPassToString( pass ) + ".Vertex.glsl";

		std::ofstream file;
		file.open( ( shaderPath + "/" + fragName ).c_str( ) );
		if ( file )
		{
			Enjon::String code = GetCode( pass, ShaderType::Fragment );
			file.write( code.c_str( ), code.length( ) );
		}
		file.close( );

		file.open( ( shaderPath + "/" + vertName ).c_str( ) );
		if ( file )
		{
			Enjon::String code = GetCode( pass, ShaderType::Vertex );
			file.write( code.c_str( ), code.length( ) );
		}
		file.close( );
	}

	//========================================================================================================================= 

	s32 ShaderGraph::Compile( )
	{
		s32 status = 1;

		if ( !mMainSurfaceNode.mTemplate )
		{
			mMainSurfaceNode.mTemplate = const_cast<ShaderGraphNodeTemplate*>( GetTemplate( "MainNodeTemplate" ) );
		}

		// For each pass, output shader code based on graph
		for ( u32 i = 0; i < ( u32 )ShaderPassType::Count; ++i )
		{
			Enjon::String code = "";

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

			if ( status > 0 )
			{
				mShaderPassCode[ ShaderPassType( i ) ] = code;
			}

			// Write shaders to file ( for now )
			WriteToFile( ShaderPassType( i ) ); 

			// Create shader TODO(): Need a way to detect that this created failed
			Enjon::Shader* shader = new Shader( AssetHandle<ShaderGraph>(this), ShaderPassType( i ) );

			// Add shader
			mShaders[ ShaderPassType( i ) ] = shader;
		}

		// Get main node template
		ShaderGraphNodeTemplate* surfaceNodeTemplate = const_cast<ShaderGraphNodeTemplate*>( GetTemplate( "MainNodeTemplate" ) );

		// If not valid template
		if ( !surfaceNodeTemplate )
		{
			// Throw error
			// Shader not valid
			status = 0;
		}

		// Loop through nodes and build uniforms
		for ( auto& l : mMainSurfaceNode.mLinks )
		{
			ConstructUniforms( l );
		}

		return status;
	}

}







