layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in ivec4 boneIds;
layout (location = 5) in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

layout(std140, binding = 3) uniform BonesMatrices
{
   mat4 finalBonesMatrices[MAX_BONES];
};

out vec2 texCoord;
out vec3 positionWS;
out vec3 positionVS;
out vec3 normalWS;
out vec3 normalVS;
//out vec4 shadowCoords;
out mat3 TBN;
out mat3 viewMatrix;

uniform mat4 _model;
uniform mat4 _view;  
uniform mat4 _projection; 
uniform bool _isSkinnedMesh;

void main()
{
   vec3 localNormal = aNormal;
   vec4 totalPosition = vec4(aPos, 1.0f);
   
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
  
   gl_Position = _projection * _view * _model * totalPosition;
   
   vec3 T = normalize(vec3(_model * vec4(aTangent, 0.0)));
   vec3 N = normalize(vec3(_model * vec4(localNormal, 0.0)));
   T = normalize(T - dot(T, N) * N);
   vec3 B = cross(N, T);
      
   //TBN can transform any vector from tangent space to world space
   TBN = mat3(T, B, N);
   viewMatrix = mat3(_view);
   
   positionWS = vec3(_model * totalPosition);
   positionVS = (_view * vec4(positionWS, 1)).xyz;
   normalWS = mat3(transpose(inverse(_model))) * localNormal;
   normalVS = viewMatrix * normalWS;
   texCoord = aTexCoord;
}