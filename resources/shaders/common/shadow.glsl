﻿
uniform float _normalBias;
uniform float _depthBias;
uniform float _pcssSearchRadius;
uniform float _pcssFilterRadius;
uniform int _cascadeCount;
uniform float _farPlane;
uniform float _nearPlane;
uniform float _lightWorldSize;

//4
uniform sampler2DArray _shadowMap;

layout(std140, binding = 1) uniform ShadowMatrices
{
    mat4 _shadowMatrix[5];
};

layout(std140, binding = 4) uniform ShadowCSMParam
{
    float _cascadePlaneDistances[4];
    float _csmFrustumSizes[4];
};

layout(std140, binding = 5) uniform RSMMatrices
{
    mat4 _rsmMatrix[1];
};

vec2 POISSON8[] = vec2[8](
    vec2(-0.2602728,0.3234085), vec2(-0.3268174,0.0442592), vec2(0.1996002,0.1386711),
    vec2(0.2615348,-0.1569698), vec2(-0.2869459,-0.3421305), vec2(0.1351001,-0.4352284),
    vec2(-0.0635913,-0.1520724), vec2(0.1454225,0.4629610)
);

vec2 POISSON16[] = vec2[16](
    vec2(0.3040781,-0.1861200), vec2(0.1485699,-0.0405212), vec2(0.4016555,0.1252352),
    vec2(-0.1526961,-0.1404687), vec2(0.3480717,0.3260515), vec2(0.0584860,-0.3266001),
    vec2(0.0891062,0.2332856), vec2(-0.3487481,-0.0159209), vec2(-0.1847383,0.1410431),
    vec2(0.4678784,-0.0888323), vec2(0.1134236,0.4119219), vec2(0.2856628,-0.3658066),
    vec2(-0.1765543,0.3937907), vec2(-0.0238326,0.0518298), vec2(-0.2949835,-0.3029899),
    vec2(-0.4593541,0.1720255)
);

vec2 POISSON32[] = vec2[32](
    vec2(0.2981409,0.0490049), vec2(0.1629048,-0.1408463), vec2(0.1691782,-0.3703386),
    vec2(0.3708196,0.2632940), vec2(-0.0602839,-0.2213077), vec2(0.3062163,-0.1364151),
    vec2(0.0094440,-0.0299901), vec2(-0.0753952,-0.3944479), vec2(-0.2073224,-0.3717136),
    vec2(0.1254510,0.0428502), vec2(0.2816537,-0.3045711), vec2(-0.2343018,-0.2459390),
    vec2(0.0625516,-0.2719784), vec2(-0.3949863,-0.2474681), vec2(0.0501389,-0.4359268),
    vec2(-0.1602987,-0.0242505), vec2(0.3998221,0.1279425), vec2(0.1698757,0.2820195),
    vec2(0.4191946,-0.0148812), vec2(0.4103152,-0.2532885), vec2(-0.0010199,0.3389769),
    vec2(-0.2646317,-0.1102498), vec2(0.2064117,0.4451604), vec2(-0.0788299,0.1059370),
    vec2(-0.3209068,0.1344933), vec2(0.0868388,0.1710649), vec2(-0.3878541,-0.0204674),
    vec2(-0.4418672,0.1825800), vec2(-0.3623412,0.3157248), vec2(-0.1956292,0.2076620),
    vec2(0.0205688,0.4664732), vec2(-0.1860556,0.4323920)
);

vec2 POISSON64[] = vec2[64](
    vec2(-0.0189662,-0.0510488), vec2(-0.1820639,-0.0553801), vec2(0.0910325,0.0252679),
    vec2(0.1096571,-0.0798338), vec2(-0.1469904,0.1132023), vec2(0.2343081,-0.1905298),
    vec2(-0.0029982,0.0958551), vec2(0.3510874,-0.1930093), vec2(0.0468733,-0.1524058),
    vec2(-0.1218595,-0.2167346), vec2(0.2739988,-0.0158153), vec2(0.1341032,-0.2588954),
    vec2(0.2062096,-0.0821571), vec2(-0.1026306,-0.0041678), vec2(-0.3240024,-0.0798507),
    vec2(0.3697911,0.0458827), vec2(-0.2538350,-0.2965067), vec2(-0.2396912,0.0628588),
    vec2(-0.3017254,-0.1893546), vec2(0.2113072,-0.3186852), vec2(0.0559174,0.2359820),
    vec2(-0.3721051,0.0980429), vec2(-0.1430048,0.2194094), vec2(-0.0514073,0.3617615),
    vec2(-0.2960384,0.1891084), vec2(-0.0552694,0.1748697), vec2(-0.0987295,-0.1174246),
    vec2(0.3565632,0.1850419), vec2(0.1723162,-0.4579452), vec2(0.3403926,-0.3167597),
    vec2(-0.1414267,0.4724176), vec2(-0.4680430,-0.1488462), vec2(0.2291788,0.1936403),
    vec2(-0.1400955,-0.4132020), vec2(0.1192180,-0.3781818), vec2(-0.3150060,0.3645030),
    vec2(0.1893810,0.0889743), vec2(0.0909581,0.1423441), vec2(-0.0500480,-0.4849751),
    vec2(-0.2104492,0.2853596), vec2(0.3527338,0.3100588), vec2(0.0354831,0.4304752),
    vec2(0.4190884,-0.0489801), vec2(0.1890273,0.3002760), vec2(0.4564034,0.0862838),
    vec2(0.1851432,0.4389251), vec2(-0.0038145,-0.2962559), vec2(0.0485585,0.3323395),
    vec2(0.2843748,0.0984157), vec2(0.4504704,-0.1657754), vec2(-0.3932974,-0.2612363),
    vec2(-0.2073296,0.3838763), vec2(-0.4316504,0.2052262), vec2(-0.2043341,-0.1549807),
    vec2(-0.3898448,0.3030459), vec2(-0.4078800,-0.0078618), vec2(-0.2387565,-0.4155289),
    vec2(-0.0335876,0.2676137), vec2(0.0709581,-0.4616181), vec2(-0.3274855,-0.3756900),
    vec2(-0.0448154,0.4841810), vec2(-0.4669865,0.1102869), vec2(-0.0956072,-0.3239126),
    vec2(0.2771143,0.3817498)
);

