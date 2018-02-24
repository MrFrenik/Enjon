#include "Graphics/ShaderManager.h"
#include "Engine.h"

#include <stdlib.h>
#include <string.h>
#include <unordered_map>

namespace Enjon { namespace ShaderManager {

 	std::unordered_map<std::string, GLSLProgram*> Shaders;

	void Init()
	{
		Enjon::String rp = Enjon::Engine::GetInstance( )->GetConfig( ).GetEngineResourcePath( );
		Enjon::String sp = rp + "/Shaders/";

		// Add shaders
		ShaderManager::AddShader("Basic", sp + "basic.v.glsl", sp + "basic.f.glsl");
		ShaderManager::AddShader("Text", sp + "text.v.glsl", sp + "text.f.glsl");
		ShaderManager::AddShader("Default", sp + "default.v.glsl", sp + "default.f.glsl");
		ShaderManager::AddShader("DiffuseShader", sp + "diffuse.v.glsl", sp + "diffuse.f.glsl");
		ShaderManager::AddShader("DeferredShader", sp + "deferred.v.glsl", sp + "deferred.f.glsl");
		ShaderManager::AddShader("NormalsShader", sp + "normal.v.glsl", sp + "normal.f.glsl");
		ShaderManager::AddShader("DepthShader", sp + "depth.v.glsl", sp + "depth.f.glsl");
		ShaderManager::AddShader("NoCameraProjection", sp + "no_camera.v.glsl", sp + "no_camera.f.glsl");
		ShaderManager::AddShader("BasicLighting", sp + "lighting_basic.v.glsl", sp + "lighting_basic.f.glsl");
		ShaderManager::AddShader("Learn", sp + "learn.v.glsl", sp + "learn.f.glsl");
		ShaderManager::AddShader("Lamp", sp + "learn.v.glsl", sp + "lamplight.f.glsl");
		ShaderManager::AddShader("DefaultLighting", sp + "defaultLight.v.glsl", sp + "defaultLight.f.glsl");
		ShaderManager::AddShader("VerticalBlur", sp + "vertical_blur.v.glsl", sp + "vertical_blur.f.glsl");
		ShaderManager::AddShader("HorizontalBlur", sp + "/horizontal_blur.v.glsl", sp + "horizontal_blur.f.glsl");
		ShaderManager::AddShader("GBuffer", sp + "deferred/deferred.v.glsl", sp + "deferred/deferred.f.glsl");
		ShaderManager::AddShader("DeferredLight", sp + "deferred/deferred_light.v.glsl", sp + "deferred/deferred_light.f.glsl");
		ShaderManager::AddShader("AmbientLight", sp + "deferred/ambient_light.v.glsl", sp + "deferred/ambient_light.f.glsl");
		ShaderManager::AddShader("DirectionalLight", sp + "deferred/directional_light.v.glsl", sp + "deferred/directional_light.f.glsl");
		ShaderManager::AddShader("PointLight", sp + "deferred/point_light.v.glsl", sp + "deferred/point_light.f.glsl");
		ShaderManager::AddShader("SpotLight", sp + "deferred/spot_light.v.glsl", sp + "deferred/spot_light.f.glsl");
		ShaderManager::AddShader("Depth", sp + "deferred/depth.v.glsl", sp + "deferred/depth.f.glsl");
		ShaderManager::AddShader("AnimatedMaterial", sp + "deferred/animated_material.v.glsl", sp + "deferred/animated_material.f.glsl");
		ShaderManager::AddShader("UVAnimation", sp + "/deferred/uv_animation.v.glsl", sp + "deferred/uv_animation.f.glsl");
		ShaderManager::AddShader("ScreenUI", sp + "screen_ui.v.glsl", sp + "screen_ui.f.glsl");
		ShaderManager::AddShader("FXAA", sp + "deferred/fxaa.v.glsl", sp + "deferred/fxaa.f.glsl");
		ShaderManager::AddShader("QuadBatch", sp + "deferred/quad_batch.v.glsl", sp + "deferred/quad_batch.f.glsl");
		ShaderManager::AddShader("WorldText", sp + "deferred/world_text.v.glsl", sp + "deferred/world_text.f.glsl");
		ShaderManager::AddShader("Composite", sp + "deferred/composite.v.glsl", sp + "deferred/composite.f.glsl");
		ShaderManager::AddShader("Bright", sp + "deferred/bright.v.glsl", sp + "deferred/bright.f.glsl");
		ShaderManager::AddShader("SimpleDepth", sp + "deferred/simple_depth.v.glsl", sp + "deferred/simple_depth.f.glsl");
		ShaderManager::AddShader("SimpleDepthAnimation", sp + "deferred/simple_depth_animation.v.glsl", sp + "deferred/simple_depth_animation.f.glsl");
		ShaderManager::AddShader("PBRDirectionalLight", sp + "pbr/directional_light.v.glsl", sp + "pbr/directional_light.f.glsl");
		ShaderManager::AddShader("PBRPointLight", sp + "pbr/point_light.v.glsl", sp + "pbr/point_light.f.glsl");
		ShaderManager::AddShader("PBRSpotLight", sp + "pbr/spot_light.v.glsl", sp + "pbr/spot_light.f.glsl");
		ShaderManager::AddShader("Transparent", sp + "deferred/transparent_material.v.glsl", sp + "deferred/transparent_material.f.glsl");
		ShaderManager::AddShader("EquiToCube", sp + "equiToCubemap.v.glsl", sp + "equiToCubemap.f.glsl");
		ShaderManager::AddShader("SkyBox", sp + "skybox.v.glsl", sp + "skybox.f.glsl");
		ShaderManager::AddShader("IrradianceCapture", sp + "pbr/Irradiance.v.glsl", sp + "pbr/Irradiance.f.glsl");
		ShaderManager::AddShader("PrefilterConvolution", sp + "pbr/PrefilterConvolution.v.glsl", sp + "pbr/PrefilterConvolution.f.glsl");
		ShaderManager::AddShader("BRDFLUT", sp + "pbr/BRDF.v.glsl", sp + "pbr/BRDF.f.glsl");
		ShaderManager::AddShader("SSAO", sp + "SSAO.v.glsl", sp + "SSAO2.f.glsl");
		ShaderManager::AddShader("SSAOBlur", sp + "SSAOBlur.v.glsl", sp + "SSAOBlur.f.glsl"); 
		ShaderManager::AddShader("Instanced", sp + "deferred/Instanced.v.glsl", sp + "deferred/Instanced.f.glsl"); 
		ShaderManager::AddShader("MotionBlur", sp + "PostProcessing/MotionBlur.v.glsl", sp + "PostProcessing/MotionBlur.f.glsl"); 
	}

