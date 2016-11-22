#version 330 core

const float kPi = 3.13159265;

layout (location = 0) out vec4 DiffuseOut;     // Diffuse
layout (location = 1) out vec4 NormalsOut;
layout (location = 2) out vec4 EmissiveOut;

in VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
	vec3 TangentLightPos;
    vec3 TangentLightDir; 
	vec3 TangentViewPos;
	vec3 TangentFragPos;
    vec3 TangentViewDir;
} fs_in;

// uniforms
uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform vec3 camPos;
uniform vec4 fogColor;
uniform float fogMin;
uniform float fogMax;
uniform vec3 LightColor;
uniform float Shininess;

// Math for Game Devs @ youtube
float RemapValClamped(float flInput, float flInLo, float flInHi, float flOutLo, float flOutHi)
{
    if (flInput < flInLo)
        return flOutLo;
    if (flInput > flInHi)
        return flOutHi;
    return (((flInput-flInLo) / (flInHi-flInLo)) * (flOutHi-flOutLo)) + flOutLo;
}

void main()
{
	// Obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;

    // Transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

    // Get diffuse color
    vec4 c = texture(diffuseMap, fs_in.TexCoords);
    if (c.a < 0.5) discard;
    vec3 color = c.rgb;

    // Ambient
    vec3 ambient = 0.1 * color;

    // Diffuse
    // vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    vec3 lightDir = normalize(fs_in.TangentLightDir);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color * LightColor;

    // Specular
    float specularLightWeighting = 0.0;
    vec3 specular = vec3(0.0);
    if (diff > 0)
    {
        float shininess = Shininess;
        shininess = texture2D(specularMap, fs_in.TexCoords).r;

        if (shininess < 255.0)
        {
            float kEnergyConservation = (8.0 + Shininess) / (8.0 * kPi);
            vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos + fs_in.TangentViewDir);
            // viewDir = normalize(viewDir + fs_in.TangentViewDir); 
            // vec3 viewDir = normalize(fs_in.TangentFragPos - fs_in.TangentViewPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = kEnergyConservation * pow(max(dot(viewDir, reflectDir), 0.0), Shininess);
            // float spec = kEnergyConservation * pow(max(dot(halfwayDir, viewDir), 0.0), Shininess);
            specular = vec3(0.2) * spec;
        }
    }

    // Fog
    float fogDistance = length(camPos - fs_in.FragPos);
    float fog = RemapValClamped(fogDistance, fogMin, fogMax, 0.0, 1.0);
    
    DiffuseOut = vec4(color, 1.0);
    EmissiveOut = (vec4(ambient + diffuse + specular, 1.0f) * (1 - fog) + fogColor * fog);
    NormalsOut = vec4(normal, 1.0);
}