// @file ShaderGraph.h
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#pragma once
#ifndef ENJON_SHADER_GRAPH_H
#define ENJON_SHADER_GRAPH_H 

#include "Graphics/Shader.h" 

#include <unordered_map>
#include <set>
#include <vector>

namespace Enjon
{ 
	// These do not necessary need to evaluate to executable code, 
	// they just need to evaluate to glsl

	enum class ShaderGraphNodeVariableType
	{
		LocalVariable,
		UniformVariable
	};

	enum class ShaderPrimitiveType
	{
		Float,
		Vec3,
		Vec2,
		Vec4,
		Texture2D,
		Mat4,
		Integer
	};

	enum class ShaderNodeState
	{
		Ready,
		Evaulated
	};

	class ShaderGraphNode
	{
		class Connection
		{
			public: 
			/**
			* @brief Constructor
			*/
			Connection( );
			
			/**
			* @brief Constructor
			*/
			Connection( const Enjon::String& name );

			/**
			* @brief Destructor
			*/
			~Connection( );

			/**
			* @brief
			*/
			Enjon::String EvaluateToGLSL( );

			/*
			* @brief
			*/
			void ConnectWith( const Connection* connection );

			/*
				ShaderMutiplyNode
				{
					ShaderMutiplyNode( const Enjon::String& id )
						: mID( id )
					{
						mOutput = Connection( "Output" );
						mInputA = Connection( "Input A" );
						mInputB = Connection( "Input B" );
					}

					private:

						// Output
						Connection mOutput;

						// Inputs
						Connection mInputA;
						Connection mInputB; 
				};

				ShaderFloatNode
				{
					ShaderFloatNode( const Enjon::String& id )
						: mID( id )
					{
						mOutput = Connection( "Value" );
					}

					private:
					Connection mOutput;
				}
				
				ShaderTexture2DNode
				{
					ShaderTexture2DNode( const Enjon::String& id )
						: mID( id )
					{
						mRGBOutput = Connection( "RGB" ); 
						mROutput = Connection( "R" );
						mGOutput = Connection( "G" );
						mBOutput = Connection( "B" );
						mAOutput = Connection( "A" );
						mUVInput = Connection( "UV" );
					}

					private:
					Connection mRGBOutput;
					Connection mROutput;
					Connection mGOutput;
					Connection mBOutput;
					Connection mAOutput;

					Connection mUVInput;
				};


				ShaderMultiplyNode mult1( "mult1 );
				ShaderTexture2DNode tex1( "tex2" );
				ShaderFloatNOde fl1( "fl1" );

				mult1.mInputA.ConnectWith( &tex1.mROutput );
				mult1.mInputB.ConnectWith( &fl1.mOutput ); 
			*/

			private:
				Enjon::String mConnectionID;
				const Connection* mConnection;
		};

		public:
			/**
			* @brief Constructor
			*/
			ShaderGraphNode( const Enjon::String& id )
				: mID( id )
			{
			}

			/**
			* @brief Destructor
			*/
			~ShaderGraphNode( ) {}

			/**
			* @brief Destructor
			*/
			void Execute( )
			{ 
			}
			
			template <typename T>
			T* Cast( )
			{
				return static_cast<T*>( this );
			} 

			/**
			* @brief
			*/
			Enjon::String Evaulate( )
			{
				if ( mState == ShaderNodeState::Ready )
				{
					mState = ShaderNodeState::Evaulated;
					return EvaluateToGLSL( ); 
				}

				return "";
			}

			/**
			* @brief
			*/
			virtual Enjon::String EvaluateToGLSL( )
			{
				return "";
			}

			/**
			* @brief
			*/
			virtual Enjon::String GetDeclaration( )
			{
				return "";
			}

			/**
			* @brief
			*/
			Enjon::String GetID( ) const { return mID; }
			
			/**
			* @brief
			*/
			virtual Enjon::String GetQualifiedID( ) { return mID; }

			/**
			* @brief
			*/
			const std::vector< ShaderGraphNode* >* GetInputs( ) const { return &mInputs; }

			/**
			* @brief
			*/
			ShaderPrimitiveType GetPrimitiveType( ) const { return mPrimitiveType; }
			
