
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aTangent;

out vec2 texCoord;
out vec3 normalWS;
out vec3 normalVS;
out vec3 positionWS;
out vec4 shadowCoords;
out mat3 TBN;

layout (std140, binding = 0) uniform Matrices
{
    //show base alignment and offset in comment
    mat4 _view;       //64 0
    mat4 _projection; //64 64
};

uniform bool _isInstancing = false;

uniform mat4 _model;

void main()
{
   mat4 modelMatrix = _model;
   if(_isInstancing)
   {
  //    modelMatrix = _models[gl_InstanceID];
   }
   gl_Position = _projection * _view * modelMatrix * vec4(aPos, 1.0);
   normalWS = mat3(transpose(inverse(modelMatrix))) * aNormal;
   normalVS = mat3(_view) * normalWS;
   positionWS = vec3(modelMatrix * vec4(aPos, 1.0));
   
   vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
   vec3 N = normalize(vec3(modelMatrix * vec4(aNormal, 0.0)));
   T = normalize(T - dot(T, N) * N);
   vec3 B = cross(N, T);
   
   //TBN can transform any vector from tangent space to world space
   TBN = mat3(T, B, N);
   
   texCoord = aTexCoord;
}