#define BLOCKER_SEARCH_NUM_SAMPLES 16
#define PCF_NUM_SAMPLES 16
#define NEAR_PLANE 20.0
#define LIGHT_WORLD_SIZE .5
#define LIGHT_FRUSTUM_WIDTH 8
// Assuming that LIGHT_FRUSTUM_WIDTH == LIGHT_FRUSTUM_HEIGHT
#define LIGHT_SIZE_UV LIGHT_WORLD_SIZE / LIGHT_FRUSTUM_WIDTH

float hash21(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float GetScale(int layer)
{
    float scale = 1.2;
    scale = (layer > 1) ? 1.5 : scale;
    scale = (layer > 2) ? 2.0 : scale;
    scale = (layer > 3) ? 3.0 : scale;
    return scale;
}

float GetBias(vec3 lightDir, vec3 normal, float R, int layer)
{
    float biasA = (1 + ceil(R)) * _csmFrustumSizes[0] / (1024.0 * 2.0);
    float biasB = 1.0 - saturate(dot(lightDir, normal));
    float biasD = _depthBias * biasA * biasB;
    float biasN = _normalBias * biasA * biasB;
    float biasTotal = biasD + biasN;
    return biasTotal * GetScale(layer) + 0.001;
}


vec2 Rotate(vec2 pos, vec2 rotationTrig)
{
return vec2(pos.x * rotationTrig.x - pos.y * rotationTrig.y, pos.y * rotationTrig.x + pos.x * rotationTrig.y);
}

float PenumbraSize(float zReceiver, float zBlocker) 
{
    return (zReceiver - zBlocker) / zBlocker;
}

void FindBlocker(out float avgBlockerDepth, out float numBlockers, vec2 uv, float searchRadius, 
                     float zReceiver, float nearPlane, int layer, vec3 lightDir, vec3 normal, float distanceZ)
{
    
    /*
      float3 lightDir = normalize(_MainLightPosition.xyz);
    float tan = lightDir.y / sqrt(1 - lightDir.y * lightDir.y + 0.0001);
    float texelSize = OrthHalfWidth/512;
    float deltaZ_WS = blockerSearchWidth * texelSize / tan;
    float deltaZ_LS = deltaZ_WS / _CascadeZDistanceArray[shadowCoord.w];
    float2 blocker = SampleBlockerAvgDepth(mDepthLS + deltaZ_LS, shadowCacheUVId, blockerSearchWidth, random);

    */
    vec3 lightD = normalize(lightDir);
    float tan = lightD.y / sqrt(1 - lightD.y * lightD.y + 0.0001);
    
    float texelSizeD = _csmFrustumSizes[layer]/1024.0;
    float searchWidth = searchRadius;
    float blockerSum = 0;
    numBlockers = 0;
    vec2 texelSize = 1.0 /  vec2(textureSize(_shadowMap, 0));
    float deltaZ_WS = searchWidth * texelSizeD / tan;
    float deltaZ_LS = deltaZ_WS /_cascadePlaneDistances[layer];
    for(uint i = 0u; i < uint(BLOCKER_SEARCH_NUM_SAMPLES); ++i )
    {
        vec2 xi = fract(Hammersley(i, uint(BLOCKER_SEARCH_NUM_SAMPLES)) + hash21(gl_FragCoord.xy) + vec2(0.08,0.02));
        float r = sqrt(xi.x) * searchWidth;
        float theta = xi.y * 2.0 * 3.14159;
        vec2 offset = r * vec2(cos(theta), sin(theta));

         //float pcfBias = 2.0 * r / 100;
         //float realBias = mix(pcfBias * (1+layer), 0.0,  bias);

        float bias = GetBias(lightDir, normal, r, layer);
        //vec2 offset = POISSON16[i] * searchWidth;
        float shadowMapDepth = texture(_shadowMap, vec3(uv + offset * texelSize, layer)).r;
        if ( shadowMapDepth + deltaZ_LS  < zReceiver ) {
            blockerSum += shadowMapDepth;
            numBlockers++;
        }
    }
    avgBlockerDepth = blockerSum / numBlockers;
}

float PCF_Filter(vec2 uv, float zReceiver, float filterRadius, int layer, vec3 lightDir, vec3 normal)
{
    float sum = 0.0f;
    vec2 texelSize = 1.0 /  vec2(textureSize(_shadowMap, 0));
    for ( int i = 0; i < PCF_NUM_SAMPLES; ++i )
    {
        vec2 xi = fract(Hammersley(uint(i), uint(PCF_NUM_SAMPLES)) + hash21(gl_FragCoord.xy) + vec2(3,2));
        float r = sqrt(xi.x) * filterRadius;
        float theta = xi.y * 2.0 * 3.14159;
        vec2 offset = r * vec2(cos(theta), sin(theta));
        float bias = GetBias(lightDir, normal, r, layer);
       // vec2 offset = POISSON16[i] * filterRadius;
        float pcdDepth = texture(_shadowMap, vec3(uv + offset * texelSize, layer)).r;
        sum += pcdDepth + bias < zReceiver ? 1.0 : 0.0;
    }
    return sum / PCF_NUM_SAMPLES;
}

float PCF_Classic(vec2 projCoords, int layer, vec2 texelSize, float currentDepth, float bias, vec2 rotationTrig)
{
    float shadow = 0.0;
    for(int x = -2; x <= 2; ++x)
    {
        for(int y = -2; y <= 2; ++y)
        {
            vec2 offset = vec2(x, y) * texelSize;
            //offset = Rotate(offset, rotationTrig);
            float pcfDepth = texture(_shadowMap, vec3(projCoords + offset, layer)).r;
            shadow += currentDepth - ( bias) >= pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow /= 25.0;
}

float GetCameraFrustumWidth(float fov, float nearPlane)
{
    float deg2rad = (3.1415959 * 2.0) / 360.0;
    return 2.0f * tan((deg2rad * fov) / 2.0) * nearPlane;
}

float ShadowCalculation(vec3 positionWS, vec3 normal, vec3 lightDir, vec2 uvOffset, mat4 _view)
{
    vec4 positionVS = _view * vec4(positionWS, 1.0);
    float depthValue = length(vec3(0,0,0) - vec3(positionVS));
    int layer = -1;
    for (int i = 0; i < _cascadeCount; ++i)
    {
        if (depthValue < _cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }

    if (layer == -1)
    {
        layer = _cascadeCount;
    }

    vec4 coords = _shadowMatrix[layer] * vec4(positionWS, 1.0);
    vec3 projCoords = coords.xyz / coords.w;
    if(projCoords.z > 1.0)
    {
        return 0.0;
    }
    projCoords = projCoords * 0.5 + 0.5;
    
    float currentDepth = projCoords.z;
    // blocker search
    float avgBlockerDepth = 0;
    float numBlockers = 0;
    float receiver = currentDepth;
    float nearPlane = _nearPlane;
    /*
    float3 lightDir = normalize(_MainLightPosition.xyz);
    float tan = lightDir.y / sqrt(1 - lightDir.y * lightDir.y + 0.0001);
    float texelSize = OrthHalfWidth/512;
    float deltaZ_WS = blockerSearchWidth * texelSize / tan;
    float deltaZ_LS = deltaZ_WS / _CascadeZDistanceArray[shadowCoord.w];
    float2 blocker = SampleBlockerAvgDepth(mDepthLS + deltaZ_LS, shadowCacheUVId, blockerSearchWidth, random);
    */
    FindBlocker( avgBlockerDepth, numBlockers, projCoords.xy, _pcssSearchRadius, receiver, nearPlane, layer, lightDir, normal,  coords.w);
    if( numBlockers < 1 )
        return 0.0f;
    
    float penumbraRatio = (receiver - avgBlockerDepth) * _pcssFilterRadius;
    float filterRadius = penumbraRatio;
    
    float shadow = 0.0;
    shadow = PCF_Filter(projCoords.xy, receiver, filterRadius, layer, lightDir, normal);
    return shadow;
}


vec3 ShadowCalculationDebug(vec3 positionWS, vec3 normal, vec3 lightDir, mat4 _view)
{
    vec4 positionVS = _view * vec4(positionWS, 1.0);
    float depthValue = length(vec3(0,0,0) - vec3(positionVS));
    int layer = -1;
    for (int i = 0; i < _cascadeCount; ++i)
    {
        if (depthValue < _cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }

    //lightSpace position 
    vec4 coords = _shadowMatrix[layer] * vec4(positionWS, 1.0);

    vec3 projCoords = coords.xyz / coords.w;

    projCoords = projCoords * 0.5 + 0.5;

    if(coords.z > 1.0)
    {
        //cyan color 
        return vec3(0,1,1);
    }

    if(layer == 0)
    {
        return vec3(0,1,0);
    }

    if(layer == 1)
    {
        return vec3(0,0,1);
    }

    if(layer == 2)
    {
        return vec3(1,0,1);
    }

    if(layer == 3)
    {
        return vec3(1,1,0);
    }

    return   vec3(1,0,0);
}