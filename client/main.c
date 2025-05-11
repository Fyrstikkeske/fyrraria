#define GLAD_GL_IMPLEMENTATION
#define RGFW_IMPLEMENTATION
#define RGL_LOAD_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <gl.h>
#include <string.h>
#include <RGFW.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stb_image.h>

struct Mesh{
    unsigned int VOA, VBO, EBO, material
};

void keyfunc(RGFW_window* win, unsigned char key, unsigned char keyChar, unsigned char keyMod, unsigned char pressed) {
    if (key == RGFW_escape && pressed) {
        RGFW_window_setShouldClose(win, True);
    }
}

// thanks chatgippyty. must in theory be freed :troll: important part, in theory. we should only load shaders once, which means no possibility of memory leaks
char* loadShaderSource(const char *filename) {
    FILE *file = fopen(filename, "r"); 
    if (!file) {
        perror("Error opening shader file");
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END); 
    long fileSize = ftell(file); 
    fseek(file, 0, SEEK_SET); 

    //TODO no malloc shall be needed, it is blasphameus. nvm c is just like this
    char *source = (char *)malloc(fileSize + 1);

    if (!source) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    fread(source, 1, fileSize, file);
    source[fileSize] = '\0';  

    fclose(file);
    return source;
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

int main() {
    RGFW_setGLHint(RGFW_glMinor, 3);
    RGFW_setGLHint(RGFW_glMajor, 3);
    RGFW_window* win = RGFW_createWindow("Fyrraria", RGFW_RECT(0, 0, 800, 600), RGFW_windowCenter);
    gladLoadGL((GLADloadfunc) RGFW_getProcAddress);
    RGFW_setKeyCallback(keyfunc); // you can use callbacks like this if you want
    stbi_set_flip_vertically_on_load(1);

    if (!win) {
        fprintf(stderr, "Failed to create window or OpenGL context.\n");
        return -1;
    }
    
    //LOAD FILES
    const char *vertexShaderSource = loadShaderSource("client/shaders/cube.vert");
    const char *fragmentShaderSource = loadShaderSource("client/shaders/cube.frag");

    if (!vertexShaderSource || !fragmentShaderSource) {
        printf("Failed to load shader sources\n");
        return 1;
    }

    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
    };

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

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader, "VERTEX");

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
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


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); 
   
    //texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("client/textures/grass_top.png", &width, &height, &nrChannels, 0);
    GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;


    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);



    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    
    stbi_image_free(data);
    
    

    GLuint64 handle1 = glGetTextureHandleARB(texture1);
    glMakeTextureHandleResidentARB(handle1);


    unsigned int texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    data = stbi_load("client/textures/tnt_test.png", &width, &height, &nrChannels, 0);
    format = nrChannels == 4 ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    GLuint64 handle2 = glGetTextureHandleARB(texture2);
    glMakeTextureHandleResidentARB(handle2);
    

    GLint loc1 = glGetUniformLocation(shaderProgram, "texture2");
    GLint loc2 = glGetUniformLocation(shaderProgram, "texture1");
    if (loc1 != -1) glUniformHandleui64ARB(loc1, handle1);
    if (loc2 != -1) glUniformHandleui64ARB(loc2, handle2);


    
    u32 fps = 0;
    u32 frames = 0;
    const double startTime = RGFW_getTime();

    while (RGFW_window_shouldClose(win) == RGFW_FALSE) {
        while (RGFW_window_checkEvent(win) != NULL) {  // or RGFW_window_checkEvents(); if you only want callbacks
            // you can either check the current event yourself
            if (win->event.type == RGFW_quit) break;
            
            if (win->event.type == RGFW_mouseButtonPressed && win->event.button == RGFW_mouseLeft) {
                printf("You clicked at x: %d, y: %d\n", win->event.point.x, win->event.point.y);
            }
            
            // or use the existing functions
            if (RGFW_isMousePressed(win, RGFW_mouseRight)) {
                printf("The right mouse button was clicked at x: %d, y: %d\n", win->event.point.x, win->event.point.y);
            }

            if (win->event.type == RGFW_windowResized){
                glViewport(0, 0, win->r.w, win->r.h);
            }
        }

        if (RGFW_isPressed(win, RGFW_space)){
            printf("fps: %d\n", fps);
        }
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //glPolygonMode(GL_FRONT, GL_LINE);
        glUseProgram(shaderProgram);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        RGFW_window_swapBuffers(win);
        fps = RGFW_checkFPS(startTime, frames, 60);
        frames++;
    }

    RGFW_window_close(win);
    return 0;
}





