
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 TexCoords;

uniform samplerCube _skybox;

void main()
{
    FragColor = textureLod(_skybox, normalize(TexCoords), 0);
    BrightColor = vec4(0,0,0,1);
}