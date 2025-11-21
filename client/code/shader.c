
#include <gl.h>
#include <stdio.h>
#include "shader.h"

void get_GL_uniform_locations(int shaderProgram, int locations[]){
    int modelLocaction = glGetUniformLocation(shaderProgram, "model");
    int viewLocation = glGetUniformLocation(shaderProgram, "view");
    int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    int gridXLocation = glGetUniformLocation(shaderProgram, "gridX");
    int gridYLocation = glGetUniformLocation(shaderProgram, "gridY");
    int gridZLocation = glGetUniformLocation(shaderProgram, "gridZ");
    int lodLocation = glGetUniformLocation(shaderProgram, "lod");

    if (modelLocaction == -1) {
        printf("cant find model in shader location \n");
    }
    if (viewLocation == -1) {
        printf("cant find view in shader location \n");
    }
    if (projectionLocation == -1) {
        printf("cant find projection in shader location \n");
    }
    if (gridXLocation == -1) {
        printf("cant find gridXLocation in shader location \n");
    }
    if (gridYLocation == -1) {
        printf("cant find gridYLocation in shader location \n");
    }
    if (gridZLocation == -1) {
        printf("cant find gridZLocation in shader location \n");
    }
    if (lodLocation == -1) {
        printf("cant find lod in shader location \n");
    }

    locations[MODEL] = modelLocaction;
    locations[VIEW] = viewLocation;
    locations[PROJECTION] = projectionLocation;
    locations[GRID_X] = gridXLocation;
    locations[GRID_Y] = gridYLocation;
    locations[GRID_Z] = gridZLocation;
    locations[LOD] = lodLocation;
}
