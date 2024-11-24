#pragma once

namespace PhotometryLighting
{
    float exposureSettings(float aperture, float shutterSpeed, float sensitivity);

    // Computes the exposure normalization factor from
    // the camera's EV100
    float exposure(float ev100);

    float lmToCandela(float lm);
}
