
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 texCoord;
out vec4 clipPos;

void main()
{
   texCoord = aTexCoord;
   clipPos = vec4(aPos, 1);
   gl_Position = vec4(aPos, 1.0);
}