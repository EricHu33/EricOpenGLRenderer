#define PI                 3.1415926
#define TWO_PI                 6.2831852
#define HALF_PI            1.5707963

#define MEDIUMP_FLT_MAX    65504.0
#define MEDIUMP_FLT_MIN    0.00006103515625
#define saturateMediump(x) min(x, MEDIUMP_FLT_MAX)

uniform samplerCube _irradianceMap;
//IBL
//7
uniform samplerCube _prefilterMap;
//8
uniform sampler2D   _brdfLUT;
const float PI_HIGH = 3.14159265359;

float saturate(float x) {
    return max(0, min(1, x));
}

vec2 saturate(vec2 v) {
    return vec2(saturate(v.x), saturate(v.y));
}

vec3 saturate(vec3 v) {
    return vec3(saturate(v.x), saturate(v.y), saturate(v.z));
}

vec4 saturate(vec4 v) {
    return vec4(saturate(v.x), saturate(v.y), saturate(v.z), saturate(v.w));
}

//Specular BRDF's D, G, F

//D
//Bruce Walter, 2007 Microfacet Models for Refraction through Rough Surfaces. Proceedings of the Eurographics Symposium on Rendering.
float D_GGX(float NoH, float roughness) {
    NoH = min(NoH, 0.9999);
    float a = NoH * roughness;
    float k = roughness / (1.0 - NoH * NoH + a * a);
    return k * k * (1.0 / PI);
}

//optimized D_GGX
float D_GGX(float roughness, float NoH, const vec3 n, const vec3 h) {
    vec3 NxH = cross(n, h);
    float a = NoH * roughness;
    float k = roughness / (dot(NxH, NxH) + a * a);
    float d = k * k * (1.0 / PI);
    return saturateMediump(d);
}

