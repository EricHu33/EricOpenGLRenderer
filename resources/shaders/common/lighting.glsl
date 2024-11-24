struct PointLight {
    vec4 position; // position.w is radius
    vec4 color; // color.w is intensity
};

struct SpotLight {
    vec3 position;
    vec3 front;
    vec3 color;
    vec3 ambient;
    float innerCutOff;
    float outerCutOff;
};

layout(std430, binding = 5) buffer LightsList
{
    PointLight _pointLights[];
};
layout(std430, binding = 6) buffer LightIndexesList
{
    int _lightIndexesList[];
};
layout(std430, binding = 7) buffer TileLightCounts
{
    int _perTileLightCount[];
};

float getSquareFalloffAttenuation(float distanceSquare, float falloff) {
    float factor = distanceSquare * falloff;
    float smoothFactor = max(0, min(1, 1.0 - factor * factor));
    // We would normally divide by the square distance here
    // but we do it at the call site
    return smoothFactor * smoothFactor;
}

float getDistanceAttenuation(vec3 posToLight, float falloff) {
    float distanceSquare = dot(posToLight, posToLight);
    float attenuation = getSquareFalloffAttenuation(distanceSquare, falloff);
    return attenuation / max(distanceSquare, 1e-4);
}

vec3 CalculateSpotLight(SpotLight light, vec3 positionWS, vec2 uv, vec3 normal, vec3 albedo, vec3 viewDir, float shininess, vec3 baseColor, vec3 specColor);
float CalculateLightAttenuation(vec3 lightPosition, vec3 positionWS, float constant, float linear, float quadratic);
float CalculateSpotLightAttenuation(vec3 lightDir, SpotLight light);

vec3 CalculateSpotLight(SpotLight light, vec3 positionWS, vec2 uv, vec3 normal, vec3 albedo, vec3 viewDir, float shininess, vec3 baseColor, vec3 specColor)
{
    vec3 spotLightDir = normalize(light.position - positionWS);
    float theta = dot(spotLightDir, normalize(-light.front));

    float lightAtten = CalculateLightAttenuation(light.position, positionWS,  1.0f, 0.35f, 0.44f);
    float intensity = CalculateSpotLightAttenuation(spotLightDir, light);
    float ndotl = max(dot(spotLightDir, normal), 0.0);
    vec3 diffuse = (ndotl * baseColor) * light.color * albedo.rgb * lightAtten * intensity;

    vec3 reflectDir = reflect(-spotLightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shininess, 0.0));
    vec3 specular = spec * light.color * specColor * lightAtten * intensity;
    vec3 finalColor = (diffuse + specular);
    return finalColor;
}

float CalculateLightAttenuation(vec3 lightPosition, vec3 positionWS, float constant, float linear, float quadratic)
{
    float distance = length(lightPosition - positionWS);
    return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

float CalculateSpotLightAttenuation(vec3 lightDir,SpotLight light)
{
    float theta = dot(lightDir, normalize(-light.front));
    float epsilon   = (light.innerCutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    return intensity;
}