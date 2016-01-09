#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H

#include <GLEW/glew.h>
#include <Math/Maths.h>

#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Graphics {

	class GLSLProgram
	{
	public:
		GLSLProgram();
		~GLSLProgram(); 

		void CreateShader(const char* vertexShaderFilePath, const char* fragmentShaderFilepath); 
		void AddAttribute(const GLchar* attributeName); 
		GLint GetUniformLocation(const GLchar* uniformName); 
		void Use();
		void Unuse();

		void SetUniform1i(const GLchar* name, const int& val);
		void SetUniform1fv(const GLchar* name, float* val, int count);
		void SetUniform1iv(const GLchar* name, int* val, int count);
		void SetUniform1f(const GLchar* name, const float& val);
		void SetUniform2f(const GLchar* name, const Math::Vec2& vector);
		void SetUniform3f(const GLchar* name, const Math::Vec3& vector);
		void SetUniform4f(const GLchar* name, const Math::Vec4& vector);
		void SetUniformMat4(const GLchar* name, const Math::Mat4& matrix); 
		
		GLuint inline GetProgramID() const { return m_programID; } 
	
	private:
		int m_numAttributes; 

		GLuint m_programID;

		GLuint m_vertexShaderID;
		GLuint m_fragmentShaderID;
			
	
	private: 
		void CompileShader(const char* filePath, GLuint id);
		void LinkShaders();
	};

}}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif