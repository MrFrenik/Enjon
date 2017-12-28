#version 330 core 

in vec2 TexCoords;

layout (location = 0) out vec4 ColorOut;

uniform sampler2D tex;

void main() 
{
	ColorOut = texture(tex, TexCoords);
}

