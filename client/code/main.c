#include "meshing.h"
#include "shader.h"
#include "utils.h"
#include "worldgen.h"
#include <stdint.h>

#define GLAD_GL_IMPLEMENTATION
#define RGFW_IMPLEMENTATION
#define RGL_LOAD_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <RGFW.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stb_image.h>
#include <math.h>


void keyfunc(RGFW_window* win, unsigned char key, unsigned char keyChar, unsigned char keyMod, unsigned char pressed) {
    if (key == RGFW_escape && pressed) {
        RGFW_window_setShouldClose(win, True);
    }
}

int main() {
    char windowTitle[256] = "Fyrraria"; //For safety reason must do it like this.
    RGFW_window* win = RGFW_createWindow(windowTitle, RGFW_RECT(0, 0, 800, 600), RGFW_windowCenter);
    gladLoadGL((GLADloadfunc) RGFW_getProcAddress);

    stbi_set_flip_vertically_on_load(1);

    if (!win) {
        fprintf(stderr, "Failed to create window or OpenGL context.\n");
        return -1;
    }

    player player;
    glm_vec3_copy((vec3){ 0.0, 1.0, 0.0 }, player.position);


    struct block *world[Worldx*Worldy*Worldz];

    genworld(Worldx, Worldy, Worldz, chunksize, world);

    int renderdistance = Worldx*Worldy*Worldz;
    unsigned int VAOs[renderdistance], VBOs[renderdistance], VBOsSize[renderdistance];
    glGenVertexArrays(renderdistance, VAOs);
    glGenBuffers(renderdistance,VBOs);
    int shaderProgram = makeshaderprogram();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    uint64_t handles [] ={
        [grass] = loadTextureIntoShaderBindless(shaderProgram, "client/textures/grass.png"),
        [test] = loadTextureIntoShaderBindless(shaderProgram, "client/textures/tnt_test.png"),
        [leaf] = loadTextureIntoShaderBindless(shaderProgram, "client/textures/leaf.png"),
        [woodlog] = loadTextureIntoShaderBindless(shaderProgram, "client/textures/log.png"),
    };

    generatemeshs(renderdistance, chunksize,Worldx, Worldy, Worldz, world, VAOs, VBOs, VBOsSize, handles);
    

    vec3 cameraFront = { 0.0, 0.0, -1.0 };

    float yaw = -90.0f;
    float pitch = 0.0f;
    const float sensitivity = 0.1f;

    RGFW_window_showMouse(win, 0);
    RGFW_window_mouseHold(win, RGFW_AREA(win->r.w / 2, win->r.h / 2));
    RGFW_setKeyCallback(keyfunc);

    int locations[5];
    getGLuniformlocations(shaderProgram, locations);

    glUniform1f(locations[gridX], Worldx * chunksize);
    glUniform1f(locations[gridZ], Worldz * chunksize);

    u32 fps = 0;
    u32 frames = 0;
    const double startTime = RGFW_getTime();
    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        while (RGFW_window_checkEvent(win) != NULL) {
            if (win->event.type == RGFW_quit) break;
            
            if (win->event.type == RGFW_mousePosChanged) {
                yaw -= win->event.vector.x * sensitivity;
                pitch -= win->event.vector.y * sensitivity;
            };

            if (win->event.type == RGFW_focusIn) {
                RGFW_window_showMouse(win, 0);
                RGFW_window_mouseHold(win, RGFW_AREA(win->r.w/2, win->r.h/2));
            };

            if (win->event.type == RGFW_windowResized){
                glViewport(0, 0, win->r.w, win->r.h);
            }
        }

        vec3 playersNormalisedNormalForCameraUse;
        analyticalFormulaForTorusNormalPoint(player.position, playersNormalisedNormalForCameraUse, Worldx, Worldz, chunksize);
        
        vec3 transformedplayerposition;
        transformPositionFromLocalToGlobalSpaceUsingLinearTorusMethod(player.position, transformedplayerposition, Worldx, Worldz, chunksize);

        vec3 tangent;
        vec3 bitangent;

        getTorusSurfaceFrameAtPosition(player.position, tangent, bitangent, Worldx, Worldz, chunksize);

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
        float yawRad   = glm_rad(yaw);
        float pitchRad = glm_rad(pitch);

        vec3 cameraFrontLocal = {
            cosf(pitchRad) * cosf(yawRad),   // along tangent
            sinf(pitchRad),                  // along local “up” (normal)
            cosf(pitchRad) * sinf(yawRad)    // along bitangent
        };

        glm_vec3_zero(cameraFront);
        glm_vec3_muladds(tangent, cameraFrontLocal[0], cameraFront);
        glm_vec3_muladds(playersNormalisedNormalForCameraUse, cameraFrontLocal[1], cameraFront);
        glm_vec3_muladds(bitangent, cameraFrontLocal[2], cameraFront);
        glm_vec3_normalize(cameraFront);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        movementcode(win, cameraFrontLocal, player.position);

        char buffer[256];
        sprintf(buffer, "Fyrraria: %d", fps);
        const char* fps_str = buffer;
        RGFW_window_setName(win, fps_str);

        mat4 viewmatrix;
        vec3 center;
        glm_vec3_add(transformedplayerposition, cameraFront, center);
        glm_lookat(transformedplayerposition, center, playersNormalisedNormalForCameraUse, viewmatrix);
        glUniformMatrix4fv(locations[view], 1, GL_FALSE, (const float *)viewmatrix);

        mat4 projectionmatrix;
        glm_perspective(glm_rad(70), (float)win->r.w/win->r.h, 0.1, 100000.0, projectionmatrix);
        glUniformMatrix4fv(locations[projection], 1, GL_FALSE, (const float *)projectionmatrix);

        mat4 modelMatrix = GLM_MAT4_IDENTITY_INIT;
        glUniformMatrix4fv(locations[model], 1, GL_FALSE, (const float *)modelMatrix);
        
        


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        for (int worlditer = 0; worlditer < Worldx*Worldy*Worldz; worlditer++){
            if (VBOsSize[worlditer] == 0){continue;}
            glBindVertexArray(VAOs[worlditer]);
            glBindBuffer(GL_ARRAY_BUFFER, VBOs[worlditer]);
            glDrawArrays(GL_TRIANGLES, 0, VBOsSize[worlditer]);
        }
        
        RGFW_window_swapBuffers(win);
        fps = RGFW_checkFPS(startTime, frames, 60);
        frames++;
    }

    RGFW_window_close(win);
    return 0;
}