			/**
			* @brief
			*/
			ShaderNodeState GetState( ) const { return mState; }
			
			/**
			* @brief
			*/
			virtual Enjon::String GetDefinition( )
			{
				return "";
			}

			/**
			* @brief
			*/
			void AddInput( ShaderGraphNode* node )
			{
				auto query = std::find( mInputs.begin( ), mInputs.end( ), node );
				if ( query == mInputs.end( ) && mInputs.size( ) < mMaxNumberInputs )
				{
					mInputs.push_back( node );
				}
			}

			/**
			* @brief
			*/
			void RemoveInput( ShaderGraphNode* node );

		protected:
			ShaderGraphNodeVariableType mVariableType = ShaderGraphNodeVariableType::LocalVariable;
			ShaderPrimitiveType mPrimitiveType;
			ShaderNodeState mState = ShaderNodeState::Ready;
			u32 mMaxNumberInputs = 1;
			std::vector< ShaderGraphNode* > mInputs;
			Enjon::String mID;
	};

	enum class ShaderGraphMainNodeInputType
	{ 
		Albedo,
		Normal,
		Metallic,
		Roughness,
		Emissive,
		AmbientOcculsion,
		VertexPositionOffset,
		Count
	};

	// Main entry point for shader graph
	class ShaderGraphMainNode : public ShaderGraphNode
	{
		public:

			/**
			* @brief
			*/
			ShaderGraphMainNode( )
				: ShaderGraphNode( "MainNode" )
			{ 
				// Max number of inputs allowed for main node 
				mMaxNumberInputs = u32( ShaderGraphMainNodeInputType::Count );

				// Resize inputs to fit max
				mInputs.resize( mMaxNumberInputs );
			}
			
			/**
			* @brief
			*/
			~ShaderGraphMainNode( )
			{ 
			}

		private:
	};

	class ShaderGraph
	{
		public:
			/**
			* @brief Constructor
			*/
			ShaderGraph( );

			/**
			* @brief Destructor
			*/
			~ShaderGraph( ); 

			/**
			* @brief
			*/
			ShaderGraphNode* AddNode( ShaderGraphNode* node );

			/**
			* @brief Removes node from graph and frees memory of node
			*/
			void RemoveNode( ShaderGraphNode* node );

			/**
			* @brief 
			*/
			void Reset( );

			/**
			* @brief
			*/
			void Compile( );
			
			/**
			* @brief
			*/
			bool VariableExists( const Enjon::String& var );
			
			/**
			* @brief
			*/
			void RegisterVariable( const Enjon::String& var );

			/**
			* @brief
			*/
			void RegisterRequiredDefinitions( ShaderGraphNode* node );
	
			/**
			* @brief
			*/
			void UnregisterVariable( const Enjon::String& var );

		private:
			/**
			* @brief
			*/
			void RecurseThroughChildrenAndBuildVariables( ShaderGraphNode* node );

		private:
			ShaderGraphMainNode mMainNode;
			std::set< ShaderGraphNode* > mNodes;
			std::set< Enjon::String > mRegisteredVariables;
			std::set< ShaderGraphNode* > mDefinesOnStart;
	};

	class ShaderGraphFunctionNode : public ShaderGraphNode
	{
	public:
		/**
		* @brief Constructor
		*/
		ShaderGraphFunctionNode( const Enjon::String& id ) 
			: ShaderGraphNode( id )
		{
			mPrimitiveType = ShaderPrimitiveType::Float;
		}

		/**
		* @brief Destructor
		*/
		~ShaderGraphFunctionNode( ) {}

	protected:
	};

	class BinaryFunctionNode : public ShaderGraphFunctionNode
	{
	public:

		BinaryFunctionNode( const Enjon::String& id )
			: ShaderGraphFunctionNode( id )
		{
			mMaxNumberInputs = 2;
		}

		~BinaryFunctionNode( )
		{

		}

	protected:
	};

	class UnaryFunctionNode : public ShaderGraphFunctionNode
	{
	public:

		UnaryFunctionNode( const Enjon::String& id )
			: ShaderGraphFunctionNode( id )
		{
			mMaxNumberInputs = 1;
		}

		~UnaryFunctionNode( )
		{

		}

	protected:
	};

