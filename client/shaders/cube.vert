#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 gridCoord;
uniform float gridX, gridY, gridZ;

void main()
{
    // Normalize coordinates to [0, 1]
    float u = (gridCoord.x + aPos.x) / gridX; // θ parameter
    float v = (gridCoord.y + aPos.y) / gridY; // φ parameter
    float w = (gridCoord.z + aPos.z) / gridZ; // Toroidal thickness

    // Calculate angles
    float theta = 2.0 * 3.1415926535 * u;
    float phi = 2.0 * 3.1415926535 * v;

    // Calculate radii with aspect ratio preservation
    float majorCircumference = gridX; // Total X-length around major circle
    float minorCircumference = gridY; // Total Y-length around minor circle
    float R = majorCircumference / (2.0 * 3.1415926535); // Major radius
    float r = minorCircumference / (2.0 * 3.1415926535); // Minor radius

    // Incorporate Z-coordinate as radial expansion
    float effective_r = r * (1.0 + w); // Scale minor radius with Z

    // Torus parametrization
    vec3 pos;
    pos.x = (R + effective_r * cos(phi)) * cos(theta);
    pos.y = (R + effective_r * cos(phi)) * sin(theta);
    pos.z = effective_r * sin(phi);

    gl_Position = projection * view * model * vec4(pos, 1.0);
    TexCoord = aTexCoord;
}