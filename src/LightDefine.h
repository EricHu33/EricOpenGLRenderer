#ifndef LIGHT_DEFINE_H
#define LIGHT_DEFINE_H

#include <string>
#include <glm/detail/type_vec.hpp>

struct DirectionalLight {
    glm::vec3 Direction;
    glm::vec3 Ambient;
    glm::vec3 Color;
};

struct PointLight {
    //position.w = radius
    glm::vec4 Position =  glm::vec4(0, 0, 0, 1);
    //color.w == intensity
    glm::vec4 Color = glm::vec4(1);
};

struct SpotLight
{
    glm::vec3 Position;
    glm::vec3 Direction;
    float Cutoff;
    float OuterCutOff;
    glm::vec3 Ambient;
    glm::vec3 Color;
    //attenuation terms
    //att = 1.0 / (constant + linear * dist + quadratic * (dist * dist))
    float constant;
    float linear;
    float quadratic;
};

#endif // LIGHT_DEFINE_H