#pragma once


#include <stb_image.h>
#include <cglm/cglm.h>
#include <gl.h>
#include <RGFW.h>
#include <uchar.h>
#include "cc.h"

#define GLAD_GL_IMPLEMENTATION

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif





#define PROFILE_ENABLED 0  // Set to 0 to disable profiling

#if PROFILE_ENABLED
    #define PROFILE_BEGIN(name) \
        struct timespec profile_start_##name, profile_end_##name; \
        clock_gettime(CLOCK_MONOTONIC, &profile_start_##name)

    #define PROFILE_END(name) \
        do { \
            clock_gettime(CLOCK_MONOTONIC, &profile_end_##name); \
            double duration = (profile_end_##name.tv_sec - profile_start_##name.tv_sec) * 1000.0 + \
                            (profile_end_##name.tv_nsec - profile_start_##name.tv_nsec) / 1000000.0; \
            printf("[PROFILE] %-30s %7.3f ms\n", #name, duration); \
        } while(0)

    // Simplified PROFILE_SCOPE without comma expression warning
    #define PROFILE_SCOPE(name) \
        PROFILE_BEGIN(name); \
        for(int _done = 0; !_done; _done = 1)

    #define PROFILE_SCOPE_END(name) PROFILE_END(name)
#else
    #define PROFILE_BEGIN(name)
    #define PROFILE_END(name)
    #define PROFILE_SCOPE(name) for(int _i = 0; _i < 1; _i++)
    #define PROFILE_SCOPE_END(name)
#endif




constexpr char vertexShaderSource[] = {
    #embed "../shaders/cube.vert" 
    , 0
};

constexpr char fragmentShaderSource[] = {
    #embed "../shaders/cube.frag" 
    , 0
};

enum Blocktype : unsigned char{
    AIR,
    TEST,
    GRASS,
    LEAF,
    WOOD_LOG,
    WATER,
};

const bool DoesBlockOcclude[] = {
    [AIR] = false,
    [GRASS] = true,
    [TEST] = true,
    [LEAF] = true,
    [WOOD_LOG] = true,
    [WATER] = true,
};



constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

constexpr int CHUNK_SIZE_P = CHUNK_SIZE + 2;
constexpr int CHUNK_AREA_P = CHUNK_SIZE_P * CHUNK_SIZE_P;
constexpr int CHUNK_VOLUME_P = CHUNK_SIZE_P * CHUNK_SIZE_P * CHUNK_SIZE_P;

//TODO, move this to the planets struct so we can get more planets
// X and Y must be odd due to some chunk detection glitch? otherwise the Other side of the torus final chunk doesnt render
const int WORLD_X = 1000;
const int WORLD_Y = 1;
const int WORLD_Z = 100;

#define CLAMP(x, lower, upper) ((x) < (lower) ? (lower) : ((x) > (upper) ? (upper) : (x)))
#define EUCLID_MODULO(x, modulo) ((x % modulo + modulo) % modulo)

struct shaderstruct{
    int shaderProgram;
    unsigned int VAO;
};

typedef struct {
    vec3 position;
} Player;

typedef struct {
    GLuint vbo;
    GLsizei vertexCount;
} RenderChunk;


struct block{
    enum Blocktype type;
};

typedef struct{
    int32_t x;
    int32_t y;
    int32_t z;
} vec3int;

#define CC_CMPR vec3int, {                            \
    if (val_1.x != val_2.x)                           \
        return val_1.x < val_2.x ? -1 : 1;            \
    if (val_1.y != val_2.y)                           \
        return val_1.y < val_2.y ? -1 : 1;            \
    if (val_1.z != val_2.z)                           \
        return val_1.z < val_2.z ? -1 : 1;            \
    return 0;                                         \
}

#define CC_HASH vec3int, {                             \
    return (uint64_t)val.x * 2654435761ull +           \
           (uint64_t)val.y * 2246822519ull +           \
           (uint64_t)val.z * 3266489917ull;            \
}

#include "cc.h"

typedef struct {
    struct block blocks[CHUNK_VOLUME];
    bool isdirty;
    GLuint VBO;
    GLuint VAO;
    int vertices;
    bool is_active;
    char8_t lod;
} Chunk;



//chatgpt to the save like usual
static inline void transform_to_global_position_exp(
    const float localPosition[3], float globalPosition[3],
    int Worldx, int Worldz, int chunksize)
{
    // Normalize parameters u, w in [0, 1]
    float u = localPosition[0] / (Worldx * chunksize);
    float w = localPosition[2] / (Worldz * chunksize);

    // Angles (radians)
    float theta = 2.0f * M_PI * u; // major angle
    float phi   = 2.0f * M_PI * w; // minor angle

    // Radii
    float majorRadius = (Worldx * chunksize) / (2.0f * M_PI);
    float minorRadius = (Worldz * chunksize) / (2.0f * M_PI);

    // radial offset inside the minor tube
    float radialOffset = localPosition[1];
    float distance = minorRadius * phi;

    // Exponential curvature mapping (minor circle)
    float expFactor = expf(radialOffset / minorRadius);
    float circle_x = minorRadius * (expFactor * cosf(distance / minorRadius)) - minorRadius;
    float circle_y = minorRadius * (expFactor * sinf(distance / minorRadius));

    // Final global position on torus with exponential curvature
    float x = (majorRadius + circle_x) * cosf(theta);
    float y = (majorRadius + circle_x) * sinf(theta);
    float z = circle_y;

    globalPosition[0] = x;
    globalPosition[1] = y;
    globalPosition[2] = z;
}
static inline void analytical_torus_normal(vec3 input, vec3 dest, int Worldx, int Worldz, int chunksize){
    float u = input[0] / (Worldx * chunksize);
    float w = input[2] / (Worldz * chunksize);
    float theta = 2.0 * M_PI * u;
    float phi   = 2.0 * M_PI * w;
    
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    
    
    vec3 normal = {
        cosPhi * cosTheta,
        cosPhi * sinTheta,
        sinPhi
    };

    glm_vec3_copy(normal, dest);
}
static inline void handle_movement(RGFW_window* win, vec3 cameraFrontLocal, vec3 playerpos){
    float horizontalSpeed;
    //why us this a warning in the ide, it compiles. fuck this
    if (RGFW_isPressed(win, RGFW_controlL)) {
        horizontalSpeed = 32.0;
    }else {
        horizontalSpeed = 0.1;
    }
    vec3 horizontalFront = { cameraFrontLocal[0], 0.0f, cameraFrontLocal[2] };
    glm_vec3_normalize(horizontalFront);

    if (RGFW_isPressed(win, RGFW_w)) {
        playerpos[0] += horizontalFront[0] * horizontalSpeed;
        playerpos[2] += horizontalFront[2] * horizontalSpeed;
    }
    if (RGFW_isPressed(win, RGFW_s)) {
        playerpos[0] -= horizontalFront[0] * horizontalSpeed;
        playerpos[2] -= horizontalFront[2] * horizontalSpeed;
    }
    if (RGFW_isPressed(win, RGFW_a)) {
        playerpos[0] -= horizontalFront[2] * horizontalSpeed;
        playerpos[2] += horizontalFront[0] * horizontalSpeed;
    }
    if (RGFW_isPressed(win, RGFW_d)) {
        playerpos[0] += horizontalFront[2] * horizontalSpeed;
        playerpos[2] -= horizontalFront[0] * horizontalSpeed;
    }
    if (RGFW_isPressed(win, RGFW_space)) {
        playerpos[1] += horizontalSpeed;
    }
    if (RGFW_isPressed(win, RGFW_shiftL)) {
        playerpos[1] -= horizontalSpeed;
    }
}
void get_torus_frame(
    vec3 position,
    vec3 tangentOut,
    vec3 bitangentOut,
    int Worldx, int Worldz, float chunksize)
{
    vec3 normal;
    analytical_torus_normal(position, normal, Worldx, Worldz, chunksize);

    float delta = 0.1f;
    vec3 pos_plus_dx, pos_minus_dx, pos_plus_dz, pos_minus_dz;
    
    // Tangent (dx)
    vec3 local_plus_dx = { position[0] + delta, position[1], position[2] };
    vec3 local_minus_dx = { position[0] - delta, position[1], position[2] };
    transform_to_global_position_exp(local_plus_dx, pos_plus_dx, Worldx, Worldz, chunksize);
    transform_to_global_position_exp(local_minus_dx, pos_minus_dx, Worldx, Worldz, chunksize);
    glm_vec3_sub(pos_plus_dx, pos_minus_dx, tangentOut);
    glm_vec3_normalize(tangentOut);

    // Bitangent (dz)
    vec3 local_plus_dz = { position[0], position[1], position[2] + delta };
    vec3 local_minus_dz = { position[0], position[1], position[2] - delta };
    transform_to_global_position_exp(local_plus_dz, pos_plus_dz, Worldx, Worldz, chunksize);
    transform_to_global_position_exp(local_minus_dz, pos_minus_dz, Worldx, Worldz, chunksize);
    glm_vec3_sub(pos_plus_dz, pos_minus_dz, bitangentOut);
    glm_vec3_normalize(bitangentOut);

    // Ensure orthogonality
    vec3 recalc_bitangent;
    glm_vec3_cross(normal, tangentOut, recalc_bitangent);
    glm_vec3_normalize_to(recalc_bitangent, bitangentOut);
}


unsigned long load_texture_into_shader_bindless(int shaderProgram, const char* pathToFile) {
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

int make_shader_program (){
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

static inline void rotate_camera(float_t pitch, float_t yaw, float_t* tangent, float_t* bitangent,vec3 cameraFront,  vec3 playersNormalisedNormalForCameraUse, vec3 cameraFrontLocal){ 

    
    float yawRad   = glm_rad(yaw);
    float pitchRad = glm_rad(pitch);

    vec3 cameraFrontLocalLocal = {
    cosf(pitchRad) * cosf(yawRad),   // along tangent
    sinf(pitchRad),                  // along local “up” (normal)
    cosf(pitchRad) * sinf(yawRad)    // along bitangent
    };

    glm_vec3_copy(cameraFrontLocalLocal, cameraFrontLocal);

    glm_vec3_zero(cameraFront);
    glm_vec3_muladds(tangent, cameraFrontLocal[0], cameraFront);
    glm_vec3_muladds(playersNormalisedNormalForCameraUse, cameraFrontLocal[1], cameraFront);
    glm_vec3_muladds(bitangent, cameraFrontLocal[2], cameraFront);
    glm_vec3_normalize(cameraFront);
}
