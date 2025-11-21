#pragma once

void get_GL_uniform_locations(int shaderProgram, int locations[]);

enum uniformlocations : char{
    MODEL,
    VIEW,
    PROJECTION,
    GRID_X,
    GRID_Y,
    GRID_Z,
    LOD,
};