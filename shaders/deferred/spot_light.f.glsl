#version 330 core
in vec2 TexCoords;

layout (location = 0) out vec4 ColorOut;

const float kPi = 3.13159265;

out vec4 color;

uniform sampler2D u_albedoMap;
uniform sampler2D u_normalMap;
uniform sampler2D u_positionMap;

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
    float Distance = length(u_lightPos - WorldPos);

    vec3 LightDir = normalize(u_lightPos - WorldPos);
    float DiffuseTerm = max(dot(LightDir, Normal), 0.0);

    float Attenuation = 1.0 / (u_falloff.x + u_falloff.y * Distance + u_falloff.z * Distance * Distance);

    // Soft edges for spot light
    float Theta = dot(LightDir, normalize(-u_lightDirection));
    float Epsilon = u_innerCutoff - u_outerCutoff;
    float Intensity = clamp((Theta - u_outerCutoff) / Epsilon, 0.0, 1.0);

    vec4 DiffuseColor = DiffuseTerm * Diffuse * vec4(u_lightColor, 1.0) * u_lightIntensity;
    // Specular
    float specularLightWeighting = 0.0;
    vec3 Specular = vec3(0.0);
    if (DiffuseTerm > 0)
    {
        float Shininess = 10.0;
        float kEnergyConservation = (8.0 + Shininess) / (8.0 * kPi);
        vec3 ViewDir = normalize(u_camPos - WorldPos);
        vec3 ReflectDir = reflect(-LightDir, Normal);
        float Spec = kEnergyConservation * pow(max(dot(ViewDir, ReflectDir), 0.0), Shininess); 
        Specular = vec3(0.2) * Spec * u_lightColor;
    }


    ColorOut = (DiffuseColor + vec4(Specular, 1.0)) * Attenuation * Intensity;

}
