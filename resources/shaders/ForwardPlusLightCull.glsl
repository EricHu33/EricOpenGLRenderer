#version 460 core

struct Light {
    vec4 position; // position.w is radius
    vec4 color; // color.w is intensity
};
uniform sampler2D _depthMap;

layout(rgba32f, binding = 0) uniform image2D imgOutput;

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

uniform mat4 _model;
uniform mat4 _view;
uniform mat4 _projection;
uniform mat4 _InverseProjection;
uniform vec2 _ScreenDimensions;
uniform uint _maxLightPerTile;
uniform uint _numOfLights;

// Shared values between all the threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visibleLightCount;
shared vec4 frustumPlanes[6];
shared vec4 tileCornersScreenSpace[4];


float hash11(float p)
{
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash12(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

//linearize to range between near and far
float linearize_depth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
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

/*
tmp = (P*V)^-1 * (NDC,1.0);    
world space = tmp/tmp.w;

In the strict mathematical sense, you don't do the exact inversion of the forward transform .
You could do the exact inversion if you had the clip space w value for the NDC point you want to project back. In that case, you could undo the division by w by multiplying the NDC coords with that w to get the clip space, and after multiplying with the inverse projection matrix, you would get the original point back, with the original eye space w coordinate you put in. That one is typically 1, so you completely would avoid the division of the final result by w. Now you have the exact inversion, where the division is inverted by a multiplication.
It still works the way you do it because the 4D homogenous representation of a specific 3D point is scale-invariant. So instead of the w value you got from the forward transformation, you can use any w value to represent the same point (except 0). That just means that you get some different point in the homogenous space back - you don't get the original input w value, but something else. In practice, this does not matter in most cases, since you do not care about the exact 4D homogenous coordinates, but just the 3D point it represents, and you can get that by another divide.
*/

vec4 ClipToView( vec4 clip )
{
    vec4 positionVS = _InverseProjection * clip;
    positionVS = positionVS / positionVS.w;
    return positionVS;
}

vec4 ScreenToView( vec4 screen )
{
    vec3 positionNDC = vec3(screen.xy / _ScreenDimensions * 2.0 - 1.0,  screen.z * 2.0 - 1.0);
    return ClipToView(vec4(positionNDC, 1));
}

#define TILE_SIZE 16
layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    // each workgroup = a single tile, each tile contains 16*16 threads
    
    // location = 2d pixel id = group number * group size * thread size
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);
    // thread id (0-15, 0-15)
    ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
    // tileId = (0-(90-1), 0-(60-1))
    ivec2 tileID = ivec2(gl_WorkGroupID.xy);
    ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
    // 1D index of tileID
    uint tileIndex1D = tileID.y * tileNumber.x + tileID.x;
    uint perTileStartIndex = tileIndex1D * _maxLightPerTile;
    if (gl_LocalInvocationIndex == 0) {
        minDepthInt = 0xFFFFFFFF;
        maxDepthInt = 0;
        visibleLightCount = 0;
    }
    
    barrier();

    // Step 1: Calculate the minimum and maximum depth values (from the depth buffer) for this group's tile
    float maxDepth, minDepth;
    vec2 uv = vec2(location) / _ScreenDimensions;
    float depth = texture(_depthMap, uv).r;
    float rawDepth = depth;
    float near   = _projection[3][2]/(_projection[2][2] - 1.0);
    float far    = _projection[3][2]/(_projection[2][2] + 1.0);
    // Linearize the depth value from depth buffer (must do this because we created it using projection)
    depth = linearize_depth(depth, near, far);
    
    // Convert depth to uint so we can do atomic min and max comparisons between the threads
    uint depthInt = floatBitsToUint(depth);
    atomicMin(minDepthInt, depthInt);
    atomicMax(maxDepthInt, depthInt);

    barrier();
    ivec2 positionSS[4];
    vec3 positionVS[4];
    
    // Step 2: One thread should calculate the frustum planes to be used for this tile
    if (gl_LocalInvocationIndex == 0) {
        // Convert the min and max across the entire tile back to float
        minDepth = uintBitsToFloat(minDepthInt);
        maxDepth = uintBitsToFloat(maxDepthInt);
    /*
     3          2
       ┌──────┐
       │      │
       └──────┘
    0           1
    */
        positionSS[0] = ivec2(tileID.x * TILE_SIZE, tileID.y * TILE_SIZE);
        positionSS[1] = ivec2((tileID.x + 1) * TILE_SIZE, tileID.y * TILE_SIZE);
        positionSS[2] = ivec2((tileID.x + 1) * TILE_SIZE, (tileID.y + 1) * TILE_SIZE);
        positionSS[3] = ivec2(tileID.x * TILE_SIZE , (tileID.y + 1) * TILE_SIZE);

        positionVS[0] = ScreenToView(vec4(positionSS[0], rawDepth, 1)).xyz;
        positionVS[1] = ScreenToView(vec4(positionSS[1], rawDepth, 1)).xyz;
        positionVS[2] = ScreenToView(vec4(positionSS[2], rawDepth, 1)).xyz;
        positionVS[3] = ScreenToView(vec4(positionSS[3], rawDepth, 1)).xyz;

        frustumPlanes[0] = ComputePlane(positionVS[3], vec3(0,0,0), positionVS[0]);
        frustumPlanes[1] = ComputePlane(positionVS[1], vec3(0,0,0), positionVS[2]);
        frustumPlanes[2] = ComputePlane(positionVS[2], vec3(0,0,0), positionVS[3]);
        frustumPlanes[3] = ComputePlane(positionVS[0], vec3(0,0,0), positionVS[1]);
        
        frustumPlanes[4] = vec4(0.0, 0.0, -1.0, -minDepth); 
        frustumPlanes[5] = vec4(0.0, 0.0, 1.0, maxDepth); 
    }
    barrier();

    uint threadCount = TILE_SIZE * TILE_SIZE;
    if (gl_LocalInvocationIndex == 0) {
        for (uint i = 0; i < _maxLightPerTile; i++) {
            _lightIndexesList[perTileStartIndex + i] = -1;
        }
        _perTileLightCount[tileIndex1D] = 0;
    }
    
    barrier();
    imageStore(imgOutput, location, vec4(0, 0, 0, 1));
    uint passCount = (_numOfLights  / (threadCount)) + 1;
    for (uint i = 0; i < passCount; i++) {
        uint lightIndex = itemID.y * 16 + itemID.x;
        if(lightIndex >= _numOfLights)
        {
            break;
        }
        
        vec4 position = _view *  vec4(_lights[lightIndex].position.xyz, 1);
        float radius = _lights[lightIndex].position.w;
        float hashedIndex = (float(lightIndex) / float(_numOfLights));

        float d = 1.0;
        float r = radius;
        d *= dot(position, frustumPlanes[0]) + r > 0 ? 1.0 : 0;
        d *= dot(position, frustumPlanes[1]) + r > 0 ? 1.0 : 0;
        d *= dot(position, frustumPlanes[2]) + r > 0 ? 1.0 : 0;
        d *= dot(position, frustumPlanes[3]) + r > 0 ? 1.0 : 0;
        d *= dot(position, frustumPlanes[4]) + r > 0 ? 1.0 : 0;
        d *= dot(position, frustumPlanes[5]) + r > 0 ? 1.0 : 0;
   
        if (d > 0.0) {
            // Add index to the shared array of visible indices
            uint offset = atomicAdd(visibleLightCount, 1);
            _lightIndexesList[perTileStartIndex + offset] = int(lightIndex);
        }
    }

    barrier();

    float idHash = hash12(vec2(tileID));
    if (gl_LocalInvocationIndex == 0) {
        _perTileLightCount[tileIndex1D] = int(visibleLightCount);
    }
    barrier();
    float tileLightCountHash = hash11(float(_perTileLightCount[tileIndex1D]));
    float heatLevel = float(_perTileLightCount[tileIndex1D]) / float(60);
    float tileLightIndexHash = hash11(float(_lightIndexesList[perTileStartIndex]));
    imageStore(imgOutput, location, vec4(heatLevel, heatLevel, heatLevel, 1));
}