#version 330 core
in vec2 TexCoords;

layout (location = 0) out vec4 diffuse;     // Diffuse
layout (location = 1) out vec4 position;
layout (location = 2) out vec4 normals;

out vec4 color;

uniform sampler2D u_diffuse;
uniform sampler2D u_normals;
uniform sampler2D u_position;
uniform sampler2D u_depth;

// Vertex information
in DATA
{
    vec2 Position;  
    vec2 TexCoords;
}fs_in;

struct Light {
    vec3 Position;
    vec4 Color;
    vec3 Falloff;
    float Radius;
    float Depth;
    float Intensity;
};

const int N_LIGHTS = 100;
uniform Light Lights[N_LIGHTS];

uniform int NumberOfLights;

//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 
uniform vec3 ViewPos;         //attenuation coefficients
uniform mat4 CameraInverse;
uniform mat4 View;
uniform float Scale;
uniform float LightDepth;

void main()
{
    //RGBA of our diffuse color
    vec4 DiffuseColor = texture2D(u_diffuse, fs_in.TexCoords);

    vec4 DepthColor = texture2D(u_depth, fs_in.TexCoords);
    float Depth = DepthColor.r * DepthColor.a;

    vec4 FragPos = texture2D(u_position, fs_in.TexCoords);
    FragPos.z = Depth;

    // //RGB of our normal map
    vec3 NormalMap = texture2D(u_normals, fs_in.TexCoords).rgb;
    vec4 NormalColor = texture2D(u_normals, fs_in.TexCoords);

    // Hardcoded ambient light
    vec3 Lighting = AmbientColor.rgb * AmbientColor.a * DiffuseColor.rgb;

    vec3 Position = FragPos.xyz;

    vec3 CamView = (View * vec4(ViewPos, 1.0f)).xyz;

    vec3 ViewDir = normalize(vec3(ViewPos.xy - Position.xy, ViewPos.z));

        for (int i = 0; i < NumberOfLights; i++)
        {
            vec3 LightPosVS = (View * vec4(Lights[i].Position.xyz, 1.0f)).xyz;

            // Calculate isometric light depth
            vec3 LightDir = (1.0 / Scale) * vec3(LightPosVS.xy - Position.xy, Lights[i].Position.z);
            // vec3 LightDir = vec3(LightPosVS.xy - Position.xy, Lights[i].Position.z);

            // //Determine distance (used for attenuation) BEFORE we normalize our LightDir
            float D = length(LightDir);

            LightDir = normalize(LightDir);

            LightDir.x *= Resolution.x / Resolution.y;

            vec3 L = normalize(LightDir);
        
            //normalize our vectors
            // vec3 N = normalize((NormalMap - 0.5) * 2.0);

            // vec3 HalfwayDir = normalize(LightDir + ViewDir);

            // float spec = pow(max(dot(N, HalfwayDir), 0.0), 1024.0);

            // Get light color
            vec4 LightColor = Lights[i].Color;

            // vec3 Specular = LightColor.rgb * spec;

            vec3 Diffuse = Lights[i].Intensity * LightColor.rgb * DiffuseColor.rgb * max(1.0 - D/Lights[i].Radius, 0.0);

            //Pre-multiply light color with intensity
            //Then perform "N dot L" to determine our diffuse term
            // vec3 Diffuse = LightColor.rgb * DiffuseColor.rgb * max(dot(N, LightDir), 0.0);
            // vec3 Diffuse = LightColor.rgb * max(D/Lights[i].Radius, 0.0);
            
            //calculate attenuation
            vec3 Falloff = Lights[i].Falloff;

            float Attenuation = 1.0 / (Falloff.x + Falloff.y * D + Falloff.z * D * D) / Scale;

            Diffuse *= Attenuation * LightColor.a;

            Lighting += Diffuse;
        }

    // Lighting.r = clamp(Lighting.r, 0, 1);
    // Lighting.g = clamp(Lighting.g, 0, 1);
    // Lighting.b = clamp(Lighting.b, 0, 1);
    
    color = vec4(Lighting, 1.0);

    diffuse = color;
    position = color;
    normals = color;
}
