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
uniform sampler2D ShadowMap;

uniform vec3 CamPos;
uniform vec3 LightColor;
uniform vec3 LightDirection;
uniform float LightIntensity;
uniform vec2 Resolution;
uniform vec3 CameraForward;
uniform mat4 LightSpaceMatrix;
uniform vec2 ShadowBias;

const float kPi = 3.13159265;

vec2 CalculateTexCoord()
{
    return gl_FragCoord.xy / Resolution;
}

float ShadowCalculation(vec4 FragPosLightSpace, float bias)
{
    // Perspective divide
    vec3 ProjCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    ProjCoords = ProjCoords * 0.5 + 0.5;

    // Get depth values
    float CurrentDepth = ProjCoords.z;

    // PCF
    float Shadow = 0.0;
    vec2 TexelSize = 1.0 / textureSize(ShadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float PCFDepth = texture2D(ShadowMap, ProjCoords.xy + vec2(x, y) * TexelSize).r;
            Shadow += CurrentDepth - bias > PCFDepth ? 1.0 : 0.0;
        }
    }
    Shadow /= 9.0;

    if (ProjCoords.z > 1.0) Shadow = 0.0;

    return Shadow;
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

    vec4 FragPosLightSpace = LightSpaceMatrix * vec4(WorldPos, 1.0);

    // Diffuse
    vec3 LightDir = normalize(LightDirection);
    float DiffuseTerm = max(dot(LightDir, Normal), 0.0);
    vec4 DiffuseColor = DiffuseTerm * Diffuse * vec4(LightColor, 1.0) * LightIntensity;

    float bias = max(ShadowBias.y * (1.0 - dot(Normal, LightDir)), ShadowBias.x);

    float Shadow = ShadowCalculation(FragPosLightSpace, bias);

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
 
    float val = 1.0 - Shadow;
  ColorOut = (DiffuseColor + vec4(Specular, 1.0)) * vec4(val, val, val, 1.0);
}