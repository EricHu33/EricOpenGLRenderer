//5
uniform sampler2D _heightMap;

float ComputePerPixelHeightDisplacement(vec2 uvOffset, float lod, vec2 uv)
{
    return textureLod(_heightMap, uvOffset + uv, lod).r;
}

vec2 Unity_Parallax_Mapping(float lod, float lodThreshold, int numSteps, vec3 viewDirTS, vec2 uv, float amplitude, out float outHeight)
{
    // Convention: 1.0 is top, 0.0 is bottom - POM is always inward, no extrusion
    float stepSize = 1.0 / float(numSteps);

    vec2 parallaxMaxOffsetTS = (viewDirTS.xy / -viewDirTS.z);
    vec2 texOffsetPerStep = stepSize * parallaxMaxOffsetTS * amplitude * 0.01;

    // Do a first step before the loop to init all value correctly
    vec2 texOffsetCurrent = vec2(0.0, 0.0);
    float prevHeight = ComputePerPixelHeightDisplacement(texOffsetCurrent, lod, uv);
    texOffsetCurrent += texOffsetPerStep;
    float currHeight = ComputePerPixelHeightDisplacement(texOffsetCurrent, lod, uv);
    float rayHeight = 1.0 - stepSize; // Start at top less one sample

    // Linear search
    for (int stepIndex = 0; stepIndex < numSteps; ++stepIndex)
    {
        // Have we found a height below our ray height ? then we have an intersection
        if (currHeight > rayHeight)
        break; // end the loop

        prevHeight = currHeight;
        rayHeight -= stepSize;
        texOffsetCurrent += texOffsetPerStep;

        // Sample height map which in this case is stored in the alpha channel of the normal map:
        currHeight = ComputePerPixelHeightDisplacement(texOffsetCurrent, lod, uv);
    }

    // Found below and above points, now perform line interesection (ray) with piecewise linear heightfield approximation

    // Refine the search with secant method

    float pt0 = rayHeight + stepSize;
    float pt1 = rayHeight;
    float delta0 = pt0 - prevHeight;
    float delta1 = pt1 - currHeight;

    float delta;
    vec2 offset;

    // Secant method to affine the search
    // Ref: Faster Relief Mapping Using the Secant Method - Eric Risser
    for (int i = 0; i < 3; ++i)
    {
        // intersectionHeight is the height [0..1] for the intersection between view ray and heightfield line
        float intersectionHeight = (pt0 * delta1 - pt1 * delta0) / (delta1 - delta0);
        // Retrieve offset require to find this intersectionHeight
        offset = (1.0 - intersectionHeight) * texOffsetPerStep * float(numSteps);

        currHeight = ComputePerPixelHeightDisplacement(offset, lod, uv);

        delta = intersectionHeight - currHeight;

        if (abs(delta) <= 0.01)
        break;

        // intersectionHeight < currHeight => new lower bounds
        if (delta < 0.0)
        {
            delta1 = delta;
            pt1 = intersectionHeight;
        }
        else
        {
            delta0 = delta;
            pt0 = intersectionHeight;
        }
    }

    outHeight = currHeight;

    // Fade the effect with lod (allow to avoid pop when switching to a discrete LOD mesh)
    offset *= (1.0 - max(min(lod - lodThreshold, 1), 0));

    return offset;
}