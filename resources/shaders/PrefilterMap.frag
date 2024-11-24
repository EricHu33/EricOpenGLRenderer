
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube _envCubeMap;
uniform float _roughness;

void main()
{
    vec3 N = normalize(localPos);
    vec3 R = N;
    vec3 V = R;
    
    const uint SAMPLE_COUNT = 1024u;
    float totalWeight = 0.0;
    vec3 prefilterColor = vec3(0,0,0);
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H  = ImportanceSampleGGX(Xi, N, _roughness);
        
        // l = reflect(-v, h) = 2 * v•h * h - v;
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);
        
        float NdotL = max(dot(N, L), 0.0);
        
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);
        
        if(NdotL > 0.0)
        {
            float D   = D_GGX(NdotH, _roughness);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 
            // resolution of source cubemap (per face)
            float resolution = 512.0;
            
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            
            float mipLevel = _roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            prefilterColor += textureLod(_envCubeMap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilterColor = prefilterColor / totalWeight;
    
    FragColor = vec4(prefilterColor, 1.0);
}