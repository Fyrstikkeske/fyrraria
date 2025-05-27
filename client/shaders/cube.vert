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

uniform float gridX, gridY, gridZ;

void main()
{
    // Normalize coordinates to [0, 1]
    float u = (aPos.x) / gridX; // θ parameter
    float v = (aPos.y) / (gridY * 8.0); // φ parameter
    float w = (aPos.z) / gridZ; // Toroidal thickness

    // Calculate angles
    float theta = 2.0 * 3.1415926535 * u;
    float phi = 2.0 * 3.1415926535 * w;

    // Calculate radii with aspect ratio preservation
    float majorCircumference = gridX; // Total X-length around major circle
    float minorCircumference = gridZ; // Total Y-length around minor circle
    float majorRadius = majorCircumference / (2.0 * 3.1415926535); // Major radius
    float minorRadius = minorCircumference / (2.0 * 3.1415926535);

    float effective_r = minorRadius * exp(v ) / 2.0;

    // Torus parametrization
    vec3 pos;
    pos.x = (majorRadius + effective_r * cos(phi)) * cos(theta);
    pos.y = (majorRadius + effective_r * cos(phi)) * sin(theta);
    pos.z = effective_r * sin(phi);

    gl_Position = projection * view * model * vec4(pos, 1.0);
    textureHandle = uvec2(handleLo, handleHi);
    TexCoord = aTexCoord;
}