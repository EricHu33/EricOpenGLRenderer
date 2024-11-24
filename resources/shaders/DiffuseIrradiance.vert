layout (location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 _projection;
uniform mat4 _view;

void main()
{
    localPos = aPos;  
    gl_Position =  _projection * _view * vec4(aPos, 1.0);
}