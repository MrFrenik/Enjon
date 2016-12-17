#version 330 core

layout (location = 0) out vec4 ColorOut;

in DATA
{
    vec2 TexCoords;
} fs_in;

// uniforms
uniform sampler2D DiffuseMap;
uniform sampler2D NormalMap;
uniform sampler2D PositionMap;
uniform sampler2D EmissiveMap;

uniform vec3 CamPos;
uniform vec3 LightColor;
uniform vec3 LightDirection;
uniform float LightIntensity;
uniform vec2 Resolution;
uniform vec3 CameraForward;

const float kPi = 3.13159265;

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
    vec4 Diffuse = texture(DiffuseMap, TexCoords);

    // Get world position
    vec3 WorldPos = texture(PositionMap, TexCoords).xyz;

    vec4 Ambient = vec4(0.1, 0.2, 0.5, 0.2) * Diffuse;

    vec4 Emissive = texture2D(EmissiveMap, TexCoords);

    // Diffuse
    vec3 LightDir = normalize(LightDirection);
    float DiffuseTerm = max(dot(LightDir, Normal), 0.0);
    vec4 DiffuseColor = DiffuseTerm * Diffuse * vec4(LightColor, 1.0) * LightIntensity + vec4(Ambient.rgb, 1.0) * Ambient.a;

    // Specular
    float specularLightWeighting = 0.0;
    vec3 Specular = vec3(0.0);
    if (DiffuseTerm > 0)
    {
        float Shininess = 100.0;
        float kEnergyConservation = (8.0 + Shininess) / (8.0 * kPi);
        vec3 ViewDir = normalize(CamPos - WorldPos + CameraForward);
        vec3 ReflectDir = reflect(-LightDir, Normal);
        float Spec = kEnergyConservation * pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess); 
        Specular = vec3(0.2) * Spec * LightColor;
    }

   ColorOut = DiffuseColor + vec4(Specular, 1.0) + Emissive;
}