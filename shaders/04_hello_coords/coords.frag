#version 450
layout(location = 0) out vec4 FragColor;
layout(location = 2) in vec2 TexCoord;

layout(binding = 0) uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}