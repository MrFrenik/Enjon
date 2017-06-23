#pragma once
#ifndef ENJON_SHADER_GRAPH_H
#define ENJON_SHADER_GRAPH_H 

#include "System/Types.h"
#include "Defines.h"

#include <unordered_map>

#include <rapidjson/prettywriter.h>
#include <rapidjson/document.h> 

namespace Enjon
{ 
	class ShaderUniform;
	typedef std::unordered_map< Enjon::String, Enjon::ShaderUniform* > UniformMap;
	
	enum class UniformType
	{
		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
		TextureSampler2D,
		Invalid
	};
	
	enum class ShaderType
	{
		Fragment,
		Vertex,
		Compute,
		Unknown
	}; 

	enum class ShaderPrimitiveType : u32
	{
		Float,
		Int,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
		TextureSampler2D
	};

	enum class ShaderGraphNodeType
	{
		Function,
		Component,
		Variable
	};

	class InputConnection
	{
	public:
		std::string mName;
		std::string mPrimitiveType;
		std::string mDefaultValue;
	};

	class OutputConnection
	{
	public:
		std::string mName;
		std::string mPrimitiveType;
		std::string mCodeTemplate;
	};

	struct ParameterLayout
	{
		std::vector< std::string > mInputParameters;
		std::string mOutputParameter;
	};

	class ShaderGraph;
	class ShaderGraphNodeTemplate
	{
		friend ShaderGraph;

	public:
		ShaderGraphNodeTemplate( )
		{
		}

		~ShaderGraphNodeTemplate( )
		{
		}

		bool IsFunction( ) const;

		const InputConnection* GetInput( const std::string& inputName )
		{
			auto query = mInputs.find( inputName );
			if ( query != mInputs.end( ) )
			{
				return &mInputs[ inputName ];
			}

			return nullptr;
		}

		const OutputConnection* GetOutput( const std::string& outputName )
		{
			auto query = mOutputs.find( outputName );
			if ( query != mOutputs.end( ) )
			{
				return &mOutputs[ outputName ];
			}

			return nullptr;
		}

		std::string GetName( ) { return mName; }

		const std::unordered_map< std::string, InputConnection >& GetInputs( ) const { return mInputs; }

		const ParameterLayout* GetLayout( const std::string& inputKey );

		std::string GetVariableDeclarationTemplate( ) const { return mVariableDeclaration; }
		std::string GetVariableDefinitionTemplate( ) const { return mVariableDefinition; }

		UniformType GetUniformType( ) const { return mUniformType; }

	protected:
		void AddParameterLayout( const ParameterLayout& layout );

	protected:
		std::string mName;
		std::string mVariableDeclaration;
		std::string mVariableDefinition;
		u32 mNumberInputs;
		u32 mNumberOutputs;
		UniformType mUniformType = UniformType::Invalid;
		std::unordered_map< std::string, InputConnection > mInputs;
		std::unordered_map< std::string, OutputConnection > mOutputs;
		std::unordered_map< std::string, ParameterLayout > mParameterLayouts;
		ShaderGraphNodeType mType;
	};

	class ShaderGraphNode;
	struct NodeLink
	{
		const ShaderGraphNode* mConnectingNode = nullptr;
		const InputConnection* mTo = nullptr;
		const OutputConnection* mFrom = nullptr;
	};

	class ShaderGraphNode
	{
		friend ShaderGraph;

	public:

		ShaderGraphNode( )
		{
		}

		ShaderGraphNode( const ShaderGraphNodeTemplate* templateNode, const std::string& name )
			: mName( name ), mTemplate( templateNode )
		{
		}

		~ShaderGraphNode( )
		{
		}


		void Serialize( rapidjson::PrettyWriter< rapidjson::StringBuffer >& writer );

		s32 Deserialize( rapidjson::PrettyWriter< rapidjson::StringBuffer >& reader );

		void AddLink( const NodeLink& link );

		const NodeLink* GetLink( const std::string inputName );

		std::string EvaluateOutputType( );

		std::string EvaluateVariableDeclaration( );
		std::string EvaluateVariableDefinition( );

		bool IsUniform( ) const { return mIsUniform; }

		bool IsDeclared( ) const { return mIsVariableDeclared; }

		bool IsDefined( ) const { return mIsVariableDefined; }

		void SetDeclared( bool declared ) { mIsVariableDeclared = declared; }

		void SetDefined( bool defined ) { mIsVariableDefined = defined; }

		bool HasOverride( const std::string& inputName );

		void AddOverride( const InputConnection& connection );

