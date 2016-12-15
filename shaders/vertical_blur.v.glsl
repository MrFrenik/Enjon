#version 330 core 

layout (location = 0) in vec2 vertexPosition;
layout (location = 1) in vec4 vertexColor;
layout (location = 2) in vec2 vertexUV; 

out DATA
{
	vec2 Position;	
	vec4 Color;
	vec2 TexCoords;
}fs_out;

out vec2 v_blurTexCoords[16];

void main()
{             
    gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0);
    fs_out.TexCoords = vertexUV;
    fs_out.Position = vertexPosition;
    fs_out.Color = vertexColor;

	vec2 v_texCoord = fs_out.TexCoords;
	v_blurTexCoords[ 0] = v_texCoord + vec2(0.0, -0.032);
	v_blurTexCoords[ 1] = v_texCoord + vec2(0.0, -0.028);
	v_blurTexCoords[ 2] = v_texCoord + vec2(0.0, -0.024);
	v_blurTexCoords[ 3] = v_texCoord + vec2(0.0, -0.020);
	v_blurTexCoords[ 4] = v_texCoord + vec2(0.0, -0.016);
	v_blurTexCoords[ 5] = v_texCoord + vec2(0.0, -0.012);
	v_blurTexCoords[ 6] = v_texCoord + vec2(0.0, -0.008);
	v_blurTexCoords[ 7] = v_texCoord + vec2(0.0, -0.004);	// middle
	v_blurTexCoords[ 8] = v_texCoord + vec2(0.0,  0.004);  // middle
	v_blurTexCoords[ 9] = v_texCoord + vec2(0.0,  0.008);
	v_blurTexCoords[10] = v_texCoord + vec2(0.0,  0.012);
	v_blurTexCoords[11] = v_texCoord + vec2(0.0,  0.016);
	v_blurTexCoords[12] = v_texCoord + vec2(0.0,  0.020);
	v_blurTexCoords[13] = v_texCoord + vec2(0.0,  0.024);
	v_blurTexCoords[14] = v_texCoord + vec2(0.0,  0.028);
	v_blurTexCoords[15] = v_texCoord + vec2(0.0,  0.032);
}