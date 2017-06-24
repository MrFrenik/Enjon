#version 330 core

layout (location = 0) out vec4 ColorOut;

const float PI = 3.14159265359;

in DATA
{
    vec2 TexCoords;
} fs_in;

// uniforms
uniform samplerCube uIrradianceMap;
uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uPositionMap;
uniform sampler2D uEmissiveMap;
uniform sampler2D uMaterialMap;

uniform vec2 uResolution;
uniform vec3 uCamPos;

float DistributionGGX(vec3 N, vec3 H, float Roughness);
float GeometrySchlickGGX(float NdotV, float Roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float Roughness);
vec3 FresnelSchlickRoughness(float CosTheta, vec3 F0, float Roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

vec2 CalculateTexCoord()
{
    return gl_FragCoord.xy / uResolution;
}

void main()
{
    vec2 TexCoords = CalculateTexCoord();

    // Get diffuse color
    vec3 albedo = texture(uAlbedoMap, TexCoords).rgb;

    vec4 emissive = texture2D(uEmissiveMap, TexCoords);
    
	// Get world position
    vec3 worldPos = texture(uPositionMap, TexCoords).xyz;
    
	// Obtain normal from normal map in range (world coords)
    vec3 N = normalize(texture(uNormalMap, TexCoords).xyz);

	// View vector
	vec3 V = normalize( uCamPos - worldPos ); 

	// Material Properties
	vec4 MaterialProps = texture( uMaterialMap, TexCoords );
	float metallic = MaterialProps.r;
	float roughness = MaterialProps.g;
	float ao = MaterialProps.b;
	
	// Ambient term
	 vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	vec3 kS = FresnelSchlick( max( dot( N, V ), 0.0 ), F0 );
	vec3 kD = 1.0 - kS;
	kD *= ( 1.0 - metallic );
	vec3 irradiance = texture( uIrradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo;
	vec3 ambient = (kD * diffuse);

    // Final color out
	ColorOut = vec4( ambient, 1.0 ) + vec4( max( vec3( 0.0, 0.0, 0.0 ), emissive.rgb ), 1.0 ); 
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------