	template < typename T >
	class ShaderPrimitiveNode : public ShaderGraphNode
	{
	public:
		ShaderPrimitiveNode( const Enjon::String& id )
			: ShaderGraphNode( id )
		{
			mVariableType = ShaderGraphNodeVariableType::LocalVariable;
		}

		~ShaderPrimitiveNode( )
		{

		}

	protected:
		T mData;
	};

	class ShaderFloatNode : public ShaderPrimitiveNode< f32 >
	{
	public:
		ShaderFloatNode( const Enjon::String& id )
			: ShaderPrimitiveNode( id )
		{
			mPrimitiveType = ShaderPrimitiveType::Float;
			mMaxNumberInputs = 1;
		}

		ShaderFloatNode( const Enjon::String& id, f32 value )
			: ShaderPrimitiveNode( id )
		{
			mData = value;
			mMaxNumberInputs = 1;
		}

		~ShaderFloatNode( ) { }

		Enjon::String EvaluateToGLSL( ) override
		{
			switch ( mVariableType )
			{
				case ShaderGraphNodeVariableType::LocalVariable: 
				{
					return GetQualifiedID( ) + " = " + std::to_string( mData ) + ";";
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
			
		virtual Enjon::String GetDeclaration( ) override
		{
			return ( "uniform float " + mID + ";" );
		}

		protected:
	};

	class ShaderVec4Node : public ShaderPrimitiveNode< Vec4 >
	{
		public:
			ShaderVec4Node( const Enjon::String& id )
				: ShaderPrimitiveNode( id )
			{
				mPrimitiveType = ShaderPrimitiveType::Vec4;
				mMaxNumberInputs = 4;
			}

			ShaderVec4Node( const Enjon::String& id, const Vec4& vec )
				: ShaderPrimitiveNode( id )
			{
				mData = vec;
				mMaxNumberInputs = 4;
			}

			~ShaderVec4Node( ) { }

			Enjon::String EvaluateToGLSL( ) override
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
			
			virtual Enjon::String GetDeclaration( ) override
			{
				return ( "vec4 " + mID + ";" ); 
			}
	};

	class ShaderMultiplyNode : public BinaryFunctionNode
	{
		public:
			ShaderMultiplyNode( const Enjon::String& id )
				: BinaryFunctionNode( id )
			{ 
			}

			~ShaderMultiplyNode( )
			{ 
			}

			virtual Enjon::String EvaluateToGLSL( ) override
			{
				Enjon::String inputA = mInputs.at( 0 )->Evaulate( );
				Enjon::String inputB = mInputs.at( 1 )->Evaulate( ); 
				return ( inputA + "\n"  + inputB + "\n" + GetQualifiedID( ) + " = " + mInputs.at( 0 )->GetQualifiedID( ) + " * " + mInputs.at( 1 )->GetQualifiedID( ) + ";" );
			}

			virtual Enjon::String GetDeclaration( ) override
			{
				return ( "float " + mID + ";" ); 
			}

		protected:

	};
 
	class ShaderTexture2DNode : public ShaderGraphNode
	{ 
		public:
			/**
			* @brief Constructor
			*/
			ShaderTexture2DNode( const Enjon::String& id )
				: ShaderGraphNode( id )
			{
				mVariableType = ShaderGraphNodeVariableType::UniformVariable;
				mPrimitiveType = ShaderPrimitiveType::Texture2D;
				mMaxNumberInputs = 1;
			}

			/**
			* @brief Destructor
			*/
			~ShaderTexture2DNode( )
			{ 
			}

			/**
			* @brief
			*/
			Enjon::String EvaluateToGLSL( ) override
			{
				// Already defined previously
				return "";
			}
			
			/**
			* @brief
			*/
			virtual Enjon::String GetDeclaration( ) override
			{
				return ( "uniform sampler2D " + mID + ";" ); 
			} 

			/**
			* @brief
			*/
			virtual Enjon::String GetDefinition( ) override
			{
				return ( "vec4 " + mID + "_sample = texture(" + mID + ", texCoords);" );
			}	
			
			/**
			* @brief
			*/
			virtual Enjon::String GetQualifiedID( ) 
			{ 
				return mID + "_sample"; 
			}

		private:
	}; 
}

#endif