//G
//Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3 (2). 
float V_SmithGGXCorrelated(float NoV, float NoL, float roughness) {
    float a2 = max(roughness * roughness, 1e-5);
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

//optimized G_GGX
float V_SmithGGXCorrelatedFast(float NoV, float NoL, float roughness) {
    float a = max(roughness, 1e-5);
    float GGXV = NoL * (NoV * (1.0 - a) + a);
    float GGXL = NoV * (NoL * (1.0 - a) + a);
    return 0.5 / (GGXV + GGXL);
}

//unreal F_Schlick
vec3 F_Schlick( vec3 SpecularColor, float VoH )
{
    float Fc = pow( 1 - VoH, 5);                 // 1 sub, 3 mul
    //return Fc + (1 - Fc) * SpecularColor;     // 1 add, 3 mad

    // Anything less than 2% is physically impossible and is instead considered to be shadowing
    return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
}

//F
//Christophe Schlick. 1994. An Inexpensive BRDF Model for Physically-Based Rendering. Computer Graphics Forum, 13 (3), 233–246.
vec3 F_SchlickUE(float VoH, vec3 specularColor) {
    float f = pow(1.0 - VoH, 5.0);
    return f + specularColor * (1.0 - f);
}
// Fresnel. f0 = reflectance of material at normal incidence. f90 = same thing, but 90 degrees (grazing) incidence

// TODO: use better Fresnel term
float F_Schlick1(float u, float f0, float f90)
{
  return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

vec3 F_Schlick3(float u, vec3 f0, float f90)
{
  return f0 + (vec3(f90) - f0) * pow(1.0 - u, 5.0);
}

float computeSpecOcclusion( float NdotV , float AO , float roughness )
{
    return saturate(pow( NdotV + AO , exp2 ( -16.0f * roughness - 1.0f )) - 1.0f + AO );
}



//Diffuse BRDF

float Fd_Lambert() {
    return 1.0 / PI;
}

vec3 BRDFDiffuse(vec3 diffuseColor) {
    
    return diffuseColor * Fd_Lambert();
}

// DFG for clear coat
// D & F term for standard BRDF is cheap enough, so we use keep it.
// G use experience model => 1 / 4 * (LoH)^2

float V_Kelemen(float LoH) {
    return 0.25 / clamp((LoH * LoH), 1e-5, 1.0);
}

//perceptualRoughness == user input 0 to 1
vec3 EvaluateSpecularIBL(vec3 reflectDir, float perceptualRoughness)
{
    const float MAX_REFLECTION_LOD = 4;
    vec3 prefilteredColor = textureLod(_prefilterMap, reflectDir, perceptualRoughness * MAX_REFLECTION_LOD).rgb;
    return prefilteredColor;
}

vec3 ClearCoatBRDF(vec3 lightDir, vec3 normal, vec3 viewDir, float perceptualRoughness, float clearCoat, float clearCoatPerceptualRoughness, float metallic, vec3 baseColor, vec3 compensation)
{
    lightDir = normalize(lightDir);

    float reflectanceLinear = 0.5;
    //set default f0/reflectance to 0.04 
    float ccPerceptualRoughness = clamp(clearCoatPerceptualRoughness, 0.01, 1.0);
    float clearCoatRoughness = ccPerceptualRoughness * ccPerceptualRoughness;
    
    vec3 f0 = 0.16 * reflectanceLinear * reflectanceLinear * (1.0 - metallic) + baseColor * metallic;
    vec3 h = normalize(viewDir + lightDir);
    float NoV = abs(dot(normal, viewDir)) + 1e-5;
    float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float NoH = clamp(dot(normal, h), 0.0, 1.0);
    float LoH = clamp(dot(lightDir, h), 0.0, 1.0);

    float roughness = perceptualRoughness * perceptualRoughness;
    roughness = max(0.005, roughness);

    //both standard & clear coat use same NDF term
    
    // -- clear coat BRDF
    float clearCoatD = D_GGX(clearCoatRoughness, NoH, normal, h);
    float clearCoatF0 = 0.04;
    float clearCoatF = F_Schlick1(LoH, clearCoatF0, 1.0) * clearCoat;
    float clearCoatV = V_Kelemen(LoH);
    vec3 BRDFClearCoat = clearCoatD * clearCoatV * clearCoatF * vec3(1, 1, 1);
    
    // -- standard BRDF start
    float D = D_GGX(roughness, NoH, normal, h);
    vec3  F = F_Schlick3(LoH, f0, 1.0);
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness); //  G term of DFG
    vec3 Fr = (D * V) * F * compensation;

    vec3 Fd = BRDFDiffuse(baseColor);
    
    //Fc = clearCoatF
    //fc = BRDFClearCoat
    
    //f(v,l) = Fd(v,l) (1−Fc) + Fr(v,l)(1−Fc) + fc(v,l)
    return Fd * (1.0 - metallic) * (1 - clearCoatF) + Fr * (1 - clearCoatF) + BRDFClearCoat;
}

//float at = max(roughness * (1.0 + anisotropy), 0.001);
//float ab = max(roughness * (1.0 - anisotropy), 0.001);
//Anisotropic NDF
float D_GGX_Anisotropic(float NoH, const vec3 h,
const vec3 t, const vec3 b, float at, float ab) {
    float ToH = dot(t, h);
    float BoH = dot(b, h);
    float a2 = at * ab;
    highp vec3 v = vec3(ab * ToH, at * BoH, a2 * NoH);
    highp float v2 = dot(v, v);
    float w2 = a2 / v2;
    return a2 * w2 * w2 * (1.0 / PI);
}

//G for Anisotropic
float V_SmithGGXCorrelated_Anisotropic(float at, float ab, float ToV, float BoV,
                                       float ToL, float BoL, float NoV, float NoL) {
    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    float v = 0.5 / max((lambdaV + lambdaL), 1e-5);
    return saturateMediump(v);
}

vec3 AnisotropicBRDF(vec3 lightDir, vec3 normal, vec3 tangent, vec3 bitangent, vec3 viewDir, float perceptualRoughness, float metallic, float anisotropy, vec3 baseColor, vec3 compensation)
{
    lightDir = normalize(lightDir);

    float reflectanceLinear = 0.5;
    //set default f0/reflectance to 0.04 
    vec3 f0 = 0.16 * reflectanceLinear * reflectanceLinear * (1.0 - metallic) + baseColor * metallic;
    vec3 h = normalize(viewDir + lightDir);
    float NoV = abs(dot(normal, viewDir)) + 1e-5;
    float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float NoH = clamp(dot(normal, h), 0.0, 1.0);
    float LoH = clamp(dot(lightDir, h), 0.0, 1.0);
    float ToV = clamp(dot(tangent, viewDir), 0.0, 1.0);
    float BoV = clamp(dot(bitangent, viewDir), 0.0, 1.0);
    float ToL = clamp(dot(tangent, lightDir), 0.0, 1.0);
    float BoL = clamp(dot(bitangent, lightDir), 0.0, 1.0);

    float roughness = perceptualRoughness * perceptualRoughness;
    
    float roughnessA = max(roughness * (1.0 + anisotropy), 0.001);
    float roughnessB = max(roughness * (1.0 - anisotropy), 0.001);
    
    roughness = max(0.005, roughness);
    //specular BRDF start
    float D = D_GGX_Anisotropic(NoH, h, tangent, bitangent, roughnessA, roughnessB);
    vec3  F = F_Schlick3(LoH, f0, 1.0);
    //G term of DFG
    float V = V_SmithGGXCorrelated_Anisotropic(roughnessA, roughnessB, ToV, BoV, ToL, BoL, NoV, NoL);
    vec3 Fr = (D * V) * F * compensation;
    //specular BRDF end

    vec3 Fd = BRDFDiffuse(baseColor);
    return Fr + Fd * (1.0 - metallic);
}

//NDF for cloth 
float D_Charlie(float roughness, float NoH) {
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
    float invAlpha  = 1.0 / roughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

float Fd_Wrap(float NoL, float w) {
    return saturate((NoL + w) / (1.0 + w) * (1.0 + w));
}

vec3 ClothBRDF(vec3 lightDir, vec3 normal, vec3 viewDir, float perceptualRoughness, float metallic, vec3 baseColor, vec3 sheenColor, bool useSSS, vec3 SSSColor)
{
    lightDir = normalize(lightDir);

    float reflectanceLinear = 0.5;
    //set default f0/reflectance to 0.04 
    // TODO f0 can be sheen color
    vec3 f0 = vec3(0.04, 0.04, 0.04);
    vec3 h = normalize(viewDir + lightDir);
    float NoV = abs(dot(normal, viewDir)) + 1e-5;
    float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float NoH = clamp(dot(normal, h), 0.0, 1.0);
    float LoH = clamp(dot(lightDir, h), 0.0, 1.0);

    float roughness = perceptualRoughness * perceptualRoughness;
    roughness = max(0.005, roughness);
    //specular BRDF start
    float D = D_Charlie(roughness, NoH);
    //F_Schlick(LoH, sheenColor);
    vec3  F = F_SchlickUE(LoH, sheenColor);
    // 1.0 / 4.0 * (NoL + NoV - (NoL * NoV) == V
    vec3 Fr = (D * F) / (4 * (NoL + NoV - (NoL * NoV)));
    //specular BRDF end
     
    //vec3 Fd = (vec3(1,1,1)-F) * BRDFDiffuse(baseColor);
    vec3 Fd = BRDFDiffuse(baseColor);
    vec3 brdf = (Fd + Fr) * NoL;
    if(useSSS)
    {
        Fd *= Fd_Wrap(dot(normal, lightDir), 0.5);
       // Fd *= saturate(SSSColor + vec3(NoL, NoL,NoL));
        //will be cancel out outside when (fd+fr) * ndotl, so output will match (fd + fr * ndotl) for sss
        //instead of (fd + fr) * ndotl
        brdf = Fd + Fr * NoL;
    }
    return brdf;
}

vec3 SkinBRDF(vec3 lightDir, vec3 normal, vec3 viewDir, float perceptualRoughness, float metallic, vec3 baseColor, vec3 compensation, vec3 SSSdiffuse)
{
    lightDir = normalize(lightDir);

    float reflectanceLinear = 0.5;
    //set default f0/reflectance to 0.04 
    vec3 f0 = 0.028 * (1.0 - metallic) + baseColor * metallic;
    vec3 h = normalize(viewDir + lightDir);
    float NoV = abs(dot(normal, viewDir)) + 1e-5;
    float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float NoH = clamp(dot(normal, h), 0.0, 1.0);
    float LoH = clamp(dot(lightDir, h), 0.0, 1.0);

    float roughness = perceptualRoughness * perceptualRoughness;
    roughness = max(0.005, roughness);
    //specular BRDF start
    float D = D_GGX(roughness, NoH, normal, h);
    vec3  F = F_Schlick3(LoH, f0, 1.0);
    //G term of DFG
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    vec3 Fr = (D * V) * F * compensation;
    //specular BRDF end
    
    return Fr;
}

//NdotL = saturate(dot(normalWS, lightDirectionWS));
//half3 radiance = lightColor * (lightAttenuation * NdotL);
//finalColor = StandardBRDF()  * radiance
vec3 StandardBRDF(vec3 lightDir, vec3 normal, vec3 viewDir, float perceptualRoughness, float metallic, vec3 baseColor, vec3 compensation) 
{
    lightDir = normalize(lightDir);
    
    float reflectanceLinear = 0.5;
    //set default f0/reflectance to 0.04 
    vec3 f0 = 0.16 * reflectanceLinear * reflectanceLinear * (1.0 - metallic) + baseColor * metallic;
    vec3 h = normalize(viewDir + lightDir);
    float NoV = abs(dot(normal, viewDir)) + 1e-5;
    float NoL = clamp(dot(normal, lightDir), 0.0, 1.0);
    float NoH = clamp(dot(normal, h), 0.0, 1.0);
    float LoH = clamp(dot(lightDir, h), 0.0, 1.0);
    
    float roughness = perceptualRoughness * perceptualRoughness;
    roughness = max(0.005, roughness);
    //specular BRDF start
    float D = D_GGX(roughness, NoH, normal, h);
    vec3  F = F_Schlick3(LoH, f0, 1.0);
    //G term of DFG
    float V = V_SmithGGXCorrelated(NoV, NoL, roughness);
    vec3 Fr = (D * V) * F * compensation;
    //specular BRDF end
    
    vec3 Fd = BRDFDiffuse(baseColor);
    return Fr + Fd * (1.0 - metallic);
}

vec3 EnvDFGLazarov( vec3 specularColor, float gloss, float ndotv )
{
    vec4 p0 = vec4( 0.5745, 1.548, -0.02397, 1.301 );
    vec4 p1 = vec4( 0.5753, -0.2511, -0.02066, 0.4755 );

    vec4 t = gloss * p0 + p1;

    float bias = saturate( t.x * min( t.y, exp2( -7.672 * ndotv ) ) + t.z );
    float delta = saturate( t.w );
    float scale = delta - bias;

    bias *= saturate( 50.0 * specularColor.y );
    return specularColor * scale + bias;
}

vec3 EnvBRDFApprox( vec3 SpecularColor, float Roughness, float NoV )
{
    const vec4 c0 = vec4( -1, -0.0275, -0.572, 0.022 );
    const vec4 c1 = vec4( 1, 0.0425, 1.04, -0.04 );
    vec4 r = Roughness * c0 + c1;
    float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
    vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
    return SpecularColor * AB.x + AB.y;
}

vec3 EnvDFGPolynomial( vec3 specularColor, float gloss, float ndotv )
{
    float x = gloss;
    float y = ndotv;

    float b1 = -0.1688;
    float b2 = 1.895;
    float b3 = 0.9903;
    float b4 = -4.853;
    float b5 = 8.404;
    float b6 = -5.069;
    float bias =  saturate( min( b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y ));

    float d0 = 0.6045;
    float d1 = 1.699;
    float d2 = -0.5228;
    float d3 = -3.603;
    float d4 = 1.404;
    float d5 = 0.1939;
    float d6 = 2.661;
    float delta =  saturate( d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x );
    float scale = delta - bias;

    bias *= saturate( 50.0 * specularColor.y );
    return specularColor * scale + bias;
}

//Specular IBL 

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

//for cloth BRDF lut
vec3 hemisphereUniformSample(vec2 Xi)
{
    float phi = 2.0 * PI_HIGH * Xi.x;
    float cosTheta = 1.0 - Xi.y;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;
    return H;
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float phi = 2.0 * PI_HIGH * Xi.x;
    float a = roughness * roughness;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) *  sinTheta;
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;
    
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
    
    vec3 sampleVecWS = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVecWS);
}