		const InputConnection* GetOverride( const std::string& inputName );

		std::string BuildInputKeyFromLinks( );

		std::string EvaluateOutputCodeAt( const std::string& name );

		std::string EvaluateOutputTypeAt( const std::string& outputName );

		std::string GetUniformName( ) { return mName + "_uniform"; }

		void Clear( );

		std::string mName = "INVALID";
		bool mIsUniform = false;
		bool mIsVariableDeclared = false;
		bool mIsVariableDefined = false;
		u32 mUniformLocation = 0;
		std::unordered_map< std::string, InputConnection > mDefaultOverrides;
		const ShaderGraphNodeTemplate* mTemplate = nullptr;
		std::vector< NodeLink > mLinks;
	};

	enum class ShaderPassType
	{
		Surface_StaticGeom,
		Count
	};

	class ShaderGraph
	{
	public:

		ShaderGraph( );

		~ShaderGraph( );

		void Validate( );

		s32 Compile( );

		s32 Serialize( rapidjson::PrettyWriter< rapidjson::StringBuffer >& writer );

		s32 Deserialize( const std::string& filePath );

		static s32 DeserializeTemplate( const std::string& filePath );

		static const ShaderGraphNodeTemplate* GetTemplate( const std::string& name );

		static const std::unordered_map< std::string, ShaderGraphNodeTemplate >* GetTemplates( ) { return &mTemplates; }

		static std::string FindReplaceMetaTag( const std::string& code, const std::string& toFind, const std::string& replaceWith );

		static std::string ReplaceAllGlobalMetaTags( const std::string& code );

		static std::string ReplaceTypeWithAppropriateUniformType( const std::string& code );

		static std::string FindReplaceAllMetaTag( const std::string& code, const std::string& toFind, const std::string& replaceWith );

		static bool HasTag( const std::string& code, const std::string& tag );

		static u32 TagCount( const std::string& code, const std::string& tag );

		static std::string ShaderGraph::TransformOutputType( const std::string& code, const std::string& type, const std::string& requiredType );

		const ShaderGraphNode* GetNode( const std::string& nodeName );

		std::string GetCode( ShaderPassType type, ShaderType shaderType = ShaderType::Unknown );

		static std::string ShaderPassToString( ShaderPassType type );

		//Shader* ShaderGraph::CreateShader( ShaderPassType type );

		//void AddUniformsToShader( const NodeLink& link, Shader* shader );

	private:
		static rapidjson::Document GetJSONDocumentFromFilePath( const std::string& filePath, s32* status );

		void ShaderGraph::ClearGraph( );

	private:
		std::string OutputPassTypeMetaData( const ShaderPassType& pass, s32* status );
		std::string OutputVertexHeaderBeginTag( );
		std::string OutputVertexHeaderEndTag( );
		std::string OutputVertexHeader( const ShaderPassType& pass, s32* status );
		std::string OutputVertexIncludes( const ShaderPassType& pass, s32* status );
		std::string BeginVertexMain( const ShaderPassType& pass, s32* status );
		std::string OutputVertexMain( const ShaderPassType& pass, s32* status );
		std::string EndVertexMain( const ShaderPassType& pass, s32* status );
		std::string OutputFragmentHeaderBeginTag( );
		std::string OutputFragmentHeaderEndTag( );
		std::string OutputFragmentHeader( const ShaderPassType& pass, s32* status );
		std::string OutputFragmentIncludes( const ShaderPassType& pass, s32* status );
		std::string BeginFragmentMain( const ShaderPassType& pass, s32* status );
		std::string OutputFragmentMain( const ShaderPassType& pass, s32* status );
		std::string EndFragmentMain( const ShaderPassType& pass, s32* status );

	private:
		void AddNode( const ShaderGraphNode& node );
		void AddConstant( const ShaderGraphNode& node );
		void AddUniform( const ShaderGraphNode& node );
		void AddFunction( const ShaderGraphNode& node );

	protected:
		std::string mName;
		std::unordered_map< std::string, ShaderGraphNode > mNodes;
		static std::unordered_map< std::string, ShaderGraphNodeTemplate > mTemplates;
		std::unordered_map< ShaderPassType, u32 > mShaderProgramHandles; 
		std::unordered_map< ShaderPassType, std::string > mShaderPassCode;
		std::unordered_map< ShaderPassType, UniformMap > mShaderUniforms;
		ShaderGraphNode mMainSurfaceNode;
		u32 mTextureSamplerLocation = 0;
	}; 
}

#endif




























