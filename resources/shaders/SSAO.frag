
layout (location = 0)out float FragColor;
in vec2 texCoord;

uniform sampler2D _depthMap;
uniform sampler2D _gPosition;
uniform sampler2D _gNormal;
uniform sampler2D _noiseTex;

uniform float _aoStrength = 1.5;
uniform vec2 _aoRes = vec2(1440, 960);
uniform vec3 _samples[64];
uniform vec3 _cameraPos;
uniform mat4 _cameraViewMatrix;
uniform mat4 _projection;
uniform float _radius;
uniform float _bias;

vec3 reconstructPosition(in vec2 uv, in float z, in mat4x4 InvVP)
{
    float x = uv.x * 2.0f - 1.0f;
    float y = uv.y * 2.0f - 1.0f;
    vec4 position_s = vec4(x, y, z * 2 - 1, 1.0f);
    vec4 position_v = InvVP *  position_s;
    return position_v.xyz / position_v.w;
}

vec4 ClipToView( vec4 clip )
{
    vec4 positionVS = inverse(_projection) * clip;
    positionVS = positionVS / positionVS.w;
    return positionVS;
}

vec4 ScreenToView( vec2 screenUV, float rawDetph )
{
    vec3 positionNDC = vec3(screenUV ,  rawDetph * 2.0 - 1.0);
    return ClipToView(vec4(positionNDC, 1));
}

vec4 NDCtoView( vec2 screenUV, float rawDetph )
{
    vec3 positionNDC = vec3(screenUV ,  rawDetph * 2.0 - 1.0);
    return ClipToView(vec4(positionNDC, 1));
}


void main()
{
    vec2 noiseScale = vec2(_aoRes.xy / 4.0);
    vec3 randomVec = normalize(texture(_noiseTex, texCoord * noiseScale).xyz);
    float depth = texture(_depthMap, texCoord).r;
    if(depth >= 1)
    {
        FragColor = 1;
        return;
    }
    vec3 positionVS = reconstructPosition(texCoord, depth, inverse(_projection));

    vec3 positionWS = (inverse(_cameraViewMatrix) * vec4(positionVS.xyz, 1)).xyz;
    vec3 normalWS = normalize(cross(dFdx(positionWS), dFdy(positionWS)));

    vec3 normalVS = mat3(_cameraViewMatrix) * normalWS;
    vec3 T = normalize(randomVec - normalVS * dot(randomVec, normalVS));
    vec3 B = cross(normalVS, T);
    //a matrix transform vector from tangent space to view space
    mat3 TBN = mat3(T, B, normalVS);
    
    float occlusion = 0.0;
    int kernelSize = 64;
    float radius = _radius / 30.0;
    float bias = _bias;
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * _samples[i]; // from tangent to view-space
        samplePos = positionVS.xyz + samplePos * radius;
        vec4 offset = vec4(samplePos, 1);
        offset = _projection * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // from -1,1 to 0, 1
        
        if(offset.x <= 1 && offset.x >=0 && offset.y <= 1 && offset.y >=0)
        {
            float rawDepth = texture(_depthMap, offset.xy).r;
            float viewZ = ScreenToView(offset.xy * 2 - 1, rawDepth).z;
            float rangeCheck = smoothstep(0.0, 1.0, radius / abs(positionVS.z - viewZ));
            occlusion += (viewZ >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
        }
    }

    occlusion = saturate(1.0 - (occlusion / kernelSize) * _aoStrength);
    FragColor = occlusion;
}