
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 _view;
uniform mat4 _projection;

void main()
{
    TexCoords = vec3(aPos.xy, aPos.z);
    vec4 pos = _projection * _view  * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}