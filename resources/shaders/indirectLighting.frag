layout (location = 0) out vec3 FragColor;
in vec2 texCoord;

uniform sampler2D _depthMap;
//RSM
uniform sampler2D _rsmDepth;
uniform sampler2D _rsmPosition;
uniform sampler2D _rsmNormal;
uniform sampler2D _rsmFlux;

uniform mat4 _projection;
uniform mat4 _cameraViewMatrix;
uniform float _rsmMax = 0.16f;
uniform float _rsmFluxStrength = 1.0f;

vec3 reconstructPosition(in vec2 uv, in float z, in mat4x4 InvVP)
{
    float x = uv.x * 2.0f - 1.0f;
    float y = uv.y * 2.0f - 1.0f;
    vec4 position_s = vec4(x, y, z * 2 - 1, 1.0f);
    vec4 position_v = InvVP *  position_s;
    return position_v.xyz / position_v.w;
}

//linearize to range between near and far
float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

void main()
{
    float depth = textureLod(_depthMap, texCoord, 0).r;
    if(depth >= 1)
    {
        FragColor = vec3(1,1,1);
        return;
    }
    vec3 positionVS = reconstructPosition(texCoord, depth, inverse(_projection));
    vec3 positionWS = (inverse(_cameraViewMatrix) * vec4(positionVS.xyz, 1)).xyz;
    vec3 normalWS = normalize(cross(dFdx(positionWS), dFdy(positionWS)));

    vec4 rsmPositionCS = _rsmMatrix[0] * vec4(positionWS, 1.0);
    vec3 rsmProj = rsmPositionCS.xyz / rsmPositionCS.w;
    rsmProj = rsmProj * 0.5 + 0.5;
    vec3 ambient = vec3(0, 0, 0);

    mat4x4 inverseRsmVP = inverse(_rsmMatrix[0]);
    // Calculate the area of the world-space disk we are integrating over.
    float rMaxWorld = distance(reconstructPosition(rsmProj.xy, 0.0, inverseRsmVP),
    reconstructPosition(vec2(rsmProj.x + _rsmMax, rsmProj.y), 0.0, inverseRsmVP));

    // Samples need to be normalized based on the radius that is sampled, otherwise changing rMax will affect the brightness.
    float normalizationFactor = 2.0 * rMaxWorld * rMaxWorld;
    /*for(uint i = 0; i < uint(100); i++)
    {
        vec2 xi = Hammersley(i,  uint(100));
        //10 = rMax
        float r = xi.x * _rsmMax;
        float theta = xi.y * TWO_PI;
        vec2 pixelLightUV = rsmProj.xy + vec2(r * cos(theta), r * sin(theta));
        float weight = xi.x;

        float rsmDepth = texture(_rsmDepth, pixelLightUV).r;
        vec3 normalWSVpl = texture(_rsmNormal, pixelLightUV).xyz;
        //vec3 positionWsVpl = texture(_rsmPosition, pixelLightUV).xyz;
        vec3 positionWsVpl = reconstructPosition(pixelLightUV, rsmDepth, inverse(_rsmMatrix[0]));
        vec3 fluxVpl = texture(_rsmFlux, pixelLightUV).xyz * _rsmFluxStrength * 2;

        vec3 indirectLightRSM =
        fluxVpl * (max(0, dot(normalWSVpl, positionWS - positionWsVpl)) *
        max(0, dot(normalWS, positionWsVpl - positionWS))
        ) / pow(length(positionWS - positionWsVpl), 4.0);
        indirectLightRSM *= weight;
        ambient += indirectLightRSM;
    }*/
    ambient = vec3(0.05, 0.05, 0.05);
    FragColor = ambient;
}