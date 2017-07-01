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
	class Shader;
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
		Enjon::String mName;
		Enjon::String mPrimitiveType;
		Enjon::String mDefaultValue;
	};

	class OutputConnection
	{
	public:
		Enjon::String mName;
		Enjon::String mPrimitiveType;
		Enjon::String mCodeTemplate;
	};

	struct ParameterLayout
	{
		std::vector< Enjon::String > mInputParameters;
		Enjon::String mOutputParameter;
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

		const InputConnection* GetInput( const Enjon::String& inputName )
		{
			auto query = mInputs.find( inputName );
			if ( query != mInputs.end( ) )
			{
				return &mInputs[ inputName ];
			}

			return nullptr;
		}

		const OutputConnection* GetOutput( const Enjon::String& outputName )
		{
			auto query = mOutputs.find( outputName );
			if ( query != mOutputs.end( ) )
			{
				return &mOutputs[ outputName ];
			}

			return nullptr;
		}

		Enjon::String GetName( ) { return mName; }

		const std::unordered_map< Enjon::String, InputConnection >& GetInputs( ) const { return mInputs; }

		const ParameterLayout* GetLayout( const Enjon::String& inputKey );

		Enjon::String GetVariableDeclarationTemplate( ) const { return mVariableDeclaration; }
		Enjon::String GetVariableDefinitionTemplate( ) const { return mVariableDefinition; }

		UniformType GetUniformType( ) const { return mUniformType; }

	protected:
		void AddParameterLayout( const ParameterLayout& layout );

	protected:
		Enjon::String mName;
		Enjon::String mVariableDeclaration;
		Enjon::String mVariableDefinition;
		u32 mNumberInputs;
		u32 mNumberOutputs;
		UniformType mUniformType = UniformType::Invalid;
		std::unordered_map< Enjon::String, InputConnection > mInputs;
		std::unordered_map< Enjon::String, OutputConnection > mOutputs;
		std::unordered_map< Enjon::String, ParameterLayout > mParameterLayouts;
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

		ShaderGraphNode( const ShaderGraphNodeTemplate* templateNode, const Enjon::String& name )
			: mName( name ), mTemplate( templateNode )
		{
		}

		~ShaderGraphNode( )
		{
		}


		void Serialize( rapidjson::PrettyWriter< rapidjson::StringBuffer >& writer );

		s32 Deserialize( rapidjson::PrettyWriter< rapidjson::StringBuffer >& reader );

		void AddLink( const NodeLink& link );

		const NodeLink* GetLink( const Enjon::String inputName );

		Enjon::String EvaluateOutputType( );

		Enjon::String EvaluateVariableDeclaration( );
		Enjon::String EvaluateVariableDefinition( );

		bool IsUniform( ) const { return mIsUniform; }

		bool IsDeclared( ) const { return mIsVariableDeclared; }

		bool IsDefined( ) const { return mIsVariableDefined; }

		void SetDeclared( bool declared ) { mIsVariableDeclared = declared; }

		void SetDefined( bool defined ) { mIsVariableDefined = defined; }

		bool HasOverride( const Enjon::String& inputName );

		void AddOverride( const InputConnection& connection );

		const InputConnection* GetOverride( const Enjon::String& inputName );

		Enjon::String BuildInputKeyFromLinks( );

		Enjon::String EvaluateOutputCodeAt( const Enjon::String& name );

		Enjon::String EvaluateOutputTypeAt( const Enjon::String& outputName );

		Enjon::String GetUniformName( ) { return mName + "_uniform"; }

		void Clear( );

		Enjon::String mName = "INVALID";
		bool mIsUniform = false;
		bool mIsVariableDeclared = false;
		bool mIsVariableDefined = false;
		u32 mUniformLocation = 0;
		std::unordered_map< Enjon::String, InputConnection > mDefaultOverrides;
		const ShaderGraphNodeTemplate* mTemplate = nullptr;
		std::vector< NodeLink > mLinks;
	};

	enum class ShaderPassType
	{
		StaticGeom,
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

			s32 Create( const Enjon::String& filePath );

			Enjon::String GetName( ) const { return mName; }

			static s32 DeserializeTemplate( const Enjon::String& filePath );

			static const ShaderGraphNodeTemplate* GetTemplate( const Enjon::String& name );

			static const std::unordered_map< Enjon::String, ShaderGraphNodeTemplate >* GetTemplates( ) { return &mTemplates; }

			static Enjon::String FindReplaceMetaTag( const Enjon::String& code, const Enjon::String& toFind, const Enjon::String& replaceWith );

			static Enjon::String ReplaceAllGlobalMetaTags( const Enjon::String& code );

			static Enjon::String ReplaceTypeWithAppropriateUniformType( const Enjon::String& code );

			static Enjon::String FindReplaceAllMetaTag( const Enjon::String& code, const Enjon::String& toFind, const Enjon::String& replaceWith );

			static bool HasTag( const Enjon::String& code, const Enjon::String& tag );

			static u32 TagCount( const Enjon::String& code, const Enjon::String& tag );

			static Enjon::String ShaderGraph::TransformOutputType( const Enjon::String& code, const Enjon::String& type, const Enjon::String& requiredType );

			const ShaderGraphNode* GetNode( const Enjon::String& nodeName );

			Enjon::String GetCode( ShaderPassType type, ShaderType shaderType = ShaderType::Unknown );

			static Enjon::String ShaderPassToString( ShaderPassType type ); 
 
			bool HasUniform( const Enjon::String& uniformName );

			const ShaderUniform* GetUniform( const Enjon::String& uniformName );

			const std::unordered_map< Enjon::String, ShaderUniform* >* GetUniforms( );

			bool HasShader( ShaderPassType pass );

			const Shader* GetShader( ShaderPassType pass );
	
			void WriteToFile( ShaderPassType pass );

		private:
			static rapidjson::Document GetJSONDocumentFromFilePath( const Enjon::String& filePath, s32* status );

			void ShaderGraph::ClearGraph( );

		private:
			void ConstructUniforms( const NodeLink& link );
			bool AddUniform( ShaderUniform* uniform );

			Enjon::String OutputPassTypeMetaData( const ShaderPassType& pass, s32* status );
			Enjon::String OutputVertexHeaderBeginTag( );
			Enjon::String OutputVertexHeaderEndTag( );
			Enjon::String OutputVertexHeader( const ShaderPassType& pass, s32* status );
			Enjon::String OutputVertexIncludes( const ShaderPassType& pass, s32* status );
			Enjon::String BeginVertexMain( const ShaderPassType& pass, s32* status );
			Enjon::String OutputVertexMain( const ShaderPassType& pass, s32* status );
			Enjon::String EndVertexMain( const ShaderPassType& pass, s32* status );
			Enjon::String OutputFragmentHeaderBeginTag( );
			Enjon::String OutputFragmentHeaderEndTag( );
			Enjon::String OutputFragmentHeader( const ShaderPassType& pass, s32* status );
			Enjon::String OutputFragmentIncludes( const ShaderPassType& pass, s32* status );
			Enjon::String BeginFragmentMain( const ShaderPassType& pass, s32* status );
			Enjon::String OutputFragmentMain( const ShaderPassType& pass, s32* status );
			Enjon::String EndFragmentMain( const ShaderPassType& pass, s32* status );

		private:
			void AddNode( const ShaderGraphNode& node );

		protected:
			static std::unordered_map< Enjon::String, ShaderGraphNodeTemplate > mTemplates;

			Enjon::String mName;
			std::unordered_map< Enjon::String, ShaderGraphNode > mNodes;
			std::unordered_map< ShaderPassType, Shader* > mShaders;
			std::unordered_map< Enjon::String, ShaderUniform* > mUniforms;
			std::unordered_map< ShaderPassType, Enjon::String > mShaderPassCode;
			ShaderGraphNode mMainSurfaceNode;
			u32 mTextureSamplerLocation = 0;
	}; 
}

#endif




























