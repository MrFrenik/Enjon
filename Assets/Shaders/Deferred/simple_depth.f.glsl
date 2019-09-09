#version 330 core

in DATA
{
	vec3 Position;	
	vec2 TexCoords;
}fs_in;

uniform sampler2D DiffuseMap;
uniform float Near;
uniform float Far;

float LinearizeDepth(float Depth)
{
    float z = Depth * 2.0 - 1.0;
    return (2.0 * Near * Far) / (Far + Near - z * (Far - Near));
}

out vec4 DepthColor;

void main()
{
	vec4 Color = texture(DiffuseMap, fs_in.TexCoords);
	if (Color.a < 0.5) discard;
    // float Depth = LinearizeDepth(gl_FragCoord.z);

    DepthColor = Color;
}