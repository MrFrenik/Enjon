#version 330 core

layout (location = 0) out vec4 ColorOut;

const float PI = 3.14159265359;

in DATA
{
    vec2 TexCoords;
} fs_in;

// uniforms
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBRDFLUT;
uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uPositionMap;
uniform sampler2D uEmissiveMap;
uniform sampler2D uMaterialMap;
uniform sampler2D uSSAOMap;

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
    vec3 albedo = texture2D(uAlbedoMap, TexCoords).rgb;
    albedo = vec3(pow(albedo.r, 2.2), pow(albedo.g, 2.2), pow(albedo.b, 2.2));

    vec4 emissive = texture2D(uEmissiveMap, TexCoords);
    
	// Get world position
    vec3 worldPos = texture2D(uPositionMap, TexCoords).xyz;
    
	// Obtain normal from normal map in range (world coords)
    vec3 N = normalize(texture2D(uNormalMap, TexCoords).xyz); 

	// View vector
	vec3 V = normalize( uCamPos - worldPos ); 

	// Reflection vector
	 vec3 R = reflect(-V, N); 

	// Material Properties
	vec4 MaterialProps = texture2D( uMaterialMap, TexCoords );
	float metallic = MaterialProps.r;
	float roughness = clamp( MaterialProps.g * MaterialProps.g, 0.08, 0.9 );
	float specPower = 1.0 - metallic;
	//float a = clamp( roughness * pow(roughness, specPower), 0.08, 1.0 );
	float a = pow(roughness, 2.0);
	float ao = MaterialProps.b;

	float ssao = texture2D( uSSAOMap, TexCoords ).r;
	
	// F0 mix
	vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	
	// Get fresnel term
	vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, a);	

	// Ambient Diffuse term
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= ( 1.0 - metallic ); 

	// Irradiance
	vec3 irradiance = texture( uIrradianceMap, N).rgb;
	vec3 diffuse = irradiance * albedo * kD;

	// Ambient Specular
	const float MAX_REFLECTION_LOD = 5.0;
    vec3 prefilteredColor = textureLod(uPrefilterMap, R,  a * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(uBRDFLUT, vec2(max(dot(N, V), 0.0), a)).rg;
	vec3 brdfVec = vec3( F * brdf.x + brdf.y );
	vec3 specular = prefilteredColor * brdfVec;

	// Final ambient
	vec3 ambient = max( vec3( 0.0, 0.0, 0.0 ), (diffuse + specular) ) * ao * ssao;

    // Final color out
	ColorOut = vec4( ambient, 1.0 ) + vec4( max( vec3( 0.0, 0.0, 0.0 ), emissive.rgb ), 1.0 ); 
} 

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
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2; 
} 

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
} 

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 