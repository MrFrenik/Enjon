#version 330 core

layout (location = 0) out vec4 ColorOut;

in DATA
{
    vec2 TexCoords;
} fs_in;

// uniforms
uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_positionMap;
// uniform sampler2D u_shadowMap;

uniform vec3 u_camPos;
uniform vec3 u_lightColor;
uniform vec3 u_lightDirection;
uniform float u_lightIntensity;
uniform vec2 u_resolution;
uniform vec3 u_camForward;
// uniform mat4 u_lightSpaceMatrix;
// uniform vec2 u_shadowBias;

const float kPi = 3.13159265;

const vec2 PoissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

vec2 CalculateTexCoord()
{
    return gl_FragCoord.xy / u_resolution;
}

/*
float ShadowCalculation(vec4 FragPosLightSpace, float bias)
{
    // Perspective divide
    vec3 ProjCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;
    ProjCoords = ProjCoords * 0.5 + 0.5;

    // Early out
    if (ProjCoords.z > 1.0) return 0.0;

    // Get depth values
    float CurrentDepth = ProjCoords.z;

    // PCF
    float Shadow = 0.0;
    vec2 TexelSize = 1.0 / textureSize(u_shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float PCFDepth = texture2D(u_shadowMap, ProjCoords.xy + vec2(x, y) * TexelSize).r;
            Shadow += CurrentDepth - bias > PCFDepth ? 1.0 : 0.0;
        }
    }
    Shadow /= 9.0;

    return Shadow;
}
*/

void main()
{
    vec2 TexCoords = CalculateTexCoord();

  	// Obtain normal from normal map in range (world coords)
    vec3 Normal = texture(u_normalMap, TexCoords).xyz;

    // Get diffuse color
    vec4 Diffuse = texture(u_albedoMap, TexCoords);

    // Get world position
    vec3 WorldPos = texture(u_positionMap, TexCoords).xyz;

    // Diffuse
    vec3 LightDir = normalize(u_lightDirection);
    float NDotL = dot(LightDir, Normal);
    float Brightness = max(NDotL, 0.0);
    vec4 DiffuseColor = Brightness * Diffuse * vec4(u_lightColor, 1.0) * u_lightIntensity;

    // Specular
    float specularLightWeighting = 0.0;
    vec3 Specular = vec3(0.0);
    if (Brightness > 0)
    {
        float Shininess = 25.0;
        float kEnergyConservation = (8.0 + Shininess) / (8.0 * kPi);
        vec3 ViewDir = normalize(u_camPos - WorldPos + u_camForward);
        vec3 ReflectDir = reflect(-LightDir, Normal);
        float Spec = kEnergyConservation * pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess); 
        Specular = vec3(0.2) * Spec * u_lightColor;
    }

    // Shadow
    // float bias = max(u_shadowBias.y * (1.0 - dot(Normal, LightDir)), u_shadowBias.x);
    // vec4 FragPosLightSpace = u_lightSpaceMatrix * vec4(WorldPos, 1.0);
    // float Shadow = ShadowCalculation(FragPosLightSpace, bias);
    // float val = 1.0 - Shadow;
 

    ColorOut = DiffuseColor + vec4(Specular, 1.0);
    // ColorOut = (DiffuseColor + vec4(Specular, 1.0)) * vec4(val, val, val, 1.0);
}