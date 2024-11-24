
out vec4 FragColor;
in vec2 texCoord;
in vec4 clipPos;

uniform sampler2DArray _baseMap;
uniform mat4 _projectionMatrix;
uniform int _layer;

void main()
{
   float  finalColor = texture(_baseMap, vec3(texCoord, _layer)).r;
   FragColor = vec4(vec3(finalColor), 1.0);
}