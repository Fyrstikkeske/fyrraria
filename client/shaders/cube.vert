#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in uint handleLo;
layout (location = 3) in uint handleHi;

out vec2 TexCoord;
flat out uvec2 textureHandle;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float gridX, gridZ;

void main()
{
    // Normalize input to [0, 1]
    float u = aPos.x / gridX; // major angle around torus
    float w = aPos.z / gridZ; // minor circle angle

    // Angles
    float theta = 2.0 * 3.1415926535 * u; // major angle
    float phi   = 2.0 * 3.1415926535 * w; // minor angle

    // Radii
    float majorRadius = gridX / (2.0 * 3.1415926535); // donut center distance
    float minorRadius = gridZ / (2.0 * 3.1415926535); // donut tube radius

    float effective_r = minorRadius + aPos.y;

    // Parametrize torus
    vec3 pos;
    pos.x = (majorRadius + effective_r * cos(phi)) * cos(theta);
    pos.y = (majorRadius + effective_r * cos(phi)) * sin(theta);
    pos.z = effective_r * sin(phi);

    gl_Position = projection * view * model * vec4(pos, 1.0);
    textureHandle = uvec2(handleLo, handleHi);
    TexCoord = aTexCoord;
}