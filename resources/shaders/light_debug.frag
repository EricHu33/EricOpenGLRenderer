
out vec4 FragColor;
in vec2 texCoord;

float hash11(float p)
{
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash12(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

uniform int numberOfTilesX;
uniform int totalLightCount;

void main() {
    ivec2 location = ivec2(gl_FragCoord.xy);
    ivec2 tileID = location / ivec2(16, 16);
    uint index = tileID.y * numberOfTilesX + tileID.x;

    uint offset = index * 300;

    FragColor = vec4(0, 0, 0, 1.0);
    float heatLevel = float(_perTileLightCount[index]) / float(600);
    vec3 mapped = vec3(1.0) - exp(-vec3(heatLevel, heatLevel, 0) *  8.2f);
    FragColor = vec4(mapped, 1);        
}