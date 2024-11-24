
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gFlux;

in vec2 texCoord;
in vec3 positionWS;
in vec3 normalWS;

//0
uniform sampler2D _baseMap;
uniform float _uvScale;
uniform vec4 _baseColor;

void main()
{
    vec2 uv = texCoord;
    vec4 albedo = texture(_baseMap, uv).rgba;
    
    gPosition = positionWS;
    gNormal = normalize(normalWS);
    gFlux = albedo.rgb;
}
