#version 330 core
in vec2 TexCoords;

layout (location = 0) out vec4 ColorOut;

const float kPi = 3.13159265;

out vec4 color;

uniform sampler2D AlbedoMap;
uniform sampler2D NormalMap;
uniform sampler2D PositionMap;

uniform vec3 CamPos;
uniform vec3 Falloff;
uniform float Radius;
uniform vec3 LightColor;
uniform vec3 LightPos;
uniform float LightIntensity;
uniform vec2 Resolution;
uniform vec3 CameraForward;
uniform float num_levels;

// Vertex information
in DATA
{
    vec2 TexCoords;
}fs_in;

vec2 CalculateTexCoord()
{
    return gl_FragCoord.xy / Resolution;
}

void main()
{
    vec2 TexCoords = CalculateTexCoord();

    // Obtain normal from normal map in range (world coords)
    vec3 Normal = texture(NormalMap, TexCoords).xyz;

    // Get diffuse color
    vec4 Diffuse = texture(AlbedoMap, TexCoords);

    // Get world position
    vec3 WorldPos = texture(PositionMap, TexCoords).xyz;

    // Diffuse
    float Distance = length(LightPos - WorldPos);

    // Attenuation of light
    float Attenuation = 1.0 / (Falloff.x + Falloff.y * Distance + Falloff.z * Distance * Distance);

    vec3 LightDir = normalize(LightPos - WorldPos);
    float NDotL = dot(LightDir, Normal);
    float Brightness = max(NDotL, 0.0);

    float Level = floor(Brightness * num_levels);
    Brightness = Level / num_levels;

    vec4 DiffuseColor = Brightness * Diffuse * vec4(LightColor, 1.0) * LightIntensity;
    // Specular
    /*
    float specularLightWeighting = 0.0;
    vec3 Specular = vec3(0.0);
    if (DiffuseTerm > 0)
    {
        float Shininess = 10.0;
        float kEnergyConservation = (8.0 + Shininess) / (8.0 * kPi);
        vec3 ViewDir = normalize(CamPos - WorldPos + CameraForward);
        vec3 ReflectDir = reflect(-LightDir, Normal);
        float Spec = kEnergyConservation * pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess); 
        Specular = vec3(0.2) * Spec * LightColor;
    }
    */

    ColorOut = DiffuseColor * Attenuation;


    // ColorOut = (DiffuseColor + vec4(Specular, 1.0)) * Attenuation;
}
