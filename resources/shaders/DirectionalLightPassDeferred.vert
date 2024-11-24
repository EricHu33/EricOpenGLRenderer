
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 _model;
uniform mat4 _view;
uniform mat4 _projection;

void main()
{
   //texCoord = aTexCoord;
   gl_Position = _projection * _view * _model * vec4(aPos, 1.0);
}