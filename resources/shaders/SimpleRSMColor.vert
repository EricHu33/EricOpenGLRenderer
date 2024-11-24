
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 weights;


const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout (std140) uniform Matrices
{
//show base alignment and offset in comment
    mat4 _view;       //64 0
    mat4 _projection; //64 64
};

layout(std140, binding = 1) uniform ShadowMatrices
{
    mat4 _shadowMatrix[5];
};

layout(std140, binding = 3) uniform BonesMatrices
{
    mat4 finalBonesMatrices[MAX_BONES];
};

layout(std140, binding = 5) uniform RSMMatrices
{
    mat4 _rsmMatrix[1];
};

out vec2 texCoord;
out vec3 positionWS;
out vec3 normalWS;

uniform bool _isSkinnedMesh;
uniform mat4 _model;
uniform int _cascadeIndex;

void main()
{
    vec4 totalPosition = vec4(aPos, 1.0f);
    vec3 localNormal = aNormal;
    
    if(_isSkinnedMesh)
    {
        totalPosition = vec4(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(boneIds[i] == -1)
            continue;
            if(boneIds[i] >= MAX_BONES)
            {
                totalPosition = vec4(aPos,1.0f);
                break;
            }
            vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(aPos, 1.0f);
            totalPosition += localPosition * weights[i];
            localNormal = mat3(finalBonesMatrices[boneIds[i]]) * aNormal;
        }
    }
    gl_Position =  _rsmMatrix[0] * _model * totalPosition;

    positionWS = vec3(_model * totalPosition);
    normalWS = mat3(transpose(inverse(_model))) * localNormal;
    texCoord = aTexCoord;
}