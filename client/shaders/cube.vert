#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform vec3 blockposition; // Normalized grid position [0,1]
uniform mat4 view;
uniform mat4 projection;
uniform float gridX;        // Dynamic grid width (e.g., 400)
uniform float gridY;        // Dynamic grid height (e.g., 42)

const float PI = 3.14159265359;
const float major_radius = 120.0;
const float minor_radius = 40.0;

void main()
{
    // Calculate theta (major angle) and phi (minor angle)
    float theta = (blockposition.x + (aPos.x)) * (2.0 * PI) / gridX;
    float phi = (blockposition.y + (aPos.y)) * (2.0 * PI) / gridY;

    // Adjust minor radius dynamically (optional)
    float dynamic_minor_radius = minor_radius * exp(aPos.z * blockposition.z);

    // Torus parameterization
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);
    float cos_phi = cos(phi);
    float sin_phi = sin(phi);

    vec4 torus = vec4(1.0);
    torus.x = (major_radius + dynamic_minor_radius * cos_phi) * cos_theta;
    torus.y = (major_radius + dynamic_minor_radius * cos_phi) * sin_theta;
    torus.z = dynamic_minor_radius * sin_phi;

    gl_Position = projection * view * torus;
    TexCoord = aTexCoord;
}