
layout (location = 0)out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
//in vec2 texCoord;
//in mat3 TBN;

uniform sampler2D _gPosition;
uniform sampler2D _gNormal;
uniform sampler2D _gAlbedoSpec;
uniform sampler2D _gMaterialParam;
uniform sampler2D _SSAOTex;

uniform vec3 _altShadowColor;
uniform vec3 _mainLightColor;
uniform vec3 _mainLightDirection;
uniform vec3 _mainLightAmbient;
uniform float _PreComputedLightIntensity;
uniform float _PreComputedEnvIntensity;
uniform vec3 _cameraPos;
uniform float _bloomThreshold;
uniform mat4 _cameraViewMatrix;

layout(std140) uniform SH9
{
    SH9Color sh9;
};

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
    vec3 finalColor = vec3(0,0,0);
    vec3 lightColor = _mainLightColor;

    //indirect diffuse
    vec3 diffuseIBL = (EvalSH9Irradiance(normalize(normalWS), sh9) * Fd_Lambert()) * albedo * (1.0 - metallic);
    
    float ndotl = min(max(dot(_mainLightDirection, normalWS), 0.0), 1.0);
    float ndotv = max(dot(normalWS, viewDir), 0.0);
    vec3 R = reflect(-viewDir, normalWS);
    vec3 f0 = 0.16 * 0.5 * 0.5 * (1.0 - metallic) + albedo.rgb * metallic;
    vec3 prefilteredColor = EvaluateSpecularIBL(R, roughness);
    vec2 indirectBRDF = texture(_brdfLUT, vec2(ndotv, roughness)).rg;
    
    //mobile : specularIBL = prefilteredColor * EnvBRDFApprox(f0, roughness, ndotv)
    vec3 compensation = 1.0 + f0 * (1.0 / indirectBRDF.y - 1.0);
    vec3 specularIBL = prefilteredColor *  mix(indirectBRDF.xxx, indirectBRDF.yyy, f0); //computeSpecOcclusion(ndotv, 1, roughness);
    specularIBL *= compensation;
    vec3 brdfColor = vec3(0, 0, 0);
    if(isClearCoat)
    {
        // clearCoat_NoV == shading_NoV if the clear coat layer doesn't have its own normal map
        float Fc = F_Schlick1(ndotv, 0.04, 1.0) * clearCoat;
        float ccAtten = 1.0 - Fc;
        diffuseIBL *= ccAtten;
        specularIBL *= (ccAtten * ccAtten);
        specularIBL += EvaluateSpecularIBL(R, clearCoatPerceptualRoughness) * Fc;
        brdfColor = ClearCoatBRDF(_mainLightDirection, normalWS, viewDir, roughness, clearCoat, clearCoatPerceptualRoughness, metallic, albedo.rgb, compensation);
    }
    else
    {
        brdfColor = StandardBRDF(_mainLightDirection, normalWS, viewDir, roughness, metallic, albedo.rgb, compensation);
    }
    
    vec3 ambientLighting = diffuseIBL + specularIBL;
    ambientLighting *= _PreComputedEnvIntensity;
    ambientLighting *= 1;
    
    float shadow = (1.0 - ShadowCalculation(positionWS, normalWS, _mainLightDirection, vec2(0,0), _cameraViewMatrix));
    vec3 shadowColor = vec3(0,0,0);
    vec3 sColor = mix(_altShadowColor, shadowColor, 1-shadow);
    brdfColor *= (ndotl * shadow * _PreComputedLightIntensity) + (1-shadow) * sColor;
    finalColor = brdfColor + ambientLighting;
    
    FragColor = vec4(finalColor, 1);
    float luminance = 0.2126 * finalColor.r + 0.7152 * finalColor.g + 0.0722 * finalColor.b;
    vec3 brightColor = vec3(0,0,0);
    if(luminance > _bloomThreshold)
    {
        brightColor = vec3(finalColor.rgb);
    }
    BrightColor = vec4(brightColor.rgb,1);
}