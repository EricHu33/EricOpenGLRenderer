
out vec3 FragColor;
in vec2 texCoord;

/*float D_Charlie(float roughness, float NoH) {
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
    float invAlpha  = 1.0 / roughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}*/

float D_Charlie2(float roughness, float NoH) {
    // Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
    float invAlpha  = 1.0 / roughness;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
    return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}

vec3 hemisphereUniformSample2(vec2 Xi)
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

// Based on Karis 2014
vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
    // Sample in spherical coordinates
    float Phi = 2.0 * PI * Xi.x;
    float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float SinTheta = sqrt(1.0 - CosTheta * CosTheta);
    // Construct tangent space vector
    vec3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;

    // Tangent to world space
    vec3 UpVector = abs(N.z) < 0.999 ? vec3(0.,0.,1.0) : vec3(1.0,0.,0.);
    vec3 TangentX = normalize(cross(UpVector, N));
    vec3 TangentY = cross(N, TangentX);
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float VisibilityAshikhmin(float NoV, float NoL)
{
    return 1.0 / (4.0 * (NoL + NoV - NoL * NoV));
}

// Karis 2014
vec2 integrateBRDF( float NoV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NoV * NoV); // sin
    V.y = 0.0;
    V.z = NoV; // cos

    // N points straight upwards for this integration
    const vec3 N = vec3(0.0, 0.0, 1.0);

    float A = 0.0;
    float B = 0.0;
    float C = 0.0;

    const uint numSamples = 1024u;

    for (uint i = 0u; i < numSamples; i++) {
        vec2 Xi = Hammersley(i, numSamples);
        // Sample microfacet direction
        vec3 H = importanceSampleGGX(Xi, N, roughness);

        // Get the light direction
        vec3 L = 2.0 * dot(V, H) * H - V;

        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float VoH = saturate(dot(V, H));

        if(NoL > 0.0) {
            float V_pdf = V_SmithGGXCorrelated(NoV, NoL, roughness * roughness) * VoH * NoL / NoH;
            float Fc = pow(1.0 - VoH, 5.0);
            A += (Fc) * V_pdf;
            B +=  V_pdf;
        }
    }
    A = A * 4.0 / float(numSamples);
    B = B * 4.0 / float(numSamples);
    return vec2(A, B);
}


float IntegrateClothDG(float NdotV, float roughness)
{
    vec3 V;
    V.x = sqrt(1.0 - NdotV * NdotV);
    V.y = 0.0;
    V.z = NdotV;
    
    float C = 0.0;
    vec3 N = vec3(0.0, 0.0, 1.0);
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = hemisphereUniformSample(Xi);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NoL = saturate(dot(N, L));
        float NoH = saturate(dot(N, H));
        float VoH = saturate(dot(V, H));

        if(NoL > 0.0)
        {
            float clothD = D_Charlie2(roughness * roughness, NoH);
            float clothG = VisibilityAshikhmin(NdotV, NoL);
            C += clothD * clothG  * NoL * VoH;
        }
    }
    C = C * 4.0 * 2.0 * PI / float(SAMPLE_COUNT);
    return  C;
    
}

// ----------------------------------------------------------------------------
void main() 
{
    vec2 integratedBRDF = integrateBRDF(texCoord.x, texCoord.y);
    float integratedClothDG = IntegrateClothDG(texCoord.x, texCoord.y);
    FragColor = vec3(integratedBRDF, integratedClothDG);
}