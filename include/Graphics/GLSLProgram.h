#ifndef ENJON_GLSLPROGRAM_H
#define ENJON_GLSLPROGRAM_H

#include <GLEW/glew.h>
#include <Math/Maths.h>
#include <Graphics/Transform.h>

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
		GLint GetUniformLocation(const std::string& uniformName);
		void Use();
		void Unuse();

		void BindTexture(const std::string& name, const GLuint& TextureID, const GLuint Index);

		// template <typename T>
		// void SetUniform(const std::string& Name, const T& Val);

		void SetUniform(const std::string& name, const int& val);
		void SetUniform(const std::string& name, float* val, int count);
		void SetUniform(const std::string& name, int* val, int count);
		void SetUniform(const std::string& name, const float& val);
		void SetUniform(const std::string& name, const Math::Vec2& vector);
		void SetUniform(const std::string& name, const Math::Vec3& vector);
		void SetUniform(const std::string& name, const Math::Vec4& vector);
		void SetUniform(const std::string& name, const Math::Mat4& matrix); 
		void SetUniform(const std::string& name, const EG::Transform& T);
		
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