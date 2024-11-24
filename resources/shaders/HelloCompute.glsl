#version 460 core

uniform sampler2D imgToRead;
layout(rgba32f, binding = 1) uniform image2D imgOutput;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec4 color = texture(imgToRead, texelCoord / vec2(1440.0, 960.0)).rgba;
    if(texelCoord.x < 720)
    {
        imageStore(imgOutput, texelCoord, vec4(1,1,1,1));
    }
    else
    {
        imageStore(imgOutput, texelCoord, vec4(color.rgb,1));
    }
}