	void Shutdown( )
	{

	}

	void AddShader( const Enjon::String& shadername, const Enjon::String& vertpath, const Enjon::String& fragpath )
	{
		GLSLProgram* p = new GLSLProgram;
		p->CreateShader( vertpath.c_str( ), fragpath.c_str( ) );
		Shaders[shadername] = p;
	}

	GLSLProgram* GetShader(const char* shadername)
	{
		auto it = Shaders.find(shadername);
		if (it != Shaders.end())
		{
			return it->second;
		}

		else
		{
			// Otherwise not found
			std::string errorstr = shadername;
			Utils::FatalError("SHADERMANAGER::GETSHADER::SHADER_NOT_FOUND::" + errorstr);
			return NULL;
		}	
	} 

	GLSLProgram* Get(const char* shadername)
	{
		auto it = Shaders.find(shadername);
		if (it != Shaders.end())
		{
			return it->second;
		}

		else
		{
			// Otherwise not found
			std::string errorstr = shadername;
			Utils::FatalError("SHADERMANAGER::GETSHADER::SHADER_NOT_FOUND::" + errorstr);
			return NULL;
		}	
	} 
	
	GLuint GetUniformLocation(const char* shadername, const char* uniformname)
	{
		return GetShader(shadername)->GetUniformLocation(uniformname); 
	} 

	void UseProgram(const char* shadername)
	{
		GetShader(shadername)->Use(); 
	}

	void UnuseProgram(const char* shadername)
	{
		GetShader(shadername)->Unuse();
	}
	
	void DeleteShaders()
	{ 
		for ( auto& s : Shaders )
		{
			delete s.second;
			s.second = nullptr;
		}

		Shaders.clear( );
	}

}}