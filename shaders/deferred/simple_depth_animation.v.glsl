#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

uniform mat4 LightSpaceMatrix;
uniform mat4 Model;
uniform vec4 SpriteFrame;

out DATA
{
	vec3 Position;	
	vec2 TexCoords;
}vs_out;

void main()
{             
    gl_Position = LightSpaceMatrix * Model * vec4(position, 1.0f);
    vs_out.Position = position;

	// Calculate tex coords from sprite frame
	vec2 TexCoords = vec2(texCoords.x, texCoords.y);
	vs_out.TexCoords = SpriteFrame.xy + (TexCoords * SpriteFrame.zw);
}