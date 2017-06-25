#version 330 core

layout (location = 0) out vec4 AlbedoOut;     // Diffuse
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 PositionOut;
layout (location = 3) out vec4 EmissiveOut;
layout (location = 4) out vec4 MatPropsOut;

in vec3 localPos;
  
uniform samplerCube environmentMap;
  
void main()
{
    vec3 envColor = textureLod(environmentMap, localPos, 1.2).rgb; 
    //vec3 envColor = texture(environmentMap, localPos).rgb; 
  
    AlbedoOut = vec4(envColor, 1.0);
	NormalsOut = vec4( 0.0, 0.0, 0.0, 1.0 );
	PositionOut = vec4( 0.0, 0.0, 0.0, 1.0 );
	EmissiveOut = vec4( 0.0, 0.0, 0.0, 1.0 );
	MatPropsOut = vec4( 0.0, 1.0, 0.0, 1.0 );
}