#version 460 core

uniform mat4 _model;
uniform mat4 _view;
uniform mat4 _projection;
uniform mat4 _InverseProjection;
uniform vec2 _ScreenDimensions;
uniform uint _maxLightPerTile;
uniform uint _numOfLights;


bool SphereInsidePlane(vec3 sphereCenter, float radius, vec3 planeNoraml, float d)
{
    return abs(dot( planeNoraml, sphereCenter ) + d) < radius;
}


vec4 ComputePlane( vec3 p0, vec3 p1, vec3 p2 )
{
    vec4 plane;
    vec3 v0 = p1 - p0;
    vec3 v2 = p2 - p0;
    plane.xyz = normalize( cross( v0, v2 ) );
    plane.w = dot( plane.xyz, p0 );
    return plane;
}

vec4 ClipToView( vec4 clip )
{
    vec4 view = _InverseProjection * clip;
    view = view / view.w;
    return view;
}

vec4 ScreenToView( vec4 screen )
{
    vec3 ndcCoord = vec3(screen.xy / _ScreenDimensions.x * 2.0 - 1.0,  screen.z * 2.0 - 1.0);
    return ClipToView( clip );
}

vec3 UnprojectUV(float depth, vec2 uv, mat4 inverseProjView)
{
    float z = depth * 2.0 - 1.0;
    vec4 ndc = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 posWS = inverseProjView * ndc;
    return posWS.xyz / posWS.w;
}

struct Light {
    vec4 position; // position.w is radius
    vec4 color; // color.w is intensity
};

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform sampler2D _depthMap;

layout(std430, binding = 5) buffer LightsList
{
    Light _lights[];
};
layout(std430, binding = 6) buffer LightIndexesList
{
    int _lightIndexesList[];
};
layout(std430, binding = 7) buffer TileLightCounts
{
    int _perTileLightCount[];
};

#define TILE_SIZE 16
layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main() {
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 tileCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 numOfGlobalThreads =  ivec2(_ScreenDimensions.xy / TILE_SIZE);
    uint totalThreads = numOfGlobalThreads.x * numOfGlobalThreads.y;
    uint tileGlobalIndex1D = gl_GlobalInvocationID.y * numOfGlobalThreads.x + gl_GlobalInvocationID.x;
    uint lightIndexesStart = tileGlobalIndex1D * _maxLightPerTile;
            
    ivec2 texelCoordBegin = tileCoords * ivec2(TILE_SIZE, TILE_SIZE);
    
    if(texelCoordBegin.x >= 1440 || texelCoordBegin.y >= 960)
    {
        return;
    }
    
    ivec2 texelCoordCorner0 = ivec2(tileCoords.xy) * ivec2( TILE_SIZE, TILE_SIZE);
    ivec2 texelCoordCorner1 = ivec2(tileCoords.x + 1, tileCoords.y) * ivec2( TILE_SIZE, TILE_SIZE );
    ivec2 texelCoordCorner2 = ivec2(tileCoords.x, tileCoords.y + 1) * ivec2( TILE_SIZE, TILE_SIZE );
    ivec2 texelCoordCorner3 = ivec2(tileCoords.x + 1, tileCoords.y + 1) * ivec2( TILE_SIZE, TILE_SIZE );
    
    vec4 cornerSS[4];
    cornerSS[0] = vec4(texelCoordCorner0, -1.0f, 1.0f);
    cornerSS[1] = vec4(texelCoordCorner1, -1.0f, 1.0f);
    cornerSS[2] = vec4(texelCoordCorner2, -1.0f, 1.0f);
    cornerSS[3] = vec4(texelCoordCorner3, -1.0f, 1.0f);
    
    vec3 cornerVS[4];
    for ( int i = 0; i < 4; i++ )
    {
        cornerVS[i] = ScreenToView( cornerSS[i] ).xyz;
    }
    
    vec3 eyePos = vec3(0,0,0);

    float zNear = -1000000.;
    float zFar = 1000000.;
    uint numLightsInTile = 0;
    mat4 inverseProjectView = inverse(_projection * _view);
    mat4 inverseView = inverse(_view);
    for(int y = 0; y < TILE_SIZE; y++)
    {
        for(int x = 0; x < TILE_SIZE; x++)
        {
            ivec2 pixelCoord = texelCoordBegin + ivec2(x, y);
            vec2 uv = pixelCoord / vec2(1440.0, 960.0);
            float depth = texture(_depthMap, uv).r;
            vec4 screenDepth = vec4(pixelCoord, depth, 1.);
          //  imageStore(imgOutput, pixelCoord, vec4(screenDepth.z, screenDepth.z, screenDepth.z, 1));
            vec3 depthVS = UnprojectUV(screenDepth.z, uv, inverseProjectView);
            //float eyeDepth = 30.0 * 0.1 / ((0.1 - 30.0) * depth + 30.0);
            zNear = max(zNear, depth);
            zFar = min(zFar, depth);
        }
    }

    zNear *= 100;
    zFar *= 100;
    vec4 frustumPlanes[6];
    
    vec2 negativeStep = (2.0 * vec2(gl_GlobalInvocationID.xy)) / vec2(numOfGlobalThreads.xy);
    vec2 positiveStep = (2.0 * vec2(gl_GlobalInvocationID.xy + ivec2(1, 1))) / vec2(numOfGlobalThreads.xy);

    // Set up starting values for planes using steps and min and max z values
    frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
    frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
    frustumPlanes[2] = vec4(0.0, 1.0, 0.0, 1.0 - negativeStep.y); // Bottom
    frustumPlanes[3] = vec4(0.0, -1.0, 0.0, -1.0 + positiveStep.y); // Top
    frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -zNear); // Near
    frustumPlanes[5] = vec4(0.0, 0.0, 1.0, zFar); // Far

    // Transform the first four planes
    for (uint i = 0; i < 4; i++) {
        frustumPlanes[i] = _view * _projection * frustumPlanes[i];
        frustumPlanes[i] /= length(frustumPlanes[i].xyz);
    }

    // Transform the depth planes
    frustumPlanes[4] = _view * frustumPlanes[4];
    frustumPlanes[4] /= length(frustumPlanes[4].xyz);
    frustumPlanes[5] = _view * frustumPlanes[5];
    
    frustumPlanes[5] /= length(frustumPlanes[5].xyz);
    
    vec3 finalValue = vec3(0,0,0);
    for(int i = 0; i < int(_numOfLights); i++)
    {
        Light light = _lights[i];
        vec3 posWS = light.position.xyz;
        float radius = light.position.w;
        uint pass = 0;
        for ( int j = 0; j < 6; j++ )
        {
            float d = dot(vec4(posWS.xyz, 1), frustumPlanes[j]) + radius * 0.1;
            if(d > 0.0)
            {
                pass += 1;
            }
        }
        if(pass == 6)
        {
            finalValue += light.color.xyz;
        }
       
    }
    
    for(int y = 0; y < TILE_SIZE; y++)
    {
        for(int x = 0; x < TILE_SIZE; x++)
        {
            ivec2 pixelCoord = texelCoordBegin + ivec2(x, y);
            imageStore(imgOutput, pixelCoord, vec4(finalValue, 1));
        }
    }

}