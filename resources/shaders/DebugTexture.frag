
out vec4 FragColor;
in vec2 texCoord;
in vec4 clipPos;

uniform sampler2D _baseMap;
uniform int _layer;
uniform bool _debugCompute;
uniform mat4 _projectionMatrix;
void main()
{
      FragColor = vec4( pow(texture(_baseMap, texCoord).rgb, vec3(1.0/2.2)), 1.0);
}