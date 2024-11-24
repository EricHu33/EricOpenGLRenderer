
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D _baseMap;
uniform sampler2D _ssaoMap;
uniform sampler2D _bloomBlurMap;

uniform bool _useBloom;
uniform float _exposure;
uniform float _enableToneMapping;

void main()
{
   vec3 baseColor = texture(_baseMap, texCoord).rgb;
   vec3 finalColor = baseColor;
   if(_useBloom)
   {
      finalColor += texture(_bloomBlurMap, texCoord).rgb;
   }
   vec3 mapped = finalColor;
   if(_enableToneMapping > 0)
   {
       // exposure tone mapping
       mapped = vec3(1.0) - exp(-finalColor * _exposure);
   }
   
   FragColor = vec4(mapped.rgb, 1.0);
}