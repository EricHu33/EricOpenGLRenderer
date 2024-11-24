
layout (location = 0)out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D _gPosition;
uniform sampler2D _gNormal;
uniform sampler2D _gAlbedoSpec;
uniform sampler2D _gMaterialParam;
uniform sampler2D _SSAOTex;

layout (std140) uniform Matrices
{
    //show base alignment and offset in comment
    mat4 _view;       //64 0
    mat4 _projection; //64 64
};

uniform PointLight _pointLight;
uniform vec3 _cameraPos;
uniform vec3 _diffuseColor;
uniform float _bloomThreshold;
uniform bool _isPointLight;
uniform mat4 _cameraViewMatrix;
uniform float _radius;
uniform float _exposure;

void main()
{
    vec2 texCoord = gl_FragCoord.xy / vec2(1440.0, 960.0);
    float ssao = texture(_SSAOTex, texCoord).r;
    vec3 positionVS = texture(_gPosition, texCoord).rgb;
    vec3 positionWS = (inverse(_cameraViewMatrix) * vec4(positionVS, 1)).xyz;
    vec3 normalVS = texture(_gNormal, texCoord).rgb;
    vec3 normalWS = inverse(mat3(_cameraViewMatrix)) * normalVS;
    vec3 albedo = texture(_gAlbedoSpec, texCoord).rgb;
    vec4 matParams =  texture(_gMaterialParam, texCoord);
    float roughness = matParams.r;
    float metallic = matParams.g;
    bool isClearCoat = matParams.b > 0;
    float clearCoat = matParams.b;
    float clearCoatPerceptualRoughness = matParams.a;
    vec3 viewDir = normalize(_cameraPos - positionWS);
    vec3 lightDir = normalize(_pointLight.position.xyz - positionWS);
    vec3 finalColor = vec3(0,0,0);
    
    float ndotl = min(max(dot(lightDir, normalize(normalWS)), 0.0), 1.0);
    float atten = getDistanceAttenuation(_pointLight.position.xyz - positionWS, (1.0 / (_radius * _radius)));
    float illuminance = _pointLight.color.a * _exposure * atten * ndotl;
    vec3 brdfColor = vec3(0,0,0);
    vec2 indirectBRDF = texture(_brdfLUT, vec2(abs(dot(normalize(normalWS),viewDir)), roughness)).rg;
    vec3 f0 = 0.16 * 0.5 * 0.5 * (1.0 - metallic) + albedo.rgb * metallic;
    vec3 compensation = 1.0 + f0 * (1.0 / indirectBRDF.y - 1.0);
    if(isClearCoat)
    {
        brdfColor = ClearCoatBRDF(lightDir, normalWS, viewDir, roughness, clearCoat, clearCoatPerceptualRoughness, metallic, albedo.rgb, compensation);
    }
    else
    {
        brdfColor = StandardBRDF(lightDir, normalWS, viewDir, roughness, metallic, albedo.rgb, compensation);
    }
    brdfColor *= _pointLight.color.rgb;
    finalColor = brdfColor * illuminance;
    
    FragColor = vec4(finalColor, 1);
    float luminance = 0.2126 * finalColor.r + 0.7152 * finalColor.g + 0.0722 * finalColor.b;
    vec3 brightColor = vec3(0,0,0);
    if(luminance > _bloomThreshold)
    {
        brightColor = vec3(finalColor.rgb);
    }
    BrightColor = vec4(brightColor.rgb,1);
}