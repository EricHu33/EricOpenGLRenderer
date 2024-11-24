
layout (location = 0)out float FragColor;
in vec2 texCoord;

uniform sampler2D _SSAOTex;
uniform vec2 _res;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(_SSAOTex, 0));
    float result = 0.0;
    for(int i = -2; i < 2; i++)
    {
        for(int j = -2; j < 2; j++)
        {
           vec2 offset = vec2(float(i),float(j)) * texelSize;
           result += texture(_SSAOTex, texCoord + offset).r;
        }
    }
    float scale = 1 / 16.0;
    result *= scale;
    
    FragColor = result;
 
}