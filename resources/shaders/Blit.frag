out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D _baseMap;
uniform bool _performGammaCorrection = true;

void main()
{
    vec3 baseColor = texture(_baseMap, texCoord).rgb;
    
    if(_performGammaCorrection)
    {
        float gamma = 2.2;
        baseColor = pow(baseColor, vec3(1.0 / gamma));
    }
    FragColor = vec4(baseColor, 1.0);
}