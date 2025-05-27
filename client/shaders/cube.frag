#version 450 core
#extension GL_ARB_bindless_texture : require

flat in uvec2 textureHandle;
in vec2 TexCoord;

out vec4 FragColor;

void main() {
    sampler2D tex = sampler2D(textureHandle);
    FragColor = texture(tex, TexCoord);
}