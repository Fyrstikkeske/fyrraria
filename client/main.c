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
const int Worldz = 8; // minor ring

struct shaderstruct{
    int shaderProgram;
    unsigned int VAO;
};

float vertices[] = {
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
    0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f,

    0.0f, 0.0f,  1.0f,  0.0f, 0.0f,
     1.0f, 0.0f,  1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f,
    0.0f,  1.0f,  1.0f,  0.0f, 1.0f,
    0.0f, 0.0f,  1.0f,  0.0f, 0.0f,

    0.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,  1.0f, 1.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    0.0f, 0.0f,  1.0f,  0.0f, 0.0f,
    0.0f,  1.0f,  1.0f,  1.0f, 0.0f,

     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
     1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     1.0f, 0.0f,  1.0f,  0.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,

    0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     1.0f, 0.0f,  1.0f,  1.0f, 0.0f,
     1.0f, 0.0f,  1.0f,  1.0f, 0.0f,
    0.0f, 0.0f,  1.0f,  0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

    0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
     1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 0.0f,
    0.0f,  1.0f,  1.0f,  0.0f, 0.0f,
    0.0f,  1.0f, 0.0f,  0.0f, 1.0f
};

enum blocktype{
    grass,
    test,
    air,
    leaf,
    woodlog,
};

struct block{
    enum blocktype type;
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

   unsigned int indices[] = {  // note that we start from 0!
       0, 1, 3,   // first triangle
       1, 2, 3    // second triangle
   };  
   unsigned int VAO;
   glGenVertexArrays(1, &VAO);  
   glBindVertexArray(VAO);

