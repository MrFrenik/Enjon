// @file Shader.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/Shader.h" 
#include "Graphics/Texture.h"
#include "Graphics/Material.h"
#include "Utils/Errors.h"
#include "Utils/FileUtils.h"
#include "Asset/AssetManager.h"
#include "Graphics/GraphicsSubsystem.h"
#include "SubsystemCatalog.h"
#include "Engine.h"

#include <GLEW/glew.h> 
#include <vector>
#include <iostream>

namespace Enjon
{ 

	//=======================================================================================================================

	Shader::Shader( ) 
	{ 
	}

	//=======================================================================================================================

	Shader::Shader( const AssetHandle<ShaderGraph>& graph, ShaderPassType passType )
		: mGraph( graph ), mPassType( passType )
	{
		Compile( );
	}

	/**
	* @brief Constructor
	*/
	Shader::Shader( const AssetHandle< ShaderGraph >& graph, ShaderPassType passType, const String& vertexShaderCode, const String& fragmentShaderCode )
		: mGraph( graph ), mPassType( passType )
	{ 
		Compile( true, vertexShaderCode, fragmentShaderCode );
	}

	Enjon::Result Shader::Compile( bool passedCode, const String& vertexCode, const String& fragmentCode )
	{ 
		Enjon::String vertexShaderCode = vertexCode;
		Enjon::String fragmentShaderCode = fragmentCode; 

		// Get shader resource path
		Enjon::String sp = Enjon::Engine::GetInstance( )->GetSubsystemCatalog( )->Get< Enjon::AssetManager >( )->GetAssetsDirectoryPath( ) + "/Shaders";

		// Get vertex file path 
		Enjon::String vertName = mGraph->GetName( ) + "." + ShaderGraph::ShaderPassToString( mPassType ) + ".Vertex.glsl";
		Enjon::String vertPath = sp + "/" + vertName;
		
		// Get fragment file path 
		Enjon::String fragName = mGraph->GetName( ) + "." + ShaderGraph::ShaderPassToString( mPassType ) + ".Fragment.glsl";
		Enjon::String fragPath = sp + "/" + fragName; 

		if ( !passedCode )
		{
			// Parse shader for vertex shader output 
			Enjon::String vertexShaderCode = Enjon::Utils::read_file( vertPath.c_str( ) );

			// Parse shader for fragment shader output
			Enjon::String fragmentShaderCode = Enjon::Utils::read_file( fragPath.c_str( ) ); 

		}

		// Create GLSL Program
		mProgramID = glCreateProgram( );

		// Create vertex shader
		mVertexShaderID = glCreateShader( GL_VERTEX_SHADER );
		// Create fragment shader
		mFragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

		// Compile vertex shader
		Enjon::Result vertexResult = CompileShader( vertexShaderCode, mVertexShaderID );

		// Report any errors
		if ( vertexResult != Result::SUCCESS )
		{
			std::cout << "Error: Vertex shader failed to compile: " + vertName + "\n";
		}

		// Compile frament shader
		Enjon::Result fragmentResult = CompileShader( fragmentShaderCode, mFragmentShaderID );

		// Report any errors
		if ( fragmentResult != Result::SUCCESS )
		{
			std::cout << "Error: Fragment shader failed to compile: " + fragName + "\n"; 
		}

		// If either fails, return error
		if ( vertexResult != Enjon::Result::SUCCESS || fragmentResult != Enjon::Result::SUCCESS )
		{
			// Error
			return Enjon::Result::FAILURE;
		}

		// Link shaders 
		LinkProgram( ); 

		// Destroy previous program if active
		//DestroyProgram( ); 

		return Enjon::Result::SUCCESS;
	}

	//=======================================================================================================================

	Shader::~Shader( )
	{ 
		DestroyProgram( );
	}

	//=======================================================================================================================
			
	Enjon::Result Shader::Recompile( )
	{
		// Destroy the program
		Enjon::Result destResult = DestroyProgram( ); 

		// Recompile the graph and shader program
		return Compile( ); 
	}

