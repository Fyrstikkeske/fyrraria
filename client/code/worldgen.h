#pragma once
#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE

#define FNL_IMPL

#include <stdio.h>

#include "FastNoiseLite.h"
#include <time.h>
#include "cglm/types.h"
#include "meshing.h"
#include "utils.h"
#include "uthash.h"
#include <stdbool.h>
#include <stdlib.h>

#define CLAMP(x, lower, upper) ((x) < (lower) ? (lower) : ((x) > (upper) ? (upper) : (x)))




Chunk* chunk_add(Chunk** planet, const vec3int* key, const uint64_t* texture_handles) {
    Chunk* chunk = malloc(sizeof(Chunk));

    if (!chunk) {
        fprintf(stderr, "Memory allocation failed for chunk!\n");
        exit(EXIT_FAILURE);
    }
    chunk->Key = *key;
    chunk->isdirty = true;
    glGenVertexArrays(1, &chunk->VAO);
    glGenBuffers(1,&chunk->VBO);

    
    generate_chunk(chunk);



    generate_mesh_for_chunk(chunk, texture_handles);

    HASH_ADD(hh, *planet, Key, sizeof(vec3int), chunk);
    return chunk;
}

Chunk* chunk_find(Chunk** planet, const vec3int* key) {
    Chunk* chunk;
    HASH_FIND(hh, *planet, key, sizeof(vec3int), chunk);
    return chunk;
}

void chunk_remove(Chunk** planet, const vec3int* key) {
    Chunk* chunk = chunk_find(planet, key);
    if (chunk) {

        glDeleteBuffers(1,&chunk->VBO);
        glDeleteVertexArrays(1, &chunk->VAO);

        HASH_DEL(*planet, chunk);
        free(chunk);
    }
}

// Chunk coordinate calculations
static inline void calculate_chunk_coords(
    vec3 position,
    int render_distance,
    int iter,
    int* chunk_x,
    int* chunk_y,
    int* chunk_z
) {
    const int player_chunk_x = position[0] / CHUNK_SIZE;
    const int player_chunk_y = position[1] / CHUNK_SIZE;
    const int player_chunk_z = position[2] / CHUNK_SIZE;

    const int local_x = iter % render_distance;
    const int local_y = (iter / render_distance) % render_distance;
    const int local_z = iter / (render_distance * render_distance);

    *chunk_x = player_chunk_x + (local_x - render_distance/2);
    *chunk_y = player_chunk_y + (local_y - render_distance/2);
    *chunk_z = player_chunk_z + (local_z - render_distance/2);
}

static inline void wrap_coordinates(
    int world_x,
    int world_y,
    int world_z,
    int* x,
    int* y,
    int* z
) {
    *x = (*x % world_x + world_x) % world_x;
    *z = (*z % world_z + world_z) % world_z;

    *y = CLAMP(*y, 0, world_y - 1);
}

static void add_cubes_to_list(
    vec3int** list, 
    size_t* count,  // Changed to size_t
    size_t* capacity,  // Changed to size_t
    int x_start, int x_end,
    int y_start, int y_end,
    int z_start, int z_end
) {
    const size_t needed = (size_t)(x_end - x_start + 1) * 
                         (size_t)(y_end - y_start + 1) * 
                         (size_t)(z_end - z_start + 1);
    
    if (*count + needed > *capacity) {
        // Handle overflow-safe capacity calculation
        size_t new_capacity;
        if (*capacity == 0) {
            new_capacity = needed;
        } else {
            // Check for overflow before doubling
            if (*capacity > SIZE_MAX / 2) {
                fprintf(stderr, "Capacity overflow detected\n");
                exit(EXIT_FAILURE);
            }
            new_capacity = *capacity * 2;
        }

        // Ensure we have enough space
        if (needed > SIZE_MAX - *count || new_capacity < *count + needed) {
            new_capacity = *count + needed;
        }

        // Check for allocation size overflow
        if (new_capacity > SIZE_MAX / sizeof(vec3int)) {
            fprintf(stderr, "Allocation size too large\n");
            exit(EXIT_FAILURE);
        }

        vec3int* new_list = realloc(*list, new_capacity * sizeof(vec3int));
        if (!new_list) {
            perror("realloc failed");
            exit(EXIT_FAILURE);
        }
        *list = new_list;
        *capacity = new_capacity;
    }
    
    // Add cubes to the list
    for (int x = x_start; x <= x_end; x++) {
        for (int y = y_start; y <= y_end; y++) {
            for (int z = z_start; z <= z_end; z++) {
                (*list)[*count] = (vec3int){x, y, z};
                (*count)++;
            }
        }
    }
}

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))


