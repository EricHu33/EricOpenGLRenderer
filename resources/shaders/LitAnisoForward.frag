
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
in vec2 texCoord;
in vec3 positionWS;
in vec3 normalWS;
in vec3 normalVS;
in mat3 TBN;

uniform sampler2D _baseMap;
uniform sampler2D _specularMap;
uniform sampler2D _normalMap;
uniform samplerCube _skybox;
uniform sampler2D _occlusionRoughMetalMap;


uniform float _uvScale;
uniform float _roughness;
uniform float _metallic;
uniform float _normalStrength;
uniform vec3 _cameraPos;

uniform int _parallaxSteps;
uniform float _heightScale;
uniform float _UseParallaxMapping;

uniform bool _useOcclusionRoughMetalMap;
uniform bool _useColorOnly;
uniform vec4 _baseColor;

uniform vec3 _mainLightColor;
uniform vec3 _mainLightDirection;
uniform vec3 _mainLightAmbient;
uniform float _PreComputedLightIntensity;
uniform float _PreComputedEnvIntensity;
uniform float _bloomThreshold;
uniform mat4 _cameraViewMatrix;

uniform float _exposure;
uniform float _radius;

//PBR PARAMS
uniform float _anisotropy;

layout (std140) uniform Matrices
{
    //show base alignment and offset in comment
    mat4 _view;       //64 0
    mat4 _projection; //64 64
};

layout(std140) uniform SH9
{
    SH9Color sh9;
};

vec3 CalculatePointLight(PointLight pointLight, vec3 position, vec3 normal, vec3 albedo, vec3 viewDir, float roughness, float metallic, float anisotropy, vec3 compensation)
{
    vec3 lightDir = normalize(pointLight.position.xyz - position);
    float ndotl = min(max(dot(lightDir, normal), 0.0), 1.0);
    float atten = getDistanceAttenuation(pointLight.position.xyz - position, (1.0 / (_radius * _radius)));
    float illuminance = pointLight.color.a * _exposure * atten * ndotl;
    vec3 brdfColor = vec3(0,0,0);
    float ndotv = min(max(dot(normal, viewDir), 0.0), 1.0);
    brdfColor = AnisotropicBRDF(lightDir, normal, TBN[0], TBN[1], viewDir, roughness, metallic, anisotropy, albedo.rgb, compensation);
    // brdfColor = AnisotropicBRDF(_mainLightDirection, _normalWS, TBN[0], TBN[1], viewDir, roughness, metallic, anisotropy, albedo.rgb, vec3(1,1,1));
    return brdfColor * illuminance;
}

void main()
{
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint tileIndex1D = tileID.y * 90 + tileID.x;
    uint lightListOffestStart = tileIndex1D * 300;
    
    vec2 uv = texCoord * _uvScale;
    float depthOffset = 0;
    mat3 invTBN = transpose(TBN);
    float finalHeight = 0.0;
    vec2 offsetUV = vec2(0,0);
    if(_UseParallaxMapping > 0)
    {
        vec3 posTS = invTBN * positionWS;
        vec3 cameraPosTS = invTBN * _cameraPos;
        vec3 viewDirTS = normalize(cameraPosTS - posTS);
        float viewScale = abs(dot(vec3(0.0, 0.0, 1.0), viewDirTS));
        float distance = length(_cameraPos - positionWS);
        float minDistance = 3.0;
        float maxDistance = 15.0;
        float remapViewDistance01 = (distance - minDistance) / (maxDistance - minDistance);

        float stepScale =  1.0 - max(0, min(dot(normalize(_cameraPos - positionWS), normalize(normalWS)), 1));
        stepScale *= stepScale;
        float viewDistanceScale = (1.0 - max(0, min(remapViewDistance01, 1)));
        stepScale *= viewDistanceScale;

        float lod = 0;
        int maxSteps = _parallaxSteps;
        float steps =  ceil(mix(maxSteps * 0.2, float(maxSteps), stepScale ));
        offsetUV = Unity_Parallax_Mapping(lod, 0, int(steps), viewDirTS, uv,  _heightScale, finalHeight);
        uv += offsetUV;
    }

    vec4 albedo = texture(_baseMap, uv);
    albedo.rgb *= albedo.a;
    if(_useColorOnly)
    {
        albedo = _baseColor;
        albedo.rgb *= albedo.a;
    }

    float roughness = _roughness;
    float metallic = _metallic;
    float anisotropy = _anisotropy;
    if(_useOcclusionRoughMetalMap)
    {
        vec2 roughMetal = texture(_occlusionRoughMetalMap, uv).rg;
        roughness = roughMetal.x;
        metallic = roughMetal.y;
    }

    //normal mapping
    vec3 normalVal = texture(_normalMap, uv).rgb;
    normalVal = normalVal * 2.0 - 1.0;
    vec3 _normalWS = mix(TBN * (normalVal), normalize(normalWS), 1.0 - _normalStrength);
    _normalWS = normalize(_normalWS);
    //(1.0 - ShadowCalculation(positionWS - vec3(0, 0, 0), _normalWS, _mainLightDirection, offsetUV)
    vec3 finalColor = vec3(0,0,0);
    vec3 lightColor = _mainLightColor;

    vec3 diffuseIBL = (EvalSH9Irradiance(_normalWS, sh9) * Fd_Lambert()) * albedo.rgb * (1.0 - metallic);

    vec3 viewDir = normalize(_cameraPos - positionWS);
    vec3 R = reflect(-viewDir, _normalWS);

    float ndotl = min(max(dot(_mainLightDirection, _normalWS), 0.0), 1.0);
    float ndotv = max(dot(_normalWS, viewDir), 0.0);

    const float MAX_REFLECTION_LOD = 4;
    vec3 f0 = 0.16 * 0.5 * 0.5 * (1.0 - metallic) + albedo.rgb * metallic;
    vec3 prefilteredColor = textureLod(_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 indirectBRDF = texture(_brdfLUT, vec2(ndotv, roughness)).rg;
    vec3 specularIBL = prefilteredColor * mix(indirectBRDF.xxx, indirectBRDF.yyy, f0);
    vec3 compensation = 1.0 + f0 * (1.0 / indirectBRDF.y - 1.0);
    specularIBL *= compensation;
    
    vec3 brdfColor = vec3(0, 0, 0);
    brdfColor = AnisotropicBRDF(_mainLightDirection, _normalWS, TBN[0], TBN[1], viewDir, roughness, metallic, anisotropy, albedo.rgb, compensation);
    vec3 ambientLighting = diffuseIBL + specularIBL;
    ambientLighting *= _PreComputedEnvIntensity;

    float shadow = (1.0 - ShadowCalculation(positionWS, _normalWS, _mainLightDirection, vec2(0,0), _view));
    brdfColor *= ndotl * shadow * _PreComputedLightIntensity;

    finalColor = brdfColor + ambientLighting;

    if(_perTileLightCount[tileIndex1D] > 0)
    {
        int count = _perTileLightCount[tileIndex1D];
        for(int i = 0; i < count; i++)
        {
            int lightIndex = _lightIndexesList[lightListOffestStart + i];
            PointLight plight = _pointLights[uint(lightIndex)];
            vec3 brdfColor = CalculatePointLight(plight, positionWS, _normalWS, albedo.rgb, viewDir, roughness, metallic, anisotropy, compensation);
            brdfColor *= plight.color.rgb;
            finalColor += brdfColor;
        }
    }
    
    FragColor = vec4(finalColor.rgb, albedo.a);
    BrightColor = vec4(0,0,0, 1.0);
}