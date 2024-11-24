
layout (location = 0) out vec4 FragColor;
in vec2 texCoord;
in vec3 ndcPos;
uniform vec3 _lightColor;
uniform sampler2D _cameraColorMap;
uniform vec2 _mousePos;

void main()
{
   vec3 color = vec3(0,0,1);
   //vec2 positionSS = vec2((ndcPos.x + 1) * 1440.0 * 0.5, (ndcPos.y + 1) * 960.0 * 0.5);
   vec2 screenUV = gl_FragCoord.xy / vec2(1440.0, 960.0);
   color = texture(_cameraColorMap, screenUV +( _mousePos - vec2(48,48)) / vec2(1440.0, 960.0) ).rgb;
   FragColor = vec4(color, 1.0);
}