	Enjon::Result Shader::DestroyProgram( )
	{ 
		// Delete the program id
		if ( mProgramID )
		{
			glDeleteProgram( mProgramID );
		}
		
		// Delete vertex shader
		if ( mVertexShaderID )
		{
			glDeleteShader( mVertexShaderID );
		}

		// Delete fragment shader
		if ( mFragmentShaderID )
		{
			glDeleteShader( mFragmentShaderID );
		}

		return Enjon::Result::SUCCESS;
	}

	//=======================================================================================================================
			
	void Shader::Use( )
	{
		glUseProgram( mProgramID );
	}

	//=======================================================================================================================

	void Shader::Unuse( )
	{ 
		glUseProgram( 0 );
	}

	//======================================================================================================================= 
			
	const Enjon::ShaderGraph* Shader::GetGraph( )
	{
		return mGraph.Get();
	}

	//======================================================================================================================= 

	Enjon::Result Shader::CompileShader( const Enjon::String& shaderCode, u32 shaderID ) 
	{ 
		//Tell opengl that we want to use fileContents as the contents of the shader file
		const char* source = shaderCode.c_str( ); 
		glShaderSource( (GLuint)shaderID, 1, &source, nullptr );

		//Compile the shader
		glCompileShader( (GLuint)shaderID );

		//Check for errors
		GLint success = 0;
		glGetShaderiv( shaderID, GL_COMPILE_STATUS, &success );

		if ( success == GL_FALSE )
		{
			GLint maxLength = 0;
			glGetShaderiv( shaderID, GL_INFO_LOG_LENGTH, &maxLength );

			//The maxLength includes the NULL character
			std::vector<char> errorLog( maxLength );
			glGetShaderInfoLog( shaderID, maxLength, &maxLength, &errorLog[ 0 ] );

			// Delete shader
			glDeleteShader( shaderID );

			//Print error log and quit
			std::printf( "%s\n", &( errorLog[ 0 ] ) );

			return Result::FAILURE;
		} 

		return Result::SUCCESS;
	} 

	//======================================================================================================================= 

	Enjon::Result Shader::LinkProgram( )
	{
		 //Attach our shaders to our program
		glAttachShader(mProgramID, mVertexShaderID);
		glAttachShader(mProgramID, mFragmentShaderID);

		//Link our program
		glLinkProgram(mProgramID);

		//Create info log
		GLint isLinked = 0;
		glGetProgramiv(mProgramID, GL_LINK_STATUS, (s32*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<char> errorLog(maxLength);
			glGetProgramInfoLog(mProgramID, maxLength, &maxLength, &errorLog[0]); 

			//We don't need the program anymore.
			glDeleteProgram(mProgramID);

			//Don't leak shaders either.
			glDeleteShader(mVertexShaderID);
			glDeleteShader(mFragmentShaderID);

			//print the error log and quit
			std::printf("%s\n", &(errorLog[0]));
			Enjon::Utils::FatalError("Shaders failed to link!");
		}

		s32 UniformCount = -1;
		glGetProgramiv(mProgramID, GL_ACTIVE_UNIFORMS, &UniformCount);
		for (s32 i = 0; i < UniformCount; ++i)
		{
			s32 NameLength = -1;
			s32 Number = -1;
			GLenum Type = GL_ZERO;
			char Name[256];

			glGetActiveUniform(mProgramID, 
							   static_cast<GLuint>(i), 
							   sizeof(Name) - 1, 
							   &NameLength, 
							   &Number, 
							   &Type, 
							   Name);
			Name[NameLength] = 0;

			GLuint Location = glGetUniformLocation(mProgramID, Name);

			// Cache location in map
			mUniformMap[Name] = (u32)Location;
		}

		//Always detach shaders after a successful link
		glDetachShader(mProgramID, mVertexShaderID);
		glDetachShader(mProgramID, mFragmentShaderID);

		// Destroy shaders now that the program is made
		glDeleteShader(mVertexShaderID);
		glDeleteShader(mFragmentShaderID); 

		// Return success
		return Enjon::Result::SUCCESS;
	}

	//==============================================================================================

	u32 Shader::GetProgramID( ) const 
	{
		return mProgramID;
	}

	//==============================================================================================

	
	s32 Shader::GetUniformLocation(const Enjon::String& uniformName) 
	{
		GLint location =  glGetUniformLocation(mProgramID, uniformName.c_str());

		if( location == GL_INVALID_INDEX )
		{
			Utils::FatalError( "Shader::GetUniformLocation::" + uniformName + "_NOT_FOUND" );
		}
		return location;
	} 

	void Shader::SetUniform(const std::string& name, const Mat4& matrix)
	{
		// glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, matrix.elements);	
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniformMatrix4fv(Search->second, 1, GL_FALSE, matrix.elements);
		}
	}
	
	void Shader::SetUniform(const std::string& name, f32* val, s32 count)
	{
		// glUniform1fv(GetUniformLocation(name), count, val);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform1fv(Search->second, count, val);
		}
	}
	
	void Shader::SetUniform(const std::string& name, s32* val, s32 count)
	{
		// glUniform1iv(GetUniformLocation(name), count, val);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform1iv(Search->second, count, val);
		}
	}

	void Shader::SetUniform(const std::string& name, const f32& val)
	{
		// glUniform1f(GetUniformLocation(name), val); 
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform1f(Search->second, val);
		}
	}

	void Shader::SetUniform(const std::string& name, const Vec2& vector)
	{
		// glUniform2f(GetUniformLocation(name), vector.x, vector.y);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform2f(Search->second, vector.x, vector.y);
		}
	}

	void Shader::SetUniform(const std::string& name, const Vec3& vector) 
	{
		// glUniform3f(GetUniformLocation(name), vector.x, vector.y, vector.z);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform3f(Search->second, vector.x, vector.y, vector.z);
		}
	}

	void Shader::SetUniform(const std::string& name, const Vec4& vector)
	{
		// glUniform4f(GetUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform4f(Search->second, vector.x, vector.y, vector.z, vector.w);
		}
	}

	void Shader::SetUniform(const std::string& name, const s32& val)
	{
		// glUniform1i(GetUniformLocation(name), val);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform1i(Search->second, val);
		}
	}

	void Shader::SetUniform(const std::string& name, const f64& val)
	{
		// glUniform1f(GetUniformLocation(name), val);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform1f(Search->second, val);
		}
	} 

	void Shader::SetUniform( const std::string& name, const ColorRGBA32& color )
	{ 
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform4f( Search->second, color.r, color.g, color.b, color.a );
		}
	}

	void Shader::BindTexture(const std::string& name, const u32& TextureID, const u32 Index)
	{
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0 + Index);

		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform1i(Search->second, Index);
		} 

		glBindTexture(GL_TEXTURE_2D, TextureID); 
	} 

	//======================================================================================================================= 
			
	UniformTexture::UniformTexture( const Enjon::String& name, const Enjon::AssetHandle< Enjon::Texture >& texture, u32 location )
	{
		mType = UniformType::TextureSampler2D;
		mTexture = texture;
		mLocation = location;
		mName = name;
	}

	//======================================================================================================================= 

	void UniformTexture::CopyFields( const UniformTexture* other )
	{
		mLocation = other->mLocation;
		mName = String(other->mName);
		mType = other->mType;
		mTexture = other->mTexture; 
	}

	//======================================================================================================================= 
			
	UniformTexture::~UniformTexture( )
	{ 
	}

	//======================================================================================================================= 
	
	void UniformTexture::Bind( const Shader* shader ) const
	{
		const_cast< Enjon::Shader* >( shader )->BindTexture( mName, mTexture.Get( )->GetTextureId( ), mLocation );
	}

	//======================================================================================================================= 

	void UniformFloat::CopyFields( const UniformFloat* other )
	{
		mLocation = other->mLocation;
		mName = other->mName;
		mType = other->mType;
		mValue = other->mValue;
	}
 
	//======================================================================================================================= 

	void UniformVec2::CopyFields( const UniformVec2* other )
	{
		mLocation = other->mLocation;
		mName = other->mName;
		mType = other->mType;
		mValue = other->mValue;
	}
 
	//======================================================================================================================= 

	void UniformVec3::CopyFields( const UniformVec3* other )
	{
		mLocation = other->mLocation;
		mName = other->mName;
		mType = other->mType;
		mValue = other->mValue; 
	}
 
	//======================================================================================================================= 

	void UniformVec4::CopyFields( const UniformVec4* other )
	{
		mLocation = other->mLocation;
		mName = other->mName;
		mType = other->mType;
		mValue = other->mValue;
	}
}

