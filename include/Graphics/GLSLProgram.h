#ifndef ENJON_GLSLPROGRAM_H
#define ENJON_GLSLPROGRAM_H

#include <GLEW/glew.h>
#include <Defines.h>

#include <unordered_map>
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Enjon { namespace Math { 

	struct Vec2;
	struct Vec3;
	struct Vec4;
	struct Mat4;	
	struct Transform;
}}

namespace Enjon { namespace Graphics {

	class ColorRGBA16;

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
		void SetUniform(const std::string& name, const double& val);
		void SetUniform(const std::string& name, const float& val);
		void SetUniform(const std::string& name, const EM::Vec2& vector);
		void SetUniform(const std::string& name, const EM::Vec3& vector);
		void SetUniform(const std::string& name, const EM::Vec4& vector);
		void SetUniform(const std::string& name, const EM::Mat4& matrix); 
		void SetUniform(const std::string& name, const EM::Transform& T);
		void SetUniform(const std::string& name, const EG::ColorRGBA16& C);
		
		GLuint inline GetProgramID() const { return m_programID; } 
	
	private:
		int m_numAttributes; 

		GLuint m_programID;

		GLuint m_vertexShaderID;
		GLuint m_fragmentShaderID;
		std::unordered_map<std::string, GLuint> UniformMap;
	
	private: 
		void CompileShader(const char* filePath, GLuint id);
		void LinkShaders();
	};
}}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif