
out vec3 FragColor;
in vec2 texCoord;
#define A 0.15
#define B 0.50
#define C 0.10
#define D 0.20
#define E 0.02
#define F 0.30
#define W 11.2

vec3 Tonemap(vec3 x)
{
    return ((x * ( A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E/F;
}

float Gaussian(float v, float r)
{
    return 1.0 / sqrt(2.0 * PI * v) * exp(-(r*r)/(2.0*v));
}

vec3 R(float r)
{
    return vec3(0.0, 0.0, 0.0)
        + Gaussian(0.0064, r) * vec3(0.233, 0.455, 0.649)
        + Gaussian(0.0484, r) * vec3(0.100, 0.336, 0.344)
        + Gaussian(0.187, r) * vec3(0.118, 0.198, 0.0)
        + Gaussian(0.567, r) * vec3(0.113, 0.007, 0.007)
        + Gaussian(1.99, r) * vec3(0.358, 0.004, 0.0)
        + Gaussian(7.41, r) * vec3(0.078, 0.0, 0.0);
}

vec3 GenSkinLUT(vec2 uv)
{
    float nol = uv.x;
    float inv_r = uv.y;
    float theta = acos(nol * 2.0 - 1.0);
    float r = 1.0 / inv_r;

    vec3 scatteringFactor = vec3(0.0, 0.0, 0.0);
    vec3 normalizationFactor = vec3(0.0, 0.0, 0.0);
    for (float x = -PI / 2.0; x < PI / 2.0; x += PI * 0.001)
    {
        float dist = 2.0 * r * sin(x * 0.5);

        scatteringFactor += max(0.0, cos(theta + x)) * R(dist);

        normalizationFactor += R(dist);
    }

    vec3 result = scatteringFactor / normalizationFactor;

    vec3 tonedResult = Tonemap(result * 12.0);
    vec3 whiteScale = 1.0 / Tonemap(vec3(W, W, W));

    //tonedResult = tonedResult * whiteScale;
    //tonedResult.x = pow(tonedResult.x, 1.0 / 2.2);
    //tonedResult.y = pow(tonedResult.y, 1.0 / 2.2);
    //tonedResult.z = pow(tonedResult.z, 1.0 / 2.2);

    return tonedResult;
}

// ----------------------------------------------------------------------------
void main()
{
  
    FragColor = vec3(GenSkinLUT(texCoord));
}