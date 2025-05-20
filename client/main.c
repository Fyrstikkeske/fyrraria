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

struct shaderstruct{
    int shaderProgram;
    unsigned int VAO;
};

float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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
    }
}

void loadTextureIntoShaderBindless(int shaderProgram, const char* pathToFile, const char* textureNameInShader) {
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint64 handle = glGetTextureHandleARB(texture);
    glMakeTextureHandleResidentARB(handle);
    GLint locaction = glGetUniformLocation(shaderProgram, textureNameInShader);
    if (locaction != -1) glUniformHandleui64ARB(locaction, handle);
    stbi_image_free(data);
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

void map_3d_to_torus(vec3 point, double major_radius, double minor_radius) {

    double theta = point[0] * 2.0 * M_PI;
    double phi = point[1] * 2.0 * M_PI;

    double dynamic_minor_radius = minor_radius * exp(point[2]);

    double cos_theta = cos(theta);
    double sin_theta = sin(theta);
    double cos_phi = cos(phi);
    double sin_phi = sin(phi);

    point[0] = (major_radius + dynamic_minor_radius * cos_phi) * cos_theta;
    point[1] = (major_radius + dynamic_minor_radius * cos_phi) * sin_theta;
    point[2] = dynamic_minor_radius * sin_phi;
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
    
    int shaderProgram = makeshaderprogram();
    glEnable(GL_DEPTH_TEST);

    loadTextureIntoShaderBindless(shaderProgram, "client/textures/grass.png", "grass");
    


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
    

        vec3 front = {
            cos(glm_rad(yaw)) * cos(glm_rad(pitch)),
            sin(glm_rad(pitch)),
            sin(glm_rad(yaw)) * cos(glm_rad(pitch))
        };
        glm_vec3_normalize_to(front, cameraFront);

        glm_vec3_copy(front, cameraFront);

        vec3 cameraSpeed = {0.1f, 0.1f, 0.1f};
        if (RGFW_isPressed(win, RGFW_shiftL)){
            vec3 cameraSpeedmult = {2.f, 2.0f, 2.0f};
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

        int blockpositionLocaction = glGetUniformLocation(shaderProgram, "blockposition");
        int viewLocation = glGetUniformLocation(shaderProgram, "view");
        int projectionLocation = glGetUniformLocation(shaderProgram, "projection");

        if (blockpositionLocaction == -1) {
            printf("cant find blockpositionLocaction in shader location \n");
        }
        if (viewLocation == -1) {
            printf("cant find view in shader location \n");
        }
        if (projectionLocation == -1) {
            printf("cant find projection in shader location \n");
        }

        int gridXLocation = glGetUniformLocation(shaderProgram, "gridX");
        int gridYLocation = glGetUniformLocation(shaderProgram, "gridY");

        if (gridXLocation == -1) {
            printf("cant find gridXLocation in shader location \n");
        }
        if (gridYLocation == -1) {
            printf("cant find gridYLocation in shader location \n");
        }

        int x = 120;
        int y = 40;

        //height should the be height? dont know but it is for some reason
        int z = 1;

        glUniform1f(gridXLocation, x);
        glUniform1f(gridYLocation, y);

        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, (const float *)view);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, (const float *)projection);


        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        for (unsigned int iter = 0; iter < x*y*z; iter++){
            vec3 blockposition = {(iter % x), ((iter / x) % y), ((iter / x) / y) % z};

            glUniform3fv(blockpositionLocaction, 1, (const float *)blockposition);
            glDrawArrays(GL_TRIANGLES, 0, 36);
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
