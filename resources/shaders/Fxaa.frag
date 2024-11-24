
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D _baseMap;
uniform bool _testLogic;

// Settings for FXAA.

#define FXAA_EDGE_THRESHOLD_MIN 0.0312 // 1/32, 1/16, 1/12
#define FXAA_EDGE_THRESHOLD 0.125 // 1/4, 1/8, 1/16
#define SUBPIXEL_QUALITY 0.75
#define FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT 5

float edgeSearchSteps[FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT] = {1, 1.5, 2, 4, 12};

float rgb2luma(vec3 rgb){
    vec3 srgb = pow(rgb, vec3(1/2.2, 1/2.2, 1/2.2));
    return dot(srgb, vec3(0.299, 0.587, 0.114));
}

void main()
{
    vec2 inverseScreenSize = vec2(1 / 1440.0, 1 / 960.0);
    vec3 colorCenter = texture(_baseMap, texCoord).rgb;

    // Luma at the current fragment
    float lumaM = rgb2luma(colorCenter);

    // Luma at the four direct neighbours of the current fragment.
    float lumaS = rgb2luma(textureOffset(_baseMap, texCoord , ivec2(0,-1)).rgb);
    float lumaN = rgb2luma(textureOffset(_baseMap, texCoord, ivec2(0,1)).rgb);
    float lumaW = rgb2luma(textureOffset(_baseMap, texCoord, ivec2(-1,0)).rgb);
    float lumaE = rgb2luma(textureOffset(_baseMap, texCoord, ivec2(1,0)).rgb);

    // Find the maximum and minimum luma around the current fragment.
    float lumaMin = min(lumaM,min(min(lumaS,lumaN),min(lumaW,lumaE)));
    float lumaMax = max(lumaM,max(max(lumaS,lumaN),max(lumaW,lumaE)));

    // Compute the delta.
    float lumaRange = lumaMax - lumaMin;

    // If the luma variation is lower that a threshold (or if we are in a really dark area), we are not on an edge, don't perform any AA.
    if(lumaRange < max(FXAA_EDGE_THRESHOLD_MIN, lumaMax * FXAA_EDGE_THRESHOLD)){
        FragColor = vec4(colorCenter, 1);
        return;
    }

    // Query the 4 remaining corners lumas.
    float lumaSW = rgb2luma(textureOffset(_baseMap,texCoord,ivec2(-1,-1)).rgb);
    float lumaNE = rgb2luma(textureOffset(_baseMap,texCoord,ivec2(1,1)).rgb);
    float lumaNW = rgb2luma(textureOffset(_baseMap,texCoord,ivec2(-1,1)).rgb);
    float lumaSE = rgb2luma(textureOffset(_baseMap,texCoord,ivec2(1,-1)).rgb);

    // Combine the four edges lumas (using intermediary variables for future computations with the same values).
    float lumaDownUp = lumaS + lumaN;
    float lumaLeftRight = lumaW + lumaE;

    // Same for corners
    float lumaLeftCorners = lumaSW + lumaNW;
    float lumaDownCorners = lumaSW + lumaSE;
    float lumaRightCorners = lumaSE + lumaNE;
    float lumaUpCorners = lumaNE + lumaNW;

    // Compute an estimation of the gradient along the horizontal and vertical axis.
    float edgeHorizontal =  abs(-2.0 * lumaW + lumaLeftCorners)  + abs(-2.0 * lumaM + lumaDownUp ) * 2.0    + abs(-2.0 * lumaE + lumaRightCorners);
    float edgeVertical =    abs(-2.0 * lumaN + lumaUpCorners)      + abs(-2.0 * lumaM + lumaLeftRight) * 2.0  + abs(-2.0 * lumaS + lumaDownCorners);

    // Is the local edge horizontal or vertical ?
    bool isHorizontal = (edgeHorizontal >= edgeVertical);

    // Select the two neighboring texels lumas in the opposite direction to the local edge.
    float luma1 = isHorizontal ? lumaS : lumaW;
    float luma2 = isHorizontal ? lumaN : lumaE;
    // Compute gradients in this direction.
    float gradient1 = luma1 - lumaM;
    float gradient2 = luma2 - lumaM;


    // Gradient in the corresponding direction, normalized.
    float searchThreashold = 0.25 * max(abs(gradient1),abs(gradient2));
    
    // Which direction is the steepest ?
    bool directionPositive = abs(gradient1) >= abs(gradient2);

    // Choose the step size (one pixel) according to the edge direction.
    float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

    // Average luma in the correct direction.
    float lumaLocalAverage = 0.0;

    if(directionPositive){
        // Switch the direction
        stepLength = - stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaM);
    } else {
        lumaLocalAverage = 0.5 * (luma2 + lumaM);
    }

    // Shift UV in the correct direction by half a pixel.
    vec2 searchOriginUV = texCoord;
    if(isHorizontal){
        searchOriginUV.y += stepLength * 0.5;
    } else {
        searchOriginUV.x += stepLength * 0.5;
    }

    // Compute offset (for each iteration step) in the right direction.
    vec2 offset = isHorizontal ? vec2(inverseScreenSize.x, 0.0) : vec2(0.0, inverseScreenSize.y);
    // Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step faster.
    vec2 uv1 = searchOriginUV;
    vec2 uv2 = searchOriginUV;

    // Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
    float lumaEnd1 = 0;
    float lumaEnd2 = 0;

    // If the luma deltas at the current extremities are larger than the local gradient, we have reached the side of the edge.
    bool reached1 = false;
    bool reached2 = false;

    // If both sides have not been reached, continue to explore.

        for(int i = 0; i < FXAA_MAX_EAGE_SEARCH_SAMPLE_COUNT; i++){
            // If needed, read luma in 1st direction, compute delta.
            if(!reached1){
                uv1 -= offset * edgeSearchSteps[i];
                lumaEnd1 = rgb2luma(texture(_baseMap, uv1).rgb);
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
                reached1 = abs(lumaEnd1) >= searchThreashold;
            }
            // If needed, read luma in opposite direction, compute delta.
            if(!reached2){
                uv2 += offset * edgeSearchSteps[i];
                lumaEnd2 = rgb2luma(texture(_baseMap, uv2).rgb);
                lumaEnd2 = lumaEnd2 - lumaLocalAverage;
                reached2 = abs(lumaEnd2) >= searchThreashold;
            }

            if( reached1 && reached2){ break;}
        }
    

    // Compute the distances to each extremity of the edge.
    float distance1 = isHorizontal ? abs(texCoord.x - uv1.x) : abs(texCoord.y - uv1.y);
    float distance2 = isHorizontal ? abs(texCoord.x - uv2.x) : abs(texCoord.y - uv2.y);

    // In which direction is the extremity of the edge closer ?
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    // Length of the edge.
    float edgeThickness = (distance1 + distance2);

    // UV offset: read in the direction of the closest side of the edge.
    float pixelOffset = - distanceFinal / edgeThickness + 0.5;

    // Is the luma at center smaller than the local average ?
    bool isLumaCenterSmaller = lumaM < lumaLocalAverage;

    // If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
    // (in the direction of the closer side of the edge.)
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

    // If the luma variation is incorrect, do not offset.
    float finalOffset = correctVariation ? pixelOffset : 0.0;

    // Sub-pixel shifting
    // Full weighted average of the luma over the 3x3 neighborhood.
    float lumaAverage = (1.0/12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    // Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaM)/lumaRange,0.0,1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    // Compute a sub-pixel offset based on this delta.
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

    // Pick the biggest of the two offsets.
    finalOffset = max(finalOffset,subPixelOffsetFinal);
    
    // Compute the final UV coordinates.
    vec2 finalUv = texCoord;
    if(isHorizontal){
        finalUv.y += finalOffset * stepLength;
    } else {
        finalUv.x += finalOffset * stepLength;
    }

    // Read the color at the new UV coordinates, and use it.
    vec3 finalColor = texture(_baseMap,finalUv).rgb;
    if(_testLogic)
    {
    FragColor = vec4(finalColor,1);
        
    }
    else
    {
        FragColor = vec4(colorCenter, 1);
    }
}