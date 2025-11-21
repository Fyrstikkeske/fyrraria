#pragma once

#include "cglm/types.h"
#include <cc.h>
#include <math.h>
#include <RGFW.h>

int make_shader_program ();
unsigned long load_texture_into_shader_bindless(int shaderProgram, const char* pathToFile);
void analytical_torus_normal(vec3 input, vec3 dest, int Worldx, int Worldz, int chunksize);
void transform_to_global_position_exp(const float localPosition[3], float globalPosition[3],int Worldx, int Worldz, int chunksize);
void get_torus_frame(vec3 position,vec3 tangentOut,vec3 bitangentOut,int Worldx, int Worldz, float chunksize);
void rotate_camera(float_t pitch, float_t yaw, float_t* tangent, float_t* bitangent,vec3 cameraFront,  vec3 playersNormalisedNormalForCameraUse, vec3 cameraFrontLocal);
void handle_movement(RGFW_window* win, vec3 cameraFrontLocal, vec3 playerpos);


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




extern const int WORLD_X;
extern const int WORLD_Y;
extern const int WORLD_Z;
extern const bool DoesBlockOcclude[];




constexpr int CHUNK_SIZE = 16;
constexpr int CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

constexpr int CHUNK_SIZE_P = CHUNK_SIZE + 2;
constexpr int CHUNK_AREA_P = CHUNK_SIZE_P * CHUNK_SIZE_P;
constexpr int CHUNK_VOLUME_P = CHUNK_SIZE_P * CHUNK_SIZE_P * CHUNK_SIZE_P;






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

enum Blocktype : unsigned char{
    AIR,
    TEST,
    GRASS,
    LEAF,
    WOOD_LOG,
    WATER,
};

struct block{
    enum Blocktype type;
};

typedef struct{
    int32_t x;
    int32_t y;
    int32_t z;
} vec3int;

typedef struct{
    uint8_t x;
    uint8_t y;
    uint8_t z;
} vec3u8;

typedef struct {
    struct block blocks[CHUNK_VOLUME];
    bool isdirty;
    GLuint VBO;
    GLuint VAO;
    int vertices;
    bool is_active;
    uint8_t lod;
} Chunk;




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