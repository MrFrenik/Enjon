#version 330 core
in vec2 TexCoords;

layout (location = 0) out vec4 ColorOut;

const float kPi = 3.13159265;

uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_positionMap;
uniform sampler2D u_matProps;

uniform vec3    u_camPos;
uniform vec3    u_falloff;
uniform vec3    u_lightColor;
uniform vec3    u_lightPos;
uniform float   u_lightIntensity;
uniform vec3    u_lightDirection;
uniform vec2    u_resolution;
uniform float   u_innerCutoff;
uniform float   u_outerCutoff;

// Vertex information
in DATA
{
    vec2 TexCoords;
}fs_in;

vec2 CalculateTexCoord()
{
    return gl_FragCoord.xy / u_resolution;
}

float DistributionGGX(vec3 N, vec3 H, float Roughness);
float GeometrySchlickGGX(float NdotV, float Roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float Roughness);
vec3 FresnelSchlickRoughness(float CosTheta, vec3 F0, float Roughness);

void main()
{
    vec2 TexCoords = CalculateTexCoord();

    // Get diffuse color
    vec3 Albedo = texture(u_albedoMap, TexCoords).rgb;
    Albedo = vec3(pow(Albedo.r, 2.2), pow(Albedo.g, 2.2), pow(Albedo.b, 2.2));

    // Get world position
    vec3 WorldPos = texture(u_positionMap, TexCoords).xyz;

    // Get material properties
    vec4 MaterialProps = texture2D(u_matProps, TexCoords);

    // Roughness, Metallic, and AO
    float Metallic  = MaterialProps.r;
    float Roughness = MaterialProps.g;

    // Obtain normal from normal map in range (world coords)
    vec3 N = texture(u_normalMap, TexCoords).xyz;
    vec3 V = normalize(u_camPos - WorldPos);

    // Calculate radiance
    vec3 L = normalize(u_lightPos - WorldPos);
    vec3 H = normalize(V + L);

    vec3 F0 = vec3(0.04);
    F0      = mix(F0, Albedo, Metallic);
    vec3 F  = FresnelSchlickRoughness(max(dot(H, V), 0.0), F0, Roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - Metallic);

    // Reflectance Equation
    vec3 Lo = vec3(0.0);

    // Distance to light
    float Distance = length(u_lightPos - WorldPos);

    // Attenuation
    float Attenuation = 1.0 / (u_falloff.x + u_falloff.y * Distance + u_falloff.z * Distance * Distance);

    // Soft edges for spot light
    float Theta = dot(L, -normalize(u_lightDirection));
    float Epsilon = u_innerCutoff - u_outerCutoff;
    float Intensity = clamp((Theta - u_outerCutoff) / Epsilon, 0.0, 1.0);

    // Radiance
    vec3 Radiance = u_lightColor * Attenuation * u_lightIntensity;

    // Cook-Torrance BRDF
    float NDF   = DistributionGGX(N, H, Roughness);
    float G     = GeometrySmith(N, V, L, Roughness);

    vec3 Nominator = NDF * G * F; 
    float Denominator = 4 * max(dot(V, N), 0.0) * max(dot(L, N), 0.0) + 0.001;
    vec3 BRDF = Nominator / Denominator;

    // Add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);

    // Final color
    Lo += (kD * Albedo / kPi + BRDF) * Radiance * NdotL;

    ColorOut = vec4(max(Lo, vec3(0.0)), 1.0) * Intensity;
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = kPi * denom * denom;
    
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  