   unsigned int VBO;
   glGenBuffers(1, &VBO);  
   glBindBuffer(GL_ARRAY_BUFFER, VBO);  
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 
    


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


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);
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
            

            world[chunkiter][blockiter].type = air;

            if (globaly == 1){
                world[chunkiter][blockiter].type = grass;
            }

            globalz -= 5;
            globalx -= 160;
            if (globaly == 2 && globalz == 3 && globalx == 3){
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
            if (globaly == 5 && globalz == 2 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 5 && globalz == 4 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 5 && globalz == 3 && globalx == 2){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 5 && globalz == 3 && globalx == 4){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }

        }
    }


    
    int shaderProgram = makeshaderprogram();
    glEnable(GL_DEPTH_TEST);

    unsigned int grasshandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/grass.png");
    unsigned int tnt_testhandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/tnt_test.png");
    unsigned int woodloghandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/log.png");
    unsigned int leafhandle = loadTextureIntoShaderBindless(shaderProgram, "client/textures/leaf.png");

    GLint texturehandlelocaction = glGetUniformLocation(shaderProgram, "texturehandle");
    if (texturehandlelocaction != -1) printf("cant find the texture location in shader");
    

    vec3 cameraPos = { 0.0, 0.0, 3.0 };
    vec3 cameraFront = { 0.0, 0.0, -1.0 };
    vec3 cameraUp = { 0.0, 1.0, 0.0 };

    float yaw = -90.0f;
    float pitch = 0.0f;
    const float sensitivity = 0.1f;

    RGFW_window_showMouse(win, 0);
    RGFW_window_mouseHold(win, RGFW_AREA(win->r.w / 2, win->r.h / 2));
    RGFW_setKeyCallback(keyfunc);

    u32 fps = 0;
    u32 frames = 0;
    const double startTime = RGFW_getTime();
    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        while (RGFW_window_checkEvent(win) != NULL) {
            if (win->event.type == RGFW_quit) break;
            
            if (win->event.type == RGFW_mousePosChanged) {
                yaw += win->event.vector.x * sensitivity;
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
    
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        vec3 front = {
            cos(glm_rad(yaw)) * cos(glm_rad(pitch)),
            sin(glm_rad(pitch)),
            sin(glm_rad(yaw)) * cos(glm_rad(pitch))
        };
        glm_vec3_normalize_to(front, cameraFront);

        glm_vec3_copy(front, cameraFront);

        vec3 cameraSpeed = {0.1f, 0.1f, 0.1f};
        if (RGFW_isPressed(win, RGFW_shiftL)){
            vec3 cameraSpeedmult = {20.f, 20.0f, 20.0f};
            glm_vec3_mul(cameraSpeed, cameraSpeedmult, cameraSpeed);
        };
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (RGFW_isPressed(win, RGFW_w)){
            vec3 reducedCameraFront;
            glm_vec3_mul(cameraFront, cameraSpeed, reducedCameraFront);
            glm_vec3_add(cameraPos, reducedCameraFront, cameraPos);
        }
        if (RGFW_isPressed(win, RGFW_s)){
            vec3 reducedCameraFront;
            glm_vec3_mul(cameraFront, cameraSpeed, reducedCameraFront);
            glm_vec3_sub(cameraPos, reducedCameraFront, cameraPos);
        }
        if (RGFW_isPressed(win, RGFW_a)){
            vec3 normalised_value;
            glm_vec3_cross(cameraFront, cameraUp, normalised_value);
            glm_vec3_normalize(normalised_value);
            glm_vec3_mul(normalised_value, cameraSpeed, normalised_value);
            glm_vec3_sub(cameraPos, normalised_value, cameraPos);
        }
        if (RGFW_isPressed(win, RGFW_d)){
            vec3 normalised_value;
            glm_vec3_cross(cameraFront, cameraUp, normalised_value);
            glm_vec3_normalize(normalised_value);
            glm_vec3_mul(normalised_value, cameraSpeed, normalised_value);
            glm_vec3_add(cameraPos, normalised_value, cameraPos);
        }

        



        char buffer[256];
        sprintf(buffer, "Fyrraria: %d", fps);
        const char* fps_str = buffer;
        RGFW_window_setName(win, fps_str);

        

        
        mat4 view;
        vec3 center;
        glm_vec3_add(cameraPos, cameraFront, center);
        glm_lookat(cameraPos, center, cameraUp, view);
        
        mat4 projection;
        glm_perspective(glm_rad(70), win->r.w/win->r.h, 0.1, 100000.0, projection);

        mat4 model = GLM_MAT4_IDENTITY_INIT;
        int modelLocaction = glGetUniformLocation(shaderProgram, "model");
        if (modelLocaction == -1) {
            printf("cant find model in shader location \n");
        }
        
        glUniformMatrix4fv(modelLocaction, 1, GL_FALSE, (const float *)model);

        int gridCoordLocaction = glGetUniformLocation(shaderProgram, "gridCoord");
        int viewLocation = glGetUniformLocation(shaderProgram, "view");
        int projectionLocation = glGetUniformLocation(shaderProgram, "projection");

        if (gridCoordLocaction == -1) {
            printf("cant find gridCoordLocaction in shader location \n");
        }
        if (viewLocation == -1) {
            printf("cant find view in shader location \n");
        }
        if (projectionLocation == -1) {
            printf("cant find projection in shader location \n");
        }

        int gridXLocation = glGetUniformLocation(shaderProgram, "gridX");
        int gridYLocation = glGetUniformLocation(shaderProgram, "gridY");
        int gridZLocation = glGetUniformLocation(shaderProgram, "gridZ");

        if (gridXLocation == -1) {
            printf("cant find gridXLocation in shader location \n");
        }
        if (gridYLocation == -1) {
            printf("cant find gridYLocation in shader location \n");
        }
        if (gridZLocation == -1) {
            printf("cant find gridZLocation in shader location \n");
        }



        glUniform1f(gridXLocation, Worldx * chunksize);
        glUniform1f(gridYLocation, Worldy * chunksize);
        glUniform1f(gridZLocation, Worldz * chunksize);

        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const float *)view);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (const float *)projection);


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        for (int worlditer = 0; worlditer < Worldx*Worldy*Worldz; worlditer++){

            int chunkx = worlditer % Worldx;
            int chunky = (worlditer / Worldx) % Worldy;
            int chunkz = (worlditer / (Worldx * Worldy)) % Worldz;

            for (int chunkiter = 0; chunkiter< chunksize*chunksize*chunksize; chunkiter++){
                int localx = chunkiter % chunksize;
                int localy = (chunkiter / chunksize) % chunksize;
                int localz = (chunkiter / (chunksize * chunksize)) % chunksize;

                vec3 blockposition = {(float)(chunkx * chunksize + localx),(float)(chunky * chunksize + localy), (float)(chunkz * chunksize + localz)};

                glUniform3fv(gridCoordLocaction, 1, (const float *)blockposition);

                if (world[worlditer][chunkiter].type == air){
                    continue;
                }

                if (world[worlditer][chunkiter].type == grass){
                    glUniformHandleui64ARB(texturehandlelocaction, grasshandle);
                }
                if (world[worlditer][chunkiter].type == test){
                    glUniformHandleui64ARB(texturehandlelocaction, tnt_testhandle);
                }
                if (world[worlditer][chunkiter].type == woodlog){
                    glUniformHandleui64ARB(texturehandlelocaction, woodloghandle);
                }
                if (world[worlditer][chunkiter].type == leaf){
                    glUniformHandleui64ARB(texturehandlelocaction, leafhandle);
                }

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }


        }
        
        RGFW_window_swapBuffers(win);
        fps = RGFW_checkFPS(startTime, frames, 60);
        frames++;
    }

    RGFW_window_close(win);
    return 0;
}

/* Note for future
if (RGFW_isPressed(win, RGFW_space)){
    printf("fps: %d\n", fps);
}
*/


/*
not needed for now
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
glBindVertexArray(0);
glBindVertexArray(shaderProgram.VAO);
*/
