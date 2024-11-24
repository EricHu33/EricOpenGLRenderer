#pragma once

static const float CosineA0 = 3.141592653589793f;
static const float CosineA1 = (2.0f * 3.141592653589793f) / 3.0f;
static const float CosineA2 = 3.141592653589793f / 4.0f;

struct SH9
{
    float Coefficients[9];
};

struct SH9Color
{
    glm::vec3 Coefficients[9];

    SH9Color& operator+=(const SH9Color& other)
    {
        for(int i = 0; i < 9; ++i)
            Coefficients[i] += other.Coefficients[i];
        return *this;
    }

    SH9Color operator+(const SH9Color& other) const
    {
        SH9Color result;
        for(int i = 0; i < 9; ++i)
            result.Coefficients[i] = Coefficients[i] + other.Coefficients[i];
        return result;
    }

    SH9Color& operator-=(const SH9Color& other)
    {
        for(int i = 0; i < 9; ++i)
            Coefficients[i] -= other.Coefficients[i];
        return *this;
    }

    SH9Color operator-(const SH9Color& other) const
    {
        SH9Color result;
        for(int i = 0; i < 9; ++i)
            result.Coefficients[i] = Coefficients[i] - other.Coefficients[i];
        return result;
    }

    SH9Color& operator*=(const float& scale)
    {
        for(int i = 0; i < 9; ++i)
            Coefficients[i] *= scale;
        return *this;
    }

    SH9Color operator*(const SH9Color& other) const
    {
        SH9Color result;
        for(int i = 0; i < 9; ++i)
            result.Coefficients[i] = Coefficients[i] * other.Coefficients[i];
        return result;
    }

    SH9Color& operator*=(const SH9Color& other)
    {
        for(int i = 0; i < 9; ++i)
            Coefficients[i] *= other.Coefficients[i];
        return *this;
    }

    SH9Color operator*(const float& scale) const
    {
        SH9Color result;
        for(int i = 0; i < 9; ++i)
            result.Coefficients[i] = Coefficients[i] * scale;
        return result;
    }

    SH9Color& operator/=(const float& scale)
    {
        for(int i = 0; i < 9; ++i)
            Coefficients[i] /= scale;
        return *this;
    }

    SH9Color operator/(const float& scale) const
    {
        SH9Color result;
        for(int i = 0; i < 9; ++i)
            result.Coefficients[i] = Coefficients[i] / scale;
        return result;
    }

    SH9Color operator/(const SH9Color& other) const
    {
        SH9Color result;
        for(int i = 0; i < 9; ++i)
            result.Coefficients[i] = Coefficients[i] / other.Coefficients[i];
        return result;
    }

    SH9Color& operator/=(const SH9Color& other)
    {
        for(int i = 0; i < 9; ++i)
            Coefficients[i] /= other.Coefficients[i];
        return *this;
    }

};