static void get_entered_exited_cubes(
    vec3int start_center,
    vec3int end_center,
    int render_distance,
    vec3int** entered, size_t* entered_count,
    vec3int** exited, size_t* exited_count
) {
    // Initialize outputs
    *entered = NULL;
    *exited = NULL;
    *entered_count = 0;
    *exited_count = 0;
    size_t entered_cap = 0;
    size_t exited_cap = 0;

    // Precompute safe render distances
    const int safe_render_x = MIN(render_distance, (WORLD_X - 1)/ 2);
    const int safe_render_z = MIN(render_distance, (WORLD_Z - 1)/ 2);
    const int safe_render_y = render_distance;

    // Precompute boundaries with simplified expressions
    const int s_min_x = start_center.x - safe_render_x;
    const int s_max_x = start_center.x + safe_render_x;
    const int s_min_y = start_center.y - safe_render_y;
    const int s_max_y = start_center.y + safe_render_y;
    const int s_min_z = start_center.z - safe_render_z;
    const int s_max_z = start_center.z + safe_render_z;

    const int e_min_x = end_center.x - safe_render_x;
    const int e_max_x = end_center.x + safe_render_x;
    const int e_min_y = end_center.y - safe_render_y;
    const int e_max_y = end_center.y + safe_render_y;
    const int e_min_z = end_center.z - safe_render_z;
    const int e_max_z = end_center.z + safe_render_z;

    // Precompute overlapping regions for Y/Z faces
    const int overlap_x_min = MAX(s_min_x, e_min_x);
    const int overlap_x_max = MIN(s_max_x, e_max_x);
    const int overlap_y_min = MAX(s_min_y, e_min_y);
    const int overlap_y_max = MIN(s_max_y, e_max_y);

    // EXITED cubes (in start but not in end)
    // Left face (X-min)
    if (s_min_x < e_min_x) {
        add_cubes_to_list(exited, exited_count, &exited_cap,
            s_min_x, MIN(s_max_x, e_min_x - 1),
            s_min_y, s_max_y,
            s_min_z, s_max_z);
    }
    // Right face (X-max)
    if (s_max_x > e_max_x) {
        add_cubes_to_list(exited, exited_count, &exited_cap,
            MAX(s_min_x, e_max_x + 1), s_max_x,
            s_min_y, s_max_y,
            s_min_z, s_max_z);
    }
    // Front face (Y-min)
    if (s_min_y < e_min_y) {
        add_cubes_to_list(exited, exited_count, &exited_cap,
            overlap_x_min, overlap_x_max,
            s_min_y, MIN(s_max_y, e_min_y - 1),
            s_min_z, s_max_z);
    }
    // Back face (Y-max)
    if (s_max_y > e_max_y) {
        add_cubes_to_list(exited, exited_count, &exited_cap,
            overlap_x_min, overlap_x_max,
            MAX(s_min_y, e_max_y + 1), s_max_y,
            s_min_z, s_max_z);
    }
    // Bottom face (Z-min)
    if (s_min_z < e_min_z) {
        add_cubes_to_list(exited, exited_count, &exited_cap,
            overlap_x_min, overlap_x_max,
            overlap_y_min, overlap_y_max,
            s_min_z, MIN(s_max_z, e_min_z - 1));
    }
    // Top face (Z-max)
    if (s_max_z > e_max_z) {
        add_cubes_to_list(exited, exited_count, &exited_cap,
            overlap_x_min, overlap_x_max,
            overlap_y_min, overlap_y_max,
            MAX(s_min_z, e_max_z + 1), s_max_z);
    }

    // ENTERED cubes (in end but not in start)
    // Left face (X-min)
    if (e_min_x < s_min_x) {
        add_cubes_to_list(entered, entered_count, &entered_cap,
            e_min_x, MIN(e_max_x, s_min_x - 1),
            e_min_y, e_max_y,
            e_min_z, e_max_z);
    }
    // Right face (X-max)
    if (e_max_x > s_max_x) {
        add_cubes_to_list(entered, entered_count, &entered_cap,
            MAX(e_min_x, s_max_x + 1), e_max_x,
            e_min_y, e_max_y,
            e_min_z, e_max_z);
    }
    // Front face (Y-min)
    if (e_min_y < s_min_y) {
        add_cubes_to_list(entered, entered_count, &entered_cap,
            overlap_x_min, overlap_x_max,
            e_min_y, MIN(e_max_y, s_min_y - 1),
            e_min_z, e_max_z);
    }
    // Back face (Y-max)
    if (e_max_y > s_max_y) {
        add_cubes_to_list(entered, entered_count, &entered_cap,
            overlap_x_min, overlap_x_max,
            MAX(e_min_y, s_max_y + 1), e_max_y,
            e_min_z, e_max_z);
    }
    // Bottom face (Z-min)
    if (e_min_z < s_min_z) {
        add_cubes_to_list(entered, entered_count, &entered_cap,
            overlap_x_min, overlap_x_max,
            overlap_y_min, overlap_y_max,
            e_min_z, MIN(e_max_z, s_min_z - 1));
    }
    // Top face (Z-max)
    if (e_max_z > s_max_z) {
        add_cubes_to_list(entered, entered_count, &entered_cap,
            overlap_x_min, overlap_x_max,
            overlap_y_min, overlap_y_max,
            MAX(e_min_z, s_max_z + 1), e_max_z);
    }
}
// Chunk update functions
void update_nearby_chunks(
    Chunk** planet,
    int world_x,
    int world_y,
    int world_z,
    int render_distance,
    vec3 position,
    vec3int* previous_chunk_center,
    const uint64_t* texture_handles
) {
    // Calculate current chunk center
    const int player_chunk_x = (int)floor(position[0]) / CHUNK_SIZE;
    const int player_chunk_y = (int)floor(position[1]) / CHUNK_SIZE;
    const int player_chunk_z = (int)floor(position[2]) / CHUNK_SIZE;
    const vec3int current_center = {player_chunk_x, player_chunk_y, player_chunk_z};
    
    // Return if center hasn't changed
    if (previous_chunk_center->x == current_center.x &&
        previous_chunk_center->y == current_center.y &&
        previous_chunk_center->z == current_center.z) {
        return;
    }
    
    PROFILE_BEGIN(update_nearby_chunks);
    
    // Calculate entered/exited chunks
    vec3int* entered = NULL;
    vec3int* exited = NULL;
    size_t entered_count = 0;
    size_t exited_count = 0;
    
    get_entered_exited_cubes(
        *previous_chunk_center,
        current_center,
        render_distance,
        &entered, &entered_count,
        &exited, &exited_count);
    
    // Update previous center
    *previous_chunk_center = current_center;
    
    // Process exited chunks
    PROFILE_BEGIN(Remove_chunks);
    for (int i = 0; i < exited_count; i++) {
        if (exited[i].y < 0 || exited[i].y > world_y){continue;}

        vec3int wrapped = exited[i];
        wrap_coordinates(world_x, world_y, world_z, 
                        &wrapped.x, &wrapped.y, &wrapped.z);
        
        Chunk* chunk = chunk_find(planet, &wrapped);
        if (chunk) {
            chunk_remove(planet, &chunk->Key);
        }
    }
    PROFILE_END(Remove_chunks);
    // Process entered chunks
    PROFILE_BEGIN(Add_chunks);
    printf("%zu\n", entered_count);
    printf("%zu\n", exited_count);
    for (int i = 0; i < entered_count; i++) {
        if (entered[i].y < 0 || entered[i].y > world_y){continue;}
        vec3int wrapped = entered[i];
        wrap_coordinates(world_x, world_y, world_z, 
                        &wrapped.x, &wrapped.y, &wrapped.z);
        
        vec3int key = wrapped;
        Chunk* chunk = chunk_find(planet, &key);
        
        if (!chunk) {
            chunk = chunk_add(planet, &key, texture_handles);
            chunk->isdirty = true;
        }
        chunk->is_active = true;
    }
    PROFILE_END(Add_chunks);

    // Clean up
    free(entered);
    free(exited);
    PROFILE_END(update_nearby_chunks);
}