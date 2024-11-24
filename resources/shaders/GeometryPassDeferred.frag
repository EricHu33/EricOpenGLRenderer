
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec4 gMaterialParam;

in vec2 texCoord;
in vec3 positionWS;
in vec3 positionVS;
in vec3 normalWS;
in vec3 normalVS;
in mat3 TBN;
in mat3 viewMatrix;

//0
uniform sampler2D _baseMap;
//1
uniform sampler2D _specularMap;
//2
uniform sampler2D _normalMap;
//3
uniform samplerCube _skybox;
//4
//uniform sampler2DArray _shadowMap;
//5
//uniform sampler2D _heightMap
//9
uniform sampler2D _occlusionRoughMetalMap;

uniform float _uvScale;
uniform float _roughness;
uniform float _metallic;
uniform bool _isClearCoat;
uniform float _clearCoat;
uniform float _clearCoatRoughness;

uniform float _normalStrength;
uniform vec3 _cameraPos;

uniform int _parallaxSteps;
uniform float _heightScale;
uniform float _UseParallaxMapping;

uniform bool _useOcclusionRoughMetalMap;
uniform bool _useColorOnly;
uniform vec4 _baseColor;

void main()
{
    vec2 uv = texCoord * _uvScale;
    
    //parallax occlusion mapping
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
    
    vec3 normalVal = texture(_normalMap, uv).rgb;
    normalVal = normalVal * 2.0 - 1.0;
    
    //normalVS is from texture(_normalMap, uv), where _normalMap is not assign at all
    vec3 normal = normalVS;//mix(viewMatrix * TBN * (normalVal), normalVS, 1.0 - _normalStrength);
    
    gPosition = positionVS;
    gNormal = vec4(normalize(normal), 1);
    vec4 albedo = texture(_baseMap, uv).rgba;
    if(albedo.a < 0.5)
    {
       // discard;
    }
    if(_useColorOnly)
    {
        gAlbedoSpec.rgb = _baseColor.rgb;
    }
    else
    {
        gAlbedoSpec.rgb = _baseColor.rgb;
    }
    gAlbedoSpec.a = 1;
    float roughness = _roughness;
    float metallic = _metallic;
    float clearCoat = 0;
    float clearCoatRoughness = 0;
    if(_isClearCoat)
    {
        clearCoat = _clearCoat;
        clearCoatRoughness = _clearCoatRoughness;
    }
    if(_useOcclusionRoughMetalMap)
    {
        vec2 roughnessMetal = texture(_occlusionRoughMetalMap, uv).gb;
        roughness = roughnessMetal.x;
        metallic = roughnessMetal.y;
    }
    gMaterialParam = vec4(roughness, metallic, clearCoat, clearCoatRoughness);
}
