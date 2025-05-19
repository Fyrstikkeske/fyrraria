#version 430 core

#extension GL_ARB_bindless_texture : require

out vec4 FragColor;

in vec2 TexCoord;



uniform sampler2D grass;

void main()
{
    FragColor = texture(grass, TexCoord);
}