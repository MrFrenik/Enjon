#version 330 core
in vec2 TexCoords;

out vec4 color;

uniform sampler2D screenTexture;
uniform float time;

const float offset = 1.0f / 300.0f;

const float sharpen[9] = float[] (
    -1, -1, -1, 
    -1,  9, -1, 
    -1, -1, -1 
);

const float edge_detection[9] = float[] (
    1,  1, 1,
    1, -8, 1,
    1,  1, 1
);

const float blur[9] = float[] (
    1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f, 
    2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f, 
    1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f
);

void main()
{
    vec2 offsets[9] = vec2[] (
        vec2(-offset,   offset),    // top-left 
        vec2(0.0f,      offset),    // top-center
        vec2(offset,    offset),    // top-right
        vec2(-offset,   0.0f),      // center-left 
        vec2(0.0f,      0.0f),      // center-center
        vec2(offset,    0.0f),      // center-right
        vec2(-offset,   -offset),   // bottom-left
        vec2(0.0f,      -offset),   // bottom-center
        vec2(offset,    -offset)    // bottom-right
    );


    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++)
    {
        col += sampleTex[i] * edge_detection[i];
    }

    color = vec4(col, 1.0);
} 