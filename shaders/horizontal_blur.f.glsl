#version 330 core

in DATA
{
	vec2 Position;	
	vec4 Color;
	vec2 TexCoords;
}fs_in;

in vec2 v_blurTexCoords[14];

out vec4 color;

uniform sampler2D tex;
uniform float time;

void main() 
{
	color = vec4(0.0);
	color += texture2D(tex, v_blurTexCoords[ 0])*0.0044299121055113265;
	color += texture2D(tex, v_blurTexCoords[ 1])*0.00895781211794;
	color += texture2D(tex, v_blurTexCoords[ 2])*0.0215963866053;
	color += texture2D(tex, v_blurTexCoords[ 3])*0.0443683338718;
	color += texture2D(tex, v_blurTexCoords[ 4])*0.0776744219933;
	color += texture2D(tex, v_blurTexCoords[ 5])*0.115876621105;
	color += texture2D(tex, v_blurTexCoords[ 6])*0.147308056121;
	color += texture2D(tex, fs_in.TexCoords )*0.159576912161;
	color += texture2D(tex, v_blurTexCoords[ 7])*0.147308056121;
	color += texture2D(tex, v_blurTexCoords[ 8])*0.115876621105;
	color += texture2D(tex, v_blurTexCoords[ 9])*0.0776744219933;
	color += texture2D(tex, v_blurTexCoords[10])*0.0443683338718;
	color += texture2D(tex, v_blurTexCoords[11])*0.0215963866053;
	color += texture2D(tex, v_blurTexCoords[12])*0.00895781211794;
	color += texture2D(tex, v_blurTexCoords[13])*0.0044299121055113265;
}

