#version 330

in vec2 texCoord;
out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec4 vColor;

void main()
{
	vec4 vTexColor = texture(gSampler, texCoord);
	outputColor = vec4(vTexColor.r, vTexColor.r, vTexColor.r, vTexColor.r)*vColor;
}