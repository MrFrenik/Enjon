#version 330 core
in vec2 TexCoords;

in DATA
{
    vec2 position;
    vec4 color;
    vec2 uv; 
}fs_in;

out vec4 color;

uniform sampler2D tex;
uniform sampler2D u_position;

struct Light {
    vec3 Position;
    vec4 Color;
    vec3 Falloff;
    float Radius;
};

const int N_LIGHTS = 100;
uniform Light Lights[N_LIGHTS];

uniform int NumberOfLights;

//values used for shading algorithm...
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 
uniform mat4 View;
uniform float Scale;

void main()
{
    //RGBA of our diffuse color
    vec4 DiffuseColor = texture(tex, fs_in.uv);

    vec4 FragPos = texture(u_position, fs_in.uv);

    vec3 Position = FragPos.xyz;

    // Hardcoded ambient light
    vec3 Lighting = AmbientColor.rgb * AmbientColor.a * DiffuseColor.rgb;
    // vec3 Lighting = DiffuseColor.rgb;

    for (int i = 0; i < NumberOfLights; i++)
    {
        float Intensity = 1.0 / length(fs_in.position - Lights[i].Position.xy);

        Lighting += Lights[i].Color.rgb * Intensity;
    }

    // color = texture(u_position, fs_in.TexCoords);
    color = vec4(Lighting, DiffuseColor.a);

    // Emissiveness
    // if (DiffuseColor.r >= 1.0 || DiffuseColor.g >= 1.0 || DiffuseColor.b >= 1.0) color += DiffuseColor * DiffuseColor.a;
}














