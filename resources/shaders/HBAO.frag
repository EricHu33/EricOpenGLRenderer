
layout (location = 0)out vec3 FragColor;
in vec2 texCoord;

uniform sampler2D _depthMap;
uniform sampler2D _gPosition;
uniform sampler2D _gNormal;
uniform sampler2D _noiseTex;
uniform sampler2D _blueNoiseTex;

uniform vec3 _samples[64];
uniform vec3 _cameraPos;
uniform mat4 _cameraViewMatrix;
uniform mat4 _projection;
uniform float _radius = 10.5;
uniform float _bias;
uniform float _aoStrength = 1.5;
uniform float _aoPower = 1.5;
uniform float _aoRandomSize = 4;

uniform bool _testLogic;
uniform bool _testDither;

uniform vec2 _aoRes = vec2(1440.0, 960.0);
uniform vec2 _invAORes = vec2(1.0/1440.0, 1.0/960.0);
uniform float _RadiusPixels = 30;

uniform float TanBias = tan(30.0 * PI / 180.0);
uniform float MaxRadiusPixels = 50.0;
#define DIRECTION_COUNT 8
#define NUM_OF_SAMPLES 4

float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}


vec3 hash32(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+33.33);
    return fract((p3.xxy+p3.yzz)*p3.zyx);
}

vec3 MinDiff(vec3 P, vec3 Pr, vec3 Pl)
{
    vec3 V1 = Pr - P;
    vec3 V2 = P - Pl;
    return (dot(V1,V1) < dot(V2,V2)) ? V1 : V2;
}

vec3 GetViewPos(in vec2 uv)
{
    float x = uv.x * 2.0f - 1.0f;
    float y = uv.y * 2.0f - 1.0f;
    float z = texture(_depthMap, uv).r;
    vec4 positionNDC = vec4(x, y, z * 2 - 1, 1.0f);
    vec4 positionView = inverse(_projection) * positionNDC;
    return positionView.xyz / positionView.w;
}

vec2 RotateDirections(vec2 Dir, vec2 CosSin)
{
    return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y,
                Dir.x*CosSin.y + Dir.y*CosSin.x);
}

vec2 SnapUVOffset(vec2 uv)
{
    return round(uv * _aoRes) * _invAORes;
}

float InvLength(vec2 V)
{
    return inversesqrt(dot(V,V));
}

float GetTangentValue(vec3 V, float bias)
{
    return V.z * InvLength(V.xy) + bias;
}

float Falloff(float d2)
{
    float r = _radius * 2;
    return d2 * (-1.0 / r * r) + 1.0;
}

vec3 ReconstructNormal(vec2 UV, vec3 P)
{
    vec3 Pr = GetViewPos(UV + vec2(_invAORes.x, 0));
    vec3 Pl = GetViewPos(UV + vec2(-_invAORes.x, 0));
    vec3 Pt = GetViewPos(UV + vec2(0, _invAORes.y));
    vec3 Pb = GetViewPos(UV + vec2(0, _invAORes.y));
    return normalize(cross(MinDiff(P, Pr, Pl), MinDiff(P, Pt, Pb)));
}

//----------------------------------------------------------------------------------
// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
//----------------------------------------------------------------------------------
float ComputeAO(vec3 P, vec3 N, vec3 S)
{
    vec3 V = S - P;
    float VdotV = dot(V, V);
    float NdotV = dot(N, V) * 1.0/sqrt(VdotV);

    // Use saturate(x) instead of max(x,0.f) because that is faster on Kepler
    return clamp(NdotV - _bias * 5,0,1) * clamp(Falloff(VdotV),0,1);
}

void Unity_Dither_float4(vec4 In, vec2 ScreenPosition, out vec4 Out)
{
    vec2 uv = ScreenPosition.xy * _aoRes.xy;
    float DITHER_THRESHOLDS[16] =
    {
    1.0 / 17.0,  9.0 / 17.0,  3.0 / 17.0, 11.0 / 17.0,
    13.0 / 17.0,  5.0 / 17.0, 15.0 / 17.0,  7.0 / 17.0,
    4.0 / 17.0, 12.0 / 17.0,  2.0 / 17.0, 10.0 / 17.0,
    16.0 / 17.0,  8.0 / 17.0, 14.0 / 17.0,  6.0 / 17.0
    };
    uint index = (uint(uv.x) % 4) * 4 + uint(uv.y) % 4;
    Out = In - DITHER_THRESHOLDS[index];
}

void main()
{
    vec2 noiseScale = vec2(_aoRes.xy / 4.0);
    
    //vec3 randomVec = normalize(texture(_noiseTex, texCoord * noiseScale).xyz);
    vec3 blueNoise = texture(_blueNoiseTex, texCoord / _aoRandomSize).rgb;
    vec3 positionVS = GetViewPos(texCoord);
    float numDirections = DIRECTION_COUNT;
    vec3 P, Pr, Pl, Pt, Pb;
    P   = positionVS;
    Pr 	= GetViewPos(texCoord + vec2(_invAORes.x, 0));
    Pl 	= GetViewPos(texCoord + vec2(-_invAORes.x, 0));
    Pt 	= GetViewPos(texCoord + vec2(0, _invAORes.y));
    Pb 	= GetViewPos(texCoord + vec2( 0,-_invAORes.y));

    // Calculate tangent basis vectors using the minimu difference
    vec3 dPdu = MinDiff(P, Pr, Pl);
    vec3 dPdv = MinDiff(P, Pt, Pb) * (_aoRes.y * _invAORes.x);

    vec3 ViewNormal = -ReconstructNormal(texCoord, positionVS);
    vec3 random = blueNoise;
    /*if(_testLogic)
    {
        vec4 rand = vec4(0,0,0,0);
        Unity_Dither_float4(vec4(_aoRandomSize, _aoRandomSize, _aoRandomSize, _aoRandomSize), texCoord.xy, rand);
        random = rand.xyz;
    }*/

    //radius(in pixel)
    float radius = _radius * (1 - abs(positionVS.z / 30));
    float R2 = radius * radius;
    // vec2 rayRadiusUV = 0.5 * radius * FocalLen / -P.z;

    float ao = 0;
    float numSteps = min(NUM_OF_SAMPLES, radius);
    if (radius < 1.0)
    {
        FragColor = vec3(1,1,1);
        return;
    }
    
    vec2 perStepSize = radius / (numSteps + 1) * _invAORes;
    float alpha = 2.0 * PI / DIRECTION_COUNT;
   
    
    for(float d = 0; d < DIRECTION_COUNT; ++d)
    {
        float theta = alpha * d;
        vec2 dir = RotateDirections(vec2(cos(theta), sin(theta)), random.xy);
        vec2 deltaUV = dir * perStepSize;

        deltaUV = SnapUVOffset( deltaUV );
        //float sinTangent = sin(atan(planeTangent));
        vec2 sampleUV = texCoord;
        /*if(_testLogic)
        {
            sampleUV = SnapUVOffset(texCoord + random.z * perStepSize);
        }*/
        for(float s = 1; s <= numSteps; ++s)
        {
            sampleUV += deltaUV;
         //   sampleUV.x = saturate(sampleUV.x);
         //   sampleUV.y = saturate(sampleUV.y);
            vec3 S = GetViewPos(sampleUV);
            ao += ComputeAO(P, ViewNormal, S);
        }
    }
    ao *= _aoStrength / (DIRECTION_COUNT * NUM_OF_SAMPLES);
    ao =  clamp(1.0 - ao * 2.0,0,1);
    ao = pow(ao, _aoPower);
    
    FragColor = vec3(ao,ao,ao);
}
