#version 330 core
in vec2 TexCoords;

out vec4 color;

uniform sampler2D u_diffuse;
uniform sampler2D u_normals;
uniform sampler2D u_position;

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
};

const int N_LIGHTS = 32;
uniform Light Lights[N_LIGHTS];


//values used for shading algorithm...
uniform vec2 Resolution;      //resolution of screen
// uniform vec3 LightPos;        //light position, normalized
// uniform vec4 LightColor;      //light RGBA -- alpha is intensity
uniform vec4 AmbientColor;    //ambient RGBA -- alpha is intensity 
// uniform vec3 Falloff;         //attenuation coefficients
uniform vec3 ViewPos;         //attenuation coefficients
// uniform float Radius;
uniform mat4 CameraInverse;
uniform mat4 View;

void main()
{
    //RGBA of our diffuse color
    vec4 DiffuseColor = texture2D(u_diffuse, fs_in.TexCoords);

    vec4 FragPos = texture2D(u_position, fs_in.TexCoords);

    // //RGB of our normal map
    vec3 NormalMap = texture2D(u_normals, fs_in.TexCoords).rgb;

    // Hardcoded ambient light
    vec3 Lighting = AmbientColor.rgb * AmbientColor.a * DiffuseColor.rgb;

    vec3 Position = FragPos.xyz;

    // Get view direction
    vec3 ViewDir = normalize(vec3(ViewPos.xy - Position.xy, ViewPos.z));

    for (int i = 0; i < N_LIGHTS; i++)
    {
        vec3 LightPosVS = (View * vec4(Lights[i].Position.xyz, 1.0f)).xyz;

        // //The delta position of light
        vec3 LightDir = vec3(LightPosVS.xy - Position.xy, Lights[i].Position.z);

        // //Determine distance (used for attenuation) BEFORE we normalize our LightDir
        float D = length(LightDir);

        if (D < Lights[i].Radius) 
        {
            LightDir = normalize(LightDir);

            // //Correct for aspect ratio
            LightDir.x *= Resolution.x / Resolution.y;
        
            // //normalize our vectors
            vec3 N = normalize(NormalMap * 2.0 - 1.0);
            vec3 L = normalize(LightDir);

            vec3 HalfwayDir = normalize(LightDir + ViewDir);

            float spec = pow(max(dot(N, HalfwayDir), 0.0), 250.0);

            // Get light color
            vec4 LightColor = Lights[i].Color;

            vec3 Specular = DiffuseColor.a * LightColor.rgb * spec;

            // //Pre-multiply light color with intensity
            // //Then perform "N dot L" to determine our diffuse term
            vec3 Diffuse = LightColor.rgb * DiffuseColor.rgb * max(dot(N, L), 0.0);

            // //pre-multiply ambient color with intensity
            vec3 Ambient = AmbientColor.rgb * AmbientColor.a;

            vec3 Falloff = Lights[i].Falloff;

            // //calculate attenuation
            float Attenuation = 1.0 / (Falloff.x + (Falloff.y*D) + (Falloff.z*D*D));

            Diffuse *= Attenuation;
            Specular *= Attenuation;
            Lighting += Diffuse + Specular;
        }
    }

    // color = texture2D(u_diffuse, fs_in.TexCoords);
    color = vec4(Lighting, 1.0);
    // color = vec4(LightDir, 1.0);
}














