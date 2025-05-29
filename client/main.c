#include "cglm/cam.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "cglm/vec3.h"

#define GLAD_GL_IMPLEMENTATION
#define RGFW_IMPLEMENTATION
#define RGL_LOAD_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <gl.h>
#include <RGFW.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stb_image.h>
#include <math.h>

const char vertexShaderSource[] = { 
    #embed "shaders/cube.vert" 
    , 0
};

const char fragmentShaderSource[] = { 
    #embed "shaders/cube.frag" 
    , 0
};

const int chunksize = 16;

const int Worldx = 40; // major ring
const int Worldy = 1; // world height
const int Worldz = 20; // minor ring

const int vertexSize = 3+2+1+1;

const int faceSize = vertexSize * 6;

const int blockSize = faceSize * 6;

struct shaderstruct{
    int shaderProgram;
    unsigned int VAO;
};

enum blocktype : char{
    grass,
    test,
    air,
    leaf,
    woodlog,
};

typedef struct {
    vec3 position;
} player;

struct block{
    enum blocktype type;
};

typedef struct {
    GLuint vbo;
    GLsizei vertexCount;
} RenderChunk;

void transformPositionFromLocalToGlobalSpaceUsingLinearTorusMethod(vec3 localPosition, vec3 globalPosition)
{
    float u = localPosition[0] / (Worldx * chunksize);
    float w = localPosition[2] / (Worldz * chunksize);

    float theta = 2.0 * M_PI * u;
    float phi   = 2.0 * M_PI * w;

    float majorRadius = (Worldx * chunksize) / (2.0 * M_PI);
    float minorRadius = (Worldz * chunksize) / (2.0 * M_PI);

    float effective_r = minorRadius + localPosition[1];

    float x = (majorRadius + effective_r * cos(phi)) * cos(theta);
    float y = (majorRadius + effective_r * cos(phi)) * sin(theta);
    float z = effective_r * sin(phi);

    glm_vec3_copy((vec3){x,y,z}, globalPosition);
};



void keyfunc(RGFW_window* win, unsigned char key, unsigned char keyChar, unsigned char keyMod, unsigned char pressed) {
    if (key == RGFW_escape && pressed) {
        RGFW_window_setShouldClose(win, True);
    }
}

void checkShaderCompilation(GLuint shader, const char* type) {
    GLint success;
    GLchar infoLog[1024];

    glad_glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glad_glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
        fprintf(stderr, "[ERROR] %s SHADER COMPILATION FAILED:\n%s\n", type, infoLog);
        exit(-1);
    }
}

unsigned long loadTextureIntoShaderBindless(int shaderProgram, const char* pathToFile) {
    int width, height, nrChannels;
    unsigned char *data = stbi_load(pathToFile, &width, &height, &nrChannels, 0);
    if (data == NULL) {
        printf("Failed to load texture from file!\n");
    }
    GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint64 handle = glGetTextureHandleARB(texture);
    glMakeTextureHandleResidentARB(handle);
    return handle;
}

