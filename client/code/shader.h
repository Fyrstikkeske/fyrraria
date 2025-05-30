#pragma once

#include <gl.h>
#include <stdio.h>

enum uniformlocations : char{
    model,
    view,
    projection,
    gridX,
    gridZ,
};

void getGLuniformlocations(int shaderProgram, int locations[]){
    int modelLocaction = glGetUniformLocation(shaderProgram, "model");
    int viewLocation = glGetUniformLocation(shaderProgram, "view");
    int projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    int gridXLocation = glGetUniformLocation(shaderProgram, "gridX");
    int gridZLocation = glGetUniformLocation(shaderProgram, "gridZ");

    if (viewLocation == -1) {
        printf("cant find view in shader location \n");
    }
    if (projectionLocation == -1) {
        printf("cant find projection in shader location \n");
    }
    if (gridXLocation == -1) {
        printf("cant find gridXLocation in shader location \n");
    }
    if (gridZLocation == -1) {
        printf("cant find gridZLocation in shader location \n");
    }
    if (modelLocaction == -1) {
        printf("cant find model in shader location \n");
    }

    locations[model] = modelLocaction;
    locations[view] = viewLocation;
    locations[projection] = projectionLocation;
    locations[gridX] = gridXLocation;
    locations[gridZ] = gridZLocation;
}