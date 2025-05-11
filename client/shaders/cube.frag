#version 430 core

out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;



layout(binding = 0) uniform sampler2D texture1; 
layout(binding = 0) uniform sampler2D texture2;

void main()
{
    vec4 color1 = texture(texture1, TexCoord);
    vec4 color2 = texture(texture2, TexCoord);
    FragColor = mix(color1, color2, 0.3); // Blend textures
}