int makeshaderprogram (){
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    const char* vsSources[] = { vertexShaderSource };
    glShaderSource(vertexShader, 1, vsSources, NULL);
    
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader, "VERTEX");

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fsSources[] = { fragmentShaderSource };
    glShaderSource(fragmentShader, 1, fsSources, NULL);
    
    glCompileShader(fragmentShader);
    checkShaderCompilation(fragmentShader, "FRAGMENT");


    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    

    glUseProgram(shaderProgram);


    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  

    return shaderProgram;
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
    glm_vec3_copy((vec3){ 0.0, 3.0, 0.0 }, player.position);
    
    

    struct block *world[Worldx*Worldy*Worldz];

    for (int chunkiter = 0; chunkiter < Worldx * Worldy * Worldz; chunkiter++) {
        world[chunkiter] = malloc(sizeof(struct block) * chunksize * chunksize * chunksize);
        if (!world[chunkiter]) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(EXIT_FAILURE);
        }

        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++) {
            world[chunkiter][blockiter].type = air;
    
            // Calculate chunk coordinates
            int chunkx = chunkiter % Worldx;
            int chunky = (chunkiter / Worldx) % Worldy;
            int chunkz = chunkiter / (Worldx * Worldy);
    
            // Calculate local block coordinates within the chunk
            int localx = blockiter % chunksize;
            int localy = (blockiter / chunksize) % chunksize;
            int localz = blockiter / (chunksize * chunksize);
    
            // Calculate global coordinates
            int globalx = chunkx * chunksize + localx;
            int globaly = chunky * chunksize + localy;
            int globalz = chunkz * chunksize + localz;

            /*
                        globalz -= 480;
            globalx -= 1200;
            */


            if (globaly == 0){
                world[chunkiter][blockiter].type = grass;
            }
            if (globaly == 7 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 3 && (globalz-globaly+globalx) % Worldx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 3 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 4 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 5 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 6 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 4 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 2 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 3 && globalx == 2){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 3 && globalx == 4){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 7 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
        }
    }

    //MAKE MESHESHEESA DSA
    // it are over 
    //TODO FIX THIS PIECE OF SHIT UP

    int renderdistance = Worldx*Worldy*Worldz;

    unsigned int VAOs[renderdistance], VBOs[renderdistance], VBOsSize[renderdistance];

    glGenVertexArrays(renderdistance, VAOs);

    glGenBuffers(renderdistance,VBOs);


    

    
    int shaderProgram = makeshaderprogram();
    glEnable(GL_DEPTH_TEST);



    uint64_t grasshandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/grass.png");
    uint64_t woodloghandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/log.png");
    uint64_t leafhandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/leaf.png");
    uint64_t tnt_testhandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/tnt_test.png");

    for (int iter = 0; iter < renderdistance; iter++){

        int meshableBlocks = 0;
        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++){
            if (world[iter][blockiter].type == air){continue;}
            meshableBlocks += 1;
        }
        
        VBOsSize[iter] = blockSize * meshableBlocks;
        if (meshableBlocks == 0){
            continue;
        }
        

        float worldmeshes[blockSize * meshableBlocks];

        int offset = 0;
        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++){
            enum blocktype block = world[iter][blockiter].type;
            if (block == air) continue;

            uint64_t textHandleToUse;

            if (block == grass){textHandleToUse = grasshandle;}
            if (block == woodlog){textHandleToUse = woodloghandle;}
            if (block == leaf){textHandleToUse = leafhandle;}
            if (block == test){textHandleToUse = tnt_testhandle;}

            // Calculate chunk coordinates
            int chunkx = iter % Worldx;
            int chunky = (iter / Worldx) % Worldy;
            int chunkz = iter / (Worldx * Worldy);
    
            // Calculate local block coordinates within the chunk
            int localx = blockiter % chunksize;
            int localy = (blockiter / chunksize) % chunksize;
            int localz = blockiter / (chunksize * chunksize);
    
            // Calculate global coordinates
            int globalx = chunkx * chunksize + localx;
            int globaly = chunky * chunksize + localy;
            int globalz = chunkz * chunksize + localz;

            uint32_t handleLo = (uint32_t)(textHandleToUse & 0xFFFFFFFF);
            uint32_t handleHi = (uint32_t)(textHandleToUse >> 32);

            float floatLo;
            float floatHi;
            memcpy(&floatLo, &handleLo, sizeof(uint32_t));
            memcpy(&floatHi, &handleHi, sizeof(uint32_t));

            float ypvertice[] = {
                globalx + 0.0f, globaly +  1.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi,
                globalx + 1.0f, globaly +  1.0f, globalz + 0.0f, 1.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f, globaly +  1.0f, globalz +  1.0f, 1.0f, 0.0f,floatLo,floatHi,
                globalx +1.0f, globaly +  1.0f, globalz +  1.0f, 1.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f, globaly +  1.0f, globalz +  1.0f, 0.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f, globaly +  1.0f, globalz + 0.0f, 0.0f, 1.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset] = ypvertice[localiter];
            }
            float ynvertice[] = {
               globalx + 0.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
               globalx +0.0f, globaly +0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx +0.0f, globaly +0.0f,  globalz +1.0f,  0.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize ] = ynvertice[localiter];
            }
            float xpvertice[] = {
                globalx +1.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
                globalx +1.0f,globaly +  1.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f,globaly + 0.0f,  globalz +1.0f,  0.0f, 0.0f,floatLo,floatHi,
                globalx +1.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize * 2] = xpvertice[localiter];
            }
            
            float xnvertice[] = {
                globalx +0.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +0.0f,globaly +  1.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
                globalx +0.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +0.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f,globaly + 0.0f,  globalz +1.0f,  0.0f, 0.0f,floatLo,floatHi,

            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize * 3] = xnvertice[localiter];
            }

            
            float zpvertice[] = {
              globalx +  0.0f, globaly +0.0f, globalz + 1.0f,  1.0f, 0.0f,floatLo,floatHi,
              globalx +  1.0f, globaly + 1.0f,globalz + 1.0f,  0.0f, 1.0f,floatLo,floatHi,
              globalx +  1.0f, globaly +0.0f, globalz + 1.0f,  1.0f, 1.0f,floatLo,floatHi,
              globalx +  1.0f, globaly + 1.0f,globalz + 1.0f,  0.0f, 1.0f,floatLo,floatHi,
              globalx + 0.0f, globaly +0.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
              globalx + 0.0f,  globaly +1.0f, globalz + 1.0f,  0.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize * 4] = zpvertice[localiter];
            }
            float znvertice[] = {
               globalx + 0.0f,globaly + 0.0f, globalz +0.0f,  1.0f, 0.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly +  1.0f,globalz + 0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly +  1.0f,globalz + 0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx +0.0f, globaly + 1.0f, globalz +0.0f,  0.0f, 0.0f,floatLo,floatHi,
               globalx + 0.0f, globaly +0.0f, globalz +0.0f,  1.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + (faceSize * 5)] = znvertice[localiter];
            }
        
            offset += blockSize;
        }

        glBindVertexArray(VAOs[iter]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[iter]);
        

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * blockSize * meshableBlocks, worldmeshes, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);  
    
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 7 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 7 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);

    }
    

    vec3 cameraPos = { 0.0, 0.0, 3.0 };
    vec3 cameraFront = { 0.0, 0.0, -1.0 };
    vec3 cameraUp = { 0.0, 1.0, 0.0 };

    float yaw = -90.0f;
    float pitch = 0.0f;
    const float sensitivity = 0.1f;

    RGFW_window_showMouse(win, 0);
    RGFW_window_mouseHold(win, RGFW_AREA(win->r.w / 2, win->r.h / 2));
    RGFW_setKeyCallback(keyfunc);

    glEnable(GL_CULL_FACE);

    vec3 tangent, bitangent;
    float horizontalSpeed = 0.1f;

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

        vec3 transformedplayerposition;
        transformPositionFromLocalToGlobalSpaceUsingLinearTorusMethod(player.position, transformedplayerposition);
        
        // This is the analytic normal direction corresponding to +y in local space
        float u = player.position[0] / (Worldx * chunksize);
        float w = player.position[2] / (Worldz * chunksize);
        float theta = 2.0 * M_PI * u;
        float phi   = 2.0 * M_PI * w;

        float cosPhi = cos(phi);
        float sinPhi = sin(phi);
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        
        vec3 playersNormalisedNormalForCameraUse = {
            cosPhi * cosTheta,
            cosPhi * sinTheta,
            sinPhi
        };

        {
            float delta = 0.1f;
            vec3 pos_plus_dx, pos_minus_dx, pos_plus_dz, pos_minus_dz;
            
            // Tangent (dx)
            vec3 local_plus_dx = { player.position[0] + delta, player.position[1], player.position[2] };
            vec3 local_minus_dx = { player.position[0] - delta, player.position[1], player.position[2] };
            transformPositionFromLocalToGlobalSpaceUsingLinearTorusMethod(local_plus_dx, pos_plus_dx);
            transformPositionFromLocalToGlobalSpaceUsingLinearTorusMethod(local_minus_dx, pos_minus_dx);
            glm_vec3_sub(pos_plus_dx, pos_minus_dx, tangent);
            glm_vec3_normalize(tangent);

            // Bitangent (dz)
            vec3 local_plus_dz = { player.position[0], player.position[1], player.position[2] + delta };
            vec3 local_minus_dz = { player.position[0], player.position[1], player.position[2] - delta };
            transformPositionFromLocalToGlobalSpaceUsingLinearTorusMethod(local_plus_dz, pos_plus_dz);
            transformPositionFromLocalToGlobalSpaceUsingLinearTorusMethod(local_minus_dz, pos_minus_dz);
            glm_vec3_sub(pos_plus_dz, pos_minus_dz, bitangent);
            glm_vec3_normalize(bitangent);

            // Ensure orthogonality
            vec3 recalc_bitangent;
            glm_vec3_cross(playersNormalisedNormalForCameraUse, tangent, recalc_bitangent);
            glm_vec3_normalize_to(recalc_bitangent, bitangent);
        }
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

        if (RGFW_isPressed(win, RGFW_controlL)) {
            horizontalSpeed = 1;
        }else {
            horizontalSpeed = 0.1;
        }
        vec3 horizontalFront = { cameraFrontLocal[0], 0.0f, cameraFrontLocal[2] };
        glm_vec3_normalize(horizontalFront);

        if (RGFW_isPressed(win, RGFW_w)) {
            player.position[0] += horizontalFront[0] * horizontalSpeed;
            player.position[2] += horizontalFront[2] * horizontalSpeed;
        }
        if (RGFW_isPressed(win, RGFW_s)) {
            player.position[0] -= horizontalFront[0] * horizontalSpeed;
            player.position[2] -= horizontalFront[2] * horizontalSpeed;
        }
        if (RGFW_isPressed(win, RGFW_a)) {
            player.position[0] -= horizontalFront[2] * horizontalSpeed;
            player.position[2] += horizontalFront[0] * horizontalSpeed;
        }
        if (RGFW_isPressed(win, RGFW_d)) {
            player.position[0] += horizontalFront[2] * horizontalSpeed;
            player.position[2] -= horizontalFront[0] * horizontalSpeed;
        }
        if (RGFW_isPressed(win, RGFW_space)) {
            player.position[1] += horizontalSpeed;
        }
        if (RGFW_isPressed(win, RGFW_shiftL)) {
            player.position[1] -= horizontalSpeed;
        }

        char buffer[256];
        sprintf(buffer, "Fyrraria: %d", fps);
        const char* fps_str = buffer;
        RGFW_window_setName(win, fps_str);

        mat4 view;
        vec3 center;
        glm_vec3_add(transformedplayerposition, cameraFront, center);
        glm_lookat(transformedplayerposition, center, playersNormalisedNormalForCameraUse, view);
        
        mat4 projection;
        glm_perspective(glm_rad(70), (float)win->r.w/win->r.h, 0.1, 100000.0, projection);

        mat4 model = GLM_MAT4_IDENTITY_INIT;
        int modelLocaction = glGetUniformLocation(shaderProgram, "model");
        if (modelLocaction == -1) {
            printf("cant find model in shader location \n");
        }
        
        glUniformMatrix4fv(modelLocaction, 1, GL_FALSE, (const float *)model);

        int viewLocation = glGetUniformLocation(shaderProgram, "view");
        int projectionLocation = glGetUniformLocation(shaderProgram, "projection");

        if (viewLocation == -1) {
            printf("cant find view in shader location \n");
        }
        if (projectionLocation == -1) {
            printf("cant find projection in shader location \n");
        }

        int gridXLocation = glGetUniformLocation(shaderProgram, "gridX");
        //int gridYLocation = glGetUniformLocation(shaderProgram, "gridY");
        int gridZLocation = glGetUniformLocation(shaderProgram, "gridZ");

        if (gridXLocation == -1) {
            printf("cant find gridXLocation in shader location \n");
        }
        //if (gridYLocation == -1) {
        //    printf("cant find gridYLocation in shader location \n");
        //}
        if (gridZLocation == -1) {
            printf("cant find gridZLocation in shader location \n");
        }



        glUniform1f(gridXLocation, Worldx * chunksize);
        //glUniform1f(gridYLocation, Worldy * chunksize);
        glUniform1f(gridZLocation, Worldz * chunksize);

        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const float *)view);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (const float *)projection);


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
