
const float CosineA0 = 3.141592653589793f;
const float CosineA1 = 2.094395f ;//(2.0f * 3.141592653589793f) / 3.0f;
const float CosineA2 = 0.7853f;//3.141592653589793f / 4.0f;
const float SkinK0 = 3.5449077f;
const float SkinK1 = 2.0466534f;
const float SkinK2 = 1.5853309f;
struct SH9Color
{
    vec3 c[9];
};

struct SHFloat
{
    float c[9];
};

SHFloat ProjectOntoSH9Color(in vec3 n, in float A0, in float A1, in float A2)
{
    SHFloat sh;

    // Band 0
    sh.c[0] = 0.282095f * A0;

    // Band 1
    sh.c[1] = -0.488603f * n.y * A1;
    sh.c[2] = 0.488603f * n.z * A1;
    sh.c[3] = -0.488603f * n.x * A1;

    // Band 2
    sh.c[4] = 1.092548f * n.x * n.y * A2;
    sh.c[5] = -1.092548f * n.y * n.z * A2;
    sh.c[6] = 0.315392f * (3.0f * n.z * n.z - 1.0f) * A2;
    sh.c[7] = -1.092548f * n.x * n.z * A2;
    sh.c[8] = 0.546274f * (n.x * n.x - n.y * n.y) * A2;

    return sh;
}

SH9Color ProjectOntoSH9Color3(in vec3 n, in vec3 A0, in vec3 A1, in vec3 A2)
{
    SH9Color sh;
    
    // Band 0
    sh.c[0] = 0.282095f * A0 * SkinK0;

    // Band 1
    sh.c[1] = -0.488603f * n.y * A1 * SkinK1;
    sh.c[2] = 0.488603f * n.z * A1 * SkinK1;
    sh.c[3] = -0.488603f * n.x * A1 * SkinK1;

    // Band 2
    sh.c[4] = 1.092548f * n.x * n.y * A2 * SkinK2;
    sh.c[5] = -1.092548f * n.y * n.z * A2 * SkinK2;
    sh.c[6] = 0.315392f * (3.0f * n.z * n.z - 1.0f) * A2 * SkinK2;
    sh.c[7] = -1.092548f * n.x * n.z * A2 * SkinK2;
    sh.c[8] = 0.546274f * (n.x * n.x - n.y * n.y) * A2 * SkinK2;

    return sh;
}

vec3 SHDotProduct(in SHFloat a, in SH9Color b)
{
    vec3 result = vec3(0, 0, 0);

    for(int i = 0; i < 9; ++i)
    {
        result += (a.c[i] * b.c[i]);
    }
    return result;
}

vec3 SHDotProduct3(in SH9Color a, in SH9Color b)
{
    vec3 result = vec3(0, 0, 0);
    
    for(int i = 0; i < 9; ++i)
    {
        result += a.c[i] * b.c[i];
    }
    return result;
}

vec3 EvalSH9Irradiance(in vec3 dir, in SH9Color sh)
{
    SHFloat dirSH = ProjectOntoSH9Color(dir, CosineA0, CosineA1, CosineA2);
    return SHDotProduct(dirSH, sh);
}

vec3 EvalSKinSH9Irradiance(in vec3 dir, in SH9Color sh, vec3 a0, vec3 a1, vec3 a2)
{
    SH9Color dirSH = ProjectOntoSH9Color3(dir, a0, a1, a2);
    return SHDotProduct3(dirSH, sh);
    //return vec3(0,0,0);
}