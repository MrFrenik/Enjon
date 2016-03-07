#include <vector> 
#include <fstream>

#include "Graphics/GLSLProgram.h"
#include "Utils/FileUtils.h"
#include "Utils/Errors.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

namespace Enjon { namespace Graphics { 


	GLSLProgram::GLSLProgram() 
		: m_numAttributes(0), m_programID(0), m_vertexShaderID(0), m_fragmentShaderID(0)
	{}


	GLSLProgram::~GLSLProgram() {}

	void GLSLProgram::CreateShader(const char* vertexShaderFilePath, const char* fragmentShaderFilepath) {
		
		//Create program and get ID from OpenGL
		m_programID = glCreateProgram();

		//Create the vertex shader object, and store its ID
		m_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		if (m_vertexShaderID == 0) {
			Utils::FatalError("Vertex shader failed to be created!");
		}

		//Create the fragment shader object, and store its ID
		m_fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		if (m_fragmentShaderID == 0) {
			Utils::FatalError("Fragment shader failed to be created!");
		}

		//Compile each shader
		CompileShader(vertexShaderFilePath, m_vertexShaderID);
		CompileShader(fragmentShaderFilepath, m_fragmentShaderID);

		//Link shaders once compiled
		LinkShaders();
	}

	void GLSLProgram::LinkShaders() {

		//Attach our shaders to our program
		glAttachShader(m_programID, m_vertexShaderID);
		glAttachShader(m_programID, m_fragmentShaderID);

		//Link our program
		glLinkProgram(m_programID);

		//Create info log
		GLint isLinked = 0;
		glGetProgramiv(m_programID, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<char> errorLog(maxLength);
			glGetProgramInfoLog(m_programID, maxLength, &maxLength, &errorLog[0]); 


			//We don't need the program anymore.
			glDeleteProgram(m_programID);

			//Don't leak shaders either.
			glDeleteShader(m_vertexShaderID);
			glDeleteShader(m_fragmentShaderID);

			//print the error log and quit
			std::printf("%s\n", &(errorLog[0]));
			Utils::FatalError("Shaders failed to link!");
		}

		//Always detach shaders after a successful link.
		glDetachShader(m_programID, m_vertexShaderID);
		glDetachShader(m_programID, m_fragmentShaderID);
		glDeleteShader(m_vertexShaderID);
		glDeleteShader(m_fragmentShaderID);
	}

	//Adds an attribute to our shader. Should be called between compiling and linking.
	void GLSLProgram::AddAttribute(const GLchar* attributeName) {
		glBindAttribLocation(m_programID, m_numAttributes++, attributeName);
	}

	//enable the shader, and all its attributes
	void GLSLProgram::Use() {
		glUseProgram(m_programID);
		//enable all the attributes we added with addAttribute
		for (int i = 0; i < m_numAttributes; i++) {
			glEnableVertexAttribArray(i);
		}
	}

	//disable the shader
	void GLSLProgram::Unuse() {
		glUseProgram(0);
		for (int i = 0; i < m_numAttributes; i++) {
			glDisableVertexAttribArray(i);
		}
	}

	//Compiles a single shader file
	void GLSLProgram::CompileShader(const char* filePath, GLuint id) { 

		//Read from file
		std::string fileContents = Utils::read_file(filePath);

		//Get a pointer to our file contents c string;
		const char* contentsPtr = fileContents.c_str();
		//Tell opengl that we want to use fileContents as the contents of the shader file
		glShaderSource(id, 1, &contentsPtr, nullptr);

		//Compile the shader
		glCompileShader(id);

		//Check for errors
		GLint success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<char> errorLog(maxLength);
			glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

			//Provide the infolog in whatever manor you deem best.
			//Exit with failure.
			glDeleteShader(id); //Don't leak the shader.

			//Print error log and quit
			std::printf("%s\n", &(errorLog[0]));
			std::string path = filePath;
			Utils::FatalError("GLSLPROGRAM::COMPILESHADER::FAILED_TO_COMPILE::" + path);
		}
	}

	GLint GLSLProgram::GetUniformLocation(const GLchar* uniformName) 
	{
		GLint location =  glGetUniformLocation(m_programID, uniformName);

		if( location == GL_INVALID_INDEX )
		{
			std::string name = uniformName;
			Utils::FatalError( "GLSLPROGRAM::GETUNIFORMLOCATION::" + name + "_NOT_FOUND" );
		}
		return location;
	} 
		
	void GLSLProgram::SetUniformMat4(const GLchar* name, const Math::Mat4& matrix)
	{
		glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, matrix.elements);	
	}
	
	void GLSLProgram::SetUniform1fv(const GLchar* name, float* val, int count)
	{
		glUniform1fv(GetUniformLocation(name), count, val);
	}
	
	void GLSLProgram::SetUniform1iv(const GLchar* name, int* val, int count)
	{
		glUniform1iv(GetUniformLocation(name), count, val);
	}

	void GLSLProgram::SetUniform1f(const GLchar* name, const float& val)
	{
		glUniform1f(GetUniformLocation(name), val); 
	}

	void GLSLProgram::SetUniform2f(const GLchar* name, const Math::Vec2& vector)
	{
		glUniform2f(GetUniformLocation(name), vector.x, vector.y);
	}

	void GLSLProgram::SetUniform3f(const GLchar* name, const Math::Vec3& vector)
	{
		glUniform3f(GetUniformLocation(name), vector.x, vector.y, vector.z);
	}

	void GLSLProgram::SetUniform4f(const GLchar* name, const Math::Vec4& vector)
	{
		glUniform4f(GetUniformLocation(name), vector.x, vector.y, vector.z, vector.w);
	}

	void GLSLProgram::SetUniform1i(const GLchar* name, const int& val)
	{
		glUniform1i(GetUniformLocation(name), val);
	}

}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 