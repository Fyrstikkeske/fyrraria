#pragma once

#include "cglm/types.h"
#include "meshing.h"
#include "utils.h"
#include "uthash.h"
#include <stdbool.h>
#include <stdlib.h>

// Chunk management using hashmap
void chunk_add(Chunk** planet, const vec3int* key) {
    Chunk* chunk = malloc(sizeof(Chunk));
    if (!chunk) {
        fprintf(stderr, "Memory allocation failed for chunk!\n");
        exit(EXIT_FAILURE);
    }
    chunk->Key = *key;
    chunk->isdirty = true;
    glGenVertexArrays(1, &chunk->VAO);
    glGenBuffers(1,&chunk->VBO);

    HASH_ADD(hh, *planet, Key, sizeof(vec3int), chunk);
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
    *y = (*y % world_y + world_y) % world_y;
    *z = (*z % world_z + world_z) % world_z;
}

// Chunk update functions
void update_nearby_chunks(
    Chunk** planet,
    int world_x,
    int world_y,
    int world_z,
    int render_distance,
    vec3 position
) {
    const int num_chunks = render_distance * render_distance * render_distance;
    
    // Mark all existing chunks as inactive
    for (Chunk* c = *planet; c != NULL; c = c->hh.next) {
        c->is_active = false;
    }

    for (int i = 0; i < num_chunks; i++) {
        int chunk_x, chunk_y, chunk_z;
        calculate_chunk_coords(position, render_distance, i,   &chunk_x, &chunk_y, &chunk_z);
        wrap_coordinates(world_x, world_y, world_z, &chunk_x, &chunk_y, &chunk_z);

        vec3int key = {chunk_x, chunk_y, chunk_z};
        Chunk* chunk = chunk_find(planet, &key);
        
        if (!chunk) {
            // Create new chunk if it doesn't exist
            chunk_add(planet, &key);
            chunk = chunk_find(planet, &key);
            chunk->Key = key;
        } else {
            // Update existing chunk
            bool coordinates_changed = 
                (chunk->Key.x != chunk_x) ||
                (chunk->Key.y != chunk_y) ||
                (chunk->Key.z != chunk_z);
                
            if (coordinates_changed) {
                chunk->Key = key;
                chunk->isdirty = true;
            }
        }
        
        chunk->is_active = true;
    }

    // Remove chunks that are no longer in view
    Chunk *current, *tmp;
    HASH_ITER(hh, *planet, current, tmp) {
        if (!current->is_active) {
            chunk_remove(planet, &current->Key);
        }
    }
}

// Chunk generation
static inline void generate_active_chunks(
    Chunk** planet,
    const uint64_t* handles
) {
    Chunk *chunk, *tmp;
    HASH_ITER(hh, *planet, chunk, tmp) {
        if (!chunk->isdirty) continue;
        chunk->isdirty = false;

        // Reset all blocks to air
        for (int i = 0; i < CHUNK_VOLUME; i++) {
            chunk->blocks[i].type = AIR;
        }

        // Generate terrain features
        for (int block_idx = 0; block_idx < CHUNK_VOLUME; block_idx++) {
            const int local_x = block_idx % CHUNK_SIZE;
            const int local_y = (block_idx / CHUNK_SIZE) % CHUNK_SIZE;
            const int local_z = block_idx / (CHUNK_SIZE * CHUNK_SIZE);

            const int global_x = chunk->Key.x * CHUNK_SIZE + local_x;
            const int global_y = chunk->Key.y * CHUNK_SIZE + local_y;
            const int global_z = chunk->Key.z * CHUNK_SIZE + local_z;

            // Ground layer
            if (global_y == 0) {
                chunk->blocks[block_idx].type = GRASS;
            }
            
            // Tree trunk - 4 logs high
            if (global_x == 3 && global_z == 3) {
                if (global_y >= 3 && global_y <= 6) {
                    chunk->blocks[block_idx].type = WOOD_LOG;
                }
            }
            
            // Leaf canopy - two layers with full coverage
            const int dx = abs(global_x - 3);
            const int dz = abs(global_z - 3);
            const int dist = dx + dz;  // Manhattan distance
            
            // Layer 1 (y=5-6)
            if (global_y >= 5 && global_y <= 6) {
                // Full 3x3 area at y=5-6
                if (dx <= 1 && dz <= 1) {
                    // Only place leaves where there's no trunk
                    if (!(dx == 0 && dz == 0)) {
                        chunk->blocks[block_idx].type = LEAF;
                    }
                }
            }
            
            // Layer 2 (y=7)
            if (global_y == 7) {
                // Plus shape at y=7
                if ((dx == 0 && dz <= 1) || (dz == 0 && dx <= 1)) {
                    chunk->blocks[block_idx].type = LEAF;
                }
            }
            
            // Layer 3 (y=8)
            if (global_y == 8) {
                // Single leaf at top center
                if (dx == 0 && dz == 0) {
                    chunk->blocks[block_idx].type = LEAF;
                }
            }
        }
        generate_mesh_for_chunk(chunk, handles);
    }
}