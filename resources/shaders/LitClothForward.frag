
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
uniform sampler2D _threadMap; //R = AO, G, A = normal yx, B = 1 - roughness
uniform sampler2D _fuzzMap;

uniform SpotLight _spotLight;

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
uniform vec4 _sheenColor;
uniform bool _useSSS;
uniform vec4 _sssColor;

uniform bool _useFuzzMap;
uniform float _fuzzScale;
uniform float _fuzzStrength;

uniform float _useThreadMap;
uniform float _threadScale;
uniform float _threadStrength;
uniform float _threadAO;

uniform vec3 _DoubleSidedConstants = vec3(1 ,1, -1);


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

vec3 UnpackNormalmap(vec4 packednormal)
{
    // This do the trick
    packednormal.x *= packednormal.w;

    vec3 normal = vec3(0,0,0);
    normal.xy = packednormal.xy * 2 - 1;
    normal.z = sqrt(1 - saturate(dot(normal.xy, normal.xy)));
    return normal;
}

vec3 normalStrength(vec3 In, float Strength)
{
    return vec3(In.rg * Strength, mix(1, In.b, saturate(Strength)));
}

vec3 normalBlend(vec3 A, vec3 B)
{
    return normalize(vec3(A.rg + B.rg, A.b * B.b));
}

vec3 CalculatePointLight(PointLight pointLight, vec3 position, vec3 normal, vec3 albedo, vec3 viewDir, float roughness, float metallic)
{
    vec3 lightDir = normalize(pointLight.position.xyz - position);
    float ndotl = min(max(dot(lightDir, normal), 0.0), 1.0);
    float atten = getDistanceAttenuation(pointLight.position.xyz - position, (1.0 / (_radius * _radius)));
    float illuminance = pointLight.color.a * _exposure * atten;
    vec3 brdfColor = vec3(0,0,0);
    float ndotv = min(max(dot(normal, viewDir), 0.0), 1.0);
    brdfColor = ClothBRDF(lightDir, normal, viewDir, roughness, metallic, albedo.rgb, _sheenColor.rgb, _useSSS, _sssColor.rgb);
    return brdfColor * illuminance;
}

void main()
{
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint tileIndex1D = tileID.y * 90 + tileID.x;
    uint lightListOffestStart = tileIndex1D * 300;
    
    mat3 tangentToWorld = TBN;
    vec2 uv = texCoord * _uvScale;
    vec3 srcNoramlWS = normalWS;
    if(!gl_FrontFacing)
    {
        tangentToWorld[2] = -1 *  tangentToWorld[2];
        srcNoramlWS = normalWS + 2 * tangentToWorld[2] * max(0, -dot(tangentToWorld[2], normalWS));
    }
    srcNoramlWS = normalize(srcNoramlWS);
    
    float depthOffset = 0;
    mat3 invTBN = transpose(tangentToWorld);
    float finalHeight = 0.0;
    vec2 offsetUV = vec2(0,0);
    

    vec4 albedo = texture(_baseMap, uv);
    albedo.rgb *= albedo.a;
    
    if(_useColorOnly)
    {
        albedo = _baseColor;
        albedo.rgb *= albedo.a;
    }
    
    vec4 threadMapVal = texture(_threadMap, uv * _threadScale);
    
    vec3 threadNormal = vec3(threadMapVal.wy, 1);
    vec3 threadNormalTS = UnpackNormalmap( vec4(threadNormal,1) );
    threadNormalTS = normalStrength(normalize(threadNormalTS), _threadStrength);
    vec3 noramalTS = invTBN * srcNoramlWS;
    vec3 blendedNormalTS = normalize(normalBlend(noramalTS, threadNormalTS));
    noramalTS = mix(noramalTS, blendedNormalTS, _useThreadMap);
    srcNoramlWS = tangentToWorld * noramalTS;

    float ao = mix(1.0, mix(1.0, threadMapVal.r , _threadAO), _useThreadMap);
    
    //apply fuzz Map 
    if(_useFuzzMap)
    {
        float fuzz = texture(_fuzzMap, uv * _fuzzScale).r;
        albedo += fuzz * _fuzzStrength * 5;
        albedo = saturate(albedo);
        albedo.rgb *= ao;
    }
    float threadRoughness = mix(0.0, 1 - (2.0 * threadMapVal.b - 1.0) * 2.0, _useThreadMap);
    threadRoughness = saturate(threadRoughness);
    float roughness = _roughness - threadRoughness;
    roughness = saturate(roughness);
    float metallic = _metallic;
    if(_useOcclusionRoughMetalMap)
    {
        vec2 roughMetal = texture(_occlusionRoughMetalMap, uv).rg;
        roughness = roughMetal.x;
        metallic = roughMetal.y;
    }

    //normal mapping
    vec3 normalVal = texture(_normalMap, uv).rgb;
    normalVal = normalVal * 2.0 - 1.0;
    vec3 _normalWS = mix(tangentToWorld * (normalVal), srcNoramlWS, 1.0 - _normalStrength);
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
    float f0 = 0.16 * 0.5 * 0.5;
    vec3 prefilteredColor = textureLod(_prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
    float indirectDG = texture(_brdfLUT, vec2(ndotv, roughness)).b;
    vec3 specularIBL = prefilteredColor * indirectDG  * _sheenColor.rgb;

    vec3 brdfColor = vec3(0, 0, 0);
    brdfColor = ClothBRDF(_mainLightDirection, _normalWS, viewDir, roughness, metallic, albedo.rgb, _sheenColor.rgb, _useSSS, _sssColor.rgb);
    vec3 ambientLighting = diffuseIBL + specularIBL;
    ambientLighting *= _PreComputedEnvIntensity;

    float shadow = (1.0 - ShadowCalculation(positionWS, _normalWS, _mainLightDirection, vec2(0,0), _view));
    //ndotl already precalculated inside brdf
    brdfColor *= shadow * _PreComputedLightIntensity;
    
    finalColor = brdfColor + ambientLighting;
    finalColor *= ao;

    if(_perTileLightCount[tileIndex1D] > 0)
    {
        int count = _perTileLightCount[tileIndex1D];
        for(int i = 0; i < count; i++)
        {
            int lightIndex = _lightIndexesList[lightListOffestStart + i];
            PointLight plight = _pointLights[uint(lightIndex)];
            vec3 brdfColor =  CalculatePointLight(plight, positionWS, _normalWS, albedo.rgb, viewDir, roughness, metallic);
            brdfColor *= plight.color.rgb;
            finalColor += brdfColor;
        }
    }
    
    FragColor = vec4(finalColor.rgb, albedo.a);
    BrightColor = vec4(0,0,0, 1.0);
}