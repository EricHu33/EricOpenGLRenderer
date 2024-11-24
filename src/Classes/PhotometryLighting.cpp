#include <cmath>

#include "PhotmetryLighting.h"
namespace PhotometryLighting
{
    float exposureSettings(float aperture, float shutterSpeed, float sensitivity) {
        return log2((aperture * aperture) / shutterSpeed * 100.0 / sensitivity);
    }

    float exposure(float ev100) {
        return 1.0 / (pow(2.0, ev100) * 1.2);
    }

    float lmToCandela(float lm)
    {
        return lm / 4 * 3.141592653589793f;
    }
}