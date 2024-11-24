layout (location = 0) out vec4 FragColor;
in vec3 localPos;

uniform sampler2D _equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    //0.1591 = 1/6.28319 = 1.0 / (2.0 * PI)
    //0.3183 = 1/3.14159 = 1.0 / PI
    //https://paulbourke.net/geometry/transformationprojection/
    
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(localPos));
    vec3 color = texture(_equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}


