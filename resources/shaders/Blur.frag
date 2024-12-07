
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D _baseMap;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    vec2 tex_offset = 1.0 / textureSize(_baseMap, 0); // gets size of single texel
    vec3 result = texture(_baseMap, texCoord).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(_baseMap, texCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(_baseMap, texCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    //vertical
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(_baseMap, texCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(_baseMap, texCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
   
   FragColor = vec4(result.rgb, 1.0);
}