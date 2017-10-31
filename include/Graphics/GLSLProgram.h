#ifndef ENJON_GLSLPROGRAM_H
#define ENJON_GLSLPROGRAM_H

#include <GLEW/glew.h>

#include <unordered_map>
#include <string>

#include "System/Types.h"
#include "Defines.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 

namespace Enjon { 

	class ColorRGBA32;
	class GLTexture;
	class Vec2;
	class Vec3;
	class Vec4;
	class Mat4;
	class Transform;

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
		void BindTexture(const std::string& name, const GLTexture& texture, const GLuint index);

		// template <typename T>
		// void SetUniform(const std::string& Name, const T& Val);

		void SetUniform(const std::string& name, const s32& val);
		void SetUniform(const std::string& name, f32* val, s32 count);
		void SetUniform(const std::string& name, s32* val, s32 count);
		void SetUniform(const std::string& name, const f64& val);
		void SetUniform(const std::string& name, const f32& val);
		void SetUniform(const std::string& name, const Vec2& vector);
		void SetUniform(const std::string& name, const Vec3& vector);
		void SetUniform(const std::string& name, const Vec4& vector);
		void SetUniform(const std::string& name, const Mat4& matrix); 
		void SetUniform(const std::string& name, const Transform& T);
		void SetUniform(const std::string& name, ColorRGBA32& C);
		
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

	struct UniformBase
	{
		virtual void Set() = 0;
		GLSLProgram* mShader;
	};

	template <typename T>
	struct Uniform
	{
		Uniform()
			: mName("UNSET_UNIFORM"), mShader(nullptr)
		{}

		Uniform(const char* name, T value, GLSLProgram* shader)
			: mName(name), mValue(value), mShader(shader)
		{}

		void Set()
		{
			assert(mShader != nullptr);
			mShader->SetUniform(name, value);
		}

		const char* name;
		T value;
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif