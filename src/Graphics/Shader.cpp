// @file Shader.cpp
// Copyright 2016-2017 John Jackson. All Rights Reserved.

#include "Graphics/Shader.h" 
#include "Graphics/Texture.h"
#include "Graphics/Material.h"
#include "Utils/Errors.h"

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

	Shader::Shader( const ShaderGraph& graph )
		: mGraph( graph )
	{
		Compile( );
	}

	Enjon::Result Shader::Compile( )
	{
		// Check graph validity here? Or does this need to be checked elsewhere
		Enjon::Result valid = mGraph.Compile( );

		if ( valid != Enjon::Result::SUCCESS )
		{
			// Failure
		}

		// Parse shader for vertex shader output
		Enjon::String vertexShaderCode = mGraph.Parse( Enjon::ShaderType::Vertex );

		// Parse shader for fragment shader output
		Enjon::String fragmentShaderCode = mGraph.Parse( Enjon::ShaderType::Fragment );

		std::cout << vertexShaderCode << "\n\n";
		std::cout << fragmentShaderCode << "\n";

		// Create GLSL Program
		mProgramID = glCreateProgram( );

		// Create vertex shader
		mVertexShaderID = glCreateShader( GL_VERTEX_SHADER );
		// Create fragment shader
		mFragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

		// Compile vertex shader
		Enjon::Result vertexResult = CompileShader( vertexShaderCode, mVertexShaderID );
		// Compile frament shader
		Enjon::Result fragmentResult = CompileShader( fragmentShaderCode, mFragmentShaderID );

		// Link shaders 
		LinkProgram( );

		auto refs = mGraph.GetUniforms( );
		for ( auto& r : refs )
		{
			std::cout << r.mName << ", " << r.mLocation << '\n';
		}

		return Enjon::Result::SUCCESS;
	}

	//=======================================================================================================================

	Shader::~Shader( )
	{ 
	}

	//=======================================================================================================================
			
	Enjon::Result Shader::Recompile( )
	{
		// Destroy the program
		Enjon::Result destResult = DestroyProgram( ); 

		// Recompile the graph and shader program
		Enjon::Result comResult = Compile( );
		
		return Enjon::Result::SUCCESS;
	}

	Enjon::Result Shader::DestroyProgram( )
	{ 
		// Delete the program id
		if ( mProgramID )
		{
			glDeleteProgram( mProgramID );
		}

		return Enjon::Result::SUCCESS;
	}

	//=======================================================================================================================
			
	void Shader::Use( )
	{
		 glUseProgram(mProgramID);
	}

	//=======================================================================================================================

	void Shader::Unuse( )
	{ 
		 glUseProgram(0);
	}

	//======================================================================================================================= 
			
	const Enjon::ShaderGraph* Shader::GetGraph( )
	{
		return &mGraph;
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

			//Provide the infolog in whatever manor you deem best.
			//Exit with failure.
			glDeleteShader( shaderID ); //Don't leak the shader.

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

		//Always detach shaders after a successful link.
		glDetachShader(mProgramID, mVertexShaderID);
		glDetachShader(mProgramID, mFragmentShaderID);
		glDeleteShader(mVertexShaderID);
		glDeleteShader(mFragmentShaderID); 

		// Return success
		return Enjon::Result::SUCCESS;
	}
			
	/*
	* @brief
	*/
	Enjon::AssetHandle< Enjon::Material > Shader::CreateMaterial( )
	{
		// Create new material
		Enjon::Material* material = new Enjon::Material; 

		// Set shader
		material->mMaterialShader = this;

		const std::vector< UniformReference >& refs = mGraph.GetUniforms( );
		for ( auto& r : refs )
		{
			// Build uniforms for material
			switch ( r.mType )
			{
				case ShaderPrimitiveType::Texture2D:
				{
					// Need to have texture handle somehow...
					Enjon::AssetHandle< Enjon::Texture > texture;
					UniformTexture* tex = new UniformTexture( r.mName, this, texture, r.mLocation );
					material->AddUniform( tex );

				} break;

				case ShaderPrimitiveType::Float: 
				{
					UniformPrimitive< f32 >* uniform = new UniformPrimitive< f32 >( r.mName, this, 0.0f, r.mLocation );
					material->AddUniform( uniform );
				} break;

				case ShaderPrimitiveType::Vec2:
				{
					UniformPrimitive< Vec2 >* uniform = new UniformPrimitive< Vec2 >( r.mName, this, Enjon::Vec2( 0.0f ), r.mLocation );
					material->AddUniform( uniform );
				} break;
				
				case ShaderPrimitiveType::Vec3:
				{
					UniformPrimitive< Vec3 >* uniform = new UniformPrimitive< Vec3 >( r.mName, this, Enjon::Vec3( 0.0f ), r.mLocation );
					material->AddUniform( uniform );
				} break;
				
				case ShaderPrimitiveType::Vec4:
				{
					UniformPrimitive< Vec4 >* uniform = new UniformPrimitive< Vec4 >( r.mName, this, Enjon::Vec4( 0.0f ), r.mLocation );
					material->AddUniform( uniform );
				} break;
				
				case ShaderPrimitiveType::Mat4:
				{
					UniformPrimitive< Mat4 >* uniform = new UniformPrimitive< Mat4 >( r.mName, this, Enjon::Mat4( 1.0f ), r.mLocation );
					material->AddUniform( uniform );
				} break;

				default:
				{ 
				} break;
			}
		}

		return material; 
	}
	
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
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
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
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
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
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
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
		//else
		//{
		//	GLuint Location = GetUniformLocation(name);
		//	mUniformMap[name] = Location;
		//	glUniform1f(Location, val);
		//	// EU::FatalError("Error: GLGLProgram: SetUniform: Uniform not found: " + name);
		//}
	}

	void Shader::SetUniform(const std::string& name, const Vec2& vector)
	{
		// glUniform2f(GetUniformLocation(name), vector.x, vector.y);
		auto Search = mUniformMap.find(name);
		if (Search != mUniformMap.end())
		{
			glUniform2f(Search->second, vector.x, vector.y);
		}
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
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
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
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
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
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
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
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
		//else
		//{
		//	GLuint Location = GetUniformLocation(name);
		//	mUniformMap[name] = Location;
		//	glUniform1f(Location, val);
		//	// EU::FatalError("Error: GLGLProgram: SetUniform: Uniform not found: " + name);
		//}
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
		else
		{
			EU::FatalError("Error: Shader: SetUniform: Uniform not found: " + name);
		}

		glBindTexture(GL_TEXTURE_2D, TextureID);
	} 

	//======================================================================================================================= 
			
	UniformTexture::UniformTexture( const Enjon::String& name, const Enjon::Shader* shader, const Enjon::AssetHandle< Enjon::Texture >& texture, u32 location )
	{
		mType = UniformType::TextureSampler;
		mShader = shader;
		mTexture = texture;
		mLocation = location;
		mName = name;
	}

	//======================================================================================================================= 
			
	UniformTexture::~UniformTexture( )
	{ 
	}

	//======================================================================================================================= 
	
	void UniformTexture::Set( )
	{
		const_cast< Enjon::Shader* >( mShader )->BindTexture( mName, mTexture.Get( )->GetTextureId( ), mLocation );
	}

	//======================================================================================================================= 
}

