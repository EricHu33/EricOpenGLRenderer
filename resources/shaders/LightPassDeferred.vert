
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

//out vec2 texCoord;

layout (std140) uniform Matrices
{
    //show base alignment and offset in comment
    mat4 _view;       //Matrices64 0
    mat4 _projection; //64 64
};

uniform mat4 _model;

void main()
{
   //texCoord = aTexCoord;
   gl_Position = _projection * _view * _model * vec4(aPos, 1.0);
}