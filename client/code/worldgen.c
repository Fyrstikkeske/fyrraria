#include "cglm/types.h"
#include "gl.h"
#include "utils.h"
#include "cc.h"
#include "FastNoiseLite.h"
#include "meshing.h"
#include <X11/Xlib.h>
#include <cglm/cglm.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static inline Chunk* chunk_add(const vec3int key, const uint64_t* texture_handles, char lod, map(vec3int, Chunk)* lod_layer, vec3int world_size) {
    Chunk chunk;

    chunk.isdirty = true;
    chunk.lod = lod;
    glGenVertexArrays(1, &chunk.VAO);
    glGenBuffers(1,&chunk.VBO);
    
    generate_chunk(&chunk, key.x, key.y, key.z, lod);

    generate_mesh_for_chunk(&chunk, texture_handles, world_size, key.x, key.y, key.z, lod);

    Chunk* inserted = insert(lod_layer, key, chunk);
    if (!inserted) {
        fprintf(stderr, "Failed to insert chunk into LOD layer map.\n");
        abort();
    }
    return inserted;
}

static inline Chunk* chunk_find(const vec3int key, map(vec3int, Chunk)* lod_layer) {
    Chunk *chunk = get(lod_layer, key);
    return chunk;
}

static inline void chunk_remove(const vec3int key, map(vec3int, Chunk)* lod_layer) {


    Chunk* chunk = chunk_find(key, lod_layer);
    if (chunk) {

        glDeleteBuffers(1,&chunk->VBO);
        glDeleteVertexArrays(1, &chunk->VAO);

        erase(lod_layer, key);
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
    const vec3int world,
    vec3int* chunk_pos
) {
    chunk_pos->x = (chunk_pos->x % world.x + world.x) % world.x;
    chunk_pos->z = (chunk_pos->z % world.z + world.z) % world.z;

    chunk_pos->y = CLAMP(chunk_pos->y, 0, world.y - 1);
}

static inline void recursive_genChunkfunction(
    vec( map(vec3int, Chunk) ) *lod_layers,
    const vec3u8 world_size,
    int render_distance,
    vec3int chunk_rendering,
    uint8_t layer,
    const vec3int current_center,  // Player position in LOD 0 chunks
    const uint64_t* texture_handles
){
    vec3int lod_chunk_at = {
        current_center.x >> layer,
        current_center.y >> layer,
        current_center.z >> layer
    };

    /*printf("Layer %d: chunk_rendering=(%d,%d,%d), lod_chunk_at=(%d,%d,%d)\n", 
           layer, chunk_rendering.x, chunk_rendering.y, chunk_rendering.z,
           lod_chunk_at.x, lod_chunk_at.y, lod_chunk_at.z);*/

    // Calculate distance from player's chunk to this chunk at current LOD
    int dx = abs(chunk_rendering.x - lod_chunk_at.x);
    int dy = abs(chunk_rendering.y - lod_chunk_at.y);
    int dz = abs(chunk_rendering.z - lod_chunk_at.z);
    int max_dist = MAX(dx, MAX(dy, dz));

    
    if (max_dist < render_distance && layer > 0) {
        // Subdivide into 8 children at finer LOD
        for (int ox = 0; ox < 2; ox++) {
            for (int oy = 0; oy < 2; oy++) {
                for (int oz = 0; oz < 2; oz++) {
                    vec3int child_pos = {
                        (chunk_rendering.x * 2) + ox,
                        (chunk_rendering.y * 2) + oy,
                        (chunk_rendering.z * 2) + oz
                    };
                    
                    recursive_genChunkfunction(
                        lod_layers,
                        world_size,
                        render_distance,
                        child_pos,
                        layer - 1,
                        current_center,
                        texture_handles
                    );
                }
            }
        }
        return; // Don't render this parent chunk
    }

    // We're either far from player OR at finest LOD - render this chunk
    const vec3int transformed = {
        chunk_rendering.x,
        chunk_rendering.y,
        chunk_rendering.z
    };

    // Calculate world size for chunk generation
    // At LOD layer L, each chunk represents (1 << L) base chunks
    // So world size in base chunks is (base_world_size << L)
    const vec3int world_size_of_that_layer = {
        (1 << world_size.x) << layer,  // Total blocks at this LOD
        (1 << world_size.y) << layer,
        (1 << world_size.z) << layer
    };

    map(vec3int, Chunk) *chunklayer = get(lod_layers, layer);
    Chunk* chunk = chunk_find(transformed, chunklayer);
    if (!chunk) {
        chunk_add(
            transformed,
            texture_handles,
            layer,  // Pass the actual LOD level, not 0!
            chunklayer,
            world_size_of_that_layer
        );
    }
}

// Chunk update functions
void update_nearby_chunks(
    vec( map(vec3int, Chunk) ) *lod_layers,
    const vec3u8 world_size,
    int render_distance,
    vec3 position,
    vec3int* previous_chunk_center,
    const uint64_t* texture_handles
) {

    //Just get it to work, no need to have efficient logic yet
    const int player_chunk_x = (int)floor(position[0]) / CHUNK_SIZE;
    const int player_chunk_y = (int)floor(position[1]) / CHUNK_SIZE;
    const int player_chunk_z = (int)floor(position[2]) / CHUNK_SIZE;
    const vec3int current_center = {player_chunk_x, player_chunk_y, player_chunk_z};

    //player didnt move beetwen chunks
    if (!memcmp(&current_center, previous_chunk_center, sizeof(vec3int))){
        return;
    }

    //clean_chunk_all, saving can be done later once generation is perfected or close
    for_each(lod_layers, layer){
        for_each(layer, key, _){
            chunk_remove(*key, layer);
        }
    }

    // 6 4 4

    // 3 1 1

    //world_size is a .xyz struct, now whats unique is that the size is logarythmic representation. so to get the true size use 1 << world_size.xyz

    // therefore top level would be the largest - smallest + 1
    // WHAT AM I THINKGING?!?!?
    // top level refers to the largest level of lod or something


    //Start at the top level and go down. o algo

    // how_many_lods should be the smallest value of world_size.xyz i think
    uint8_t layeratnow = MIN(world_size.x, MIN(world_size.y, world_size.z));

    // Compute how many chunks exist at this LOD for each axis.
    // At LOD = L, chunk count = 2^(world_size.axis - L)
    vec3u8 chunk_counts = {
        (uint8_t)(1 << (world_size.x - layeratnow)),
        (uint8_t)(1 << (world_size.y - layeratnow)),
        (uint8_t)(1 << (world_size.z - layeratnow))
    };

    // Now loop over all valid chunks at this LOD
    for (uint8_t x = 0; x < chunk_counts.x; x++) {
        for (uint8_t y = 0; y < chunk_counts.y; y++) {
            for (uint8_t z = 0; z < chunk_counts.z; z++) {

                recursive_genChunkfunction(
                    lod_layers,
                    world_size,
                    render_distance,
                    (vec3int){x, y, z},
                    layeratnow,
                    current_center,
                    texture_handles
                );
            }
        }
    }

    //most of the code under here is to be removed
    //As the new method with the recursive function will make it obsolete
    *previous_chunk_center = current_center;
    return;

    int iter = 0;
    
    for_each(lod_layers, layer){

        vec3int local_world_size = {
            world_size.x >> iter,
            world_size.y >> iter,
            world_size.z >> iter
        };

        if (local_world_size.x <= 0 ||
            local_world_size.z <= 0 
        ){break;}

        const vec3int local_current_center = {
            .x = player_chunk_x >> iter,
            .y = player_chunk_y >> iter,
            .z = player_chunk_z >> iter
        };

        const vec3int local_previous_chunk_center = {
            .x = previous_chunk_center->x >> iter,
            .y = previous_chunk_center->y >> iter,
            .z = previous_chunk_center->z >> iter
        };

        if (local_previous_chunk_center.x == local_current_center.x &&
            local_previous_chunk_center.y == local_current_center.y &&
            local_previous_chunk_center.z == local_current_center.z) 
        {break;}

        vec3int lower_bounding_box = {
            local_current_center.x - render_distance, 
            local_current_center.y - render_distance, 
            local_current_center.z - render_distance};
        vec3int upper_bounding_box = {
            local_current_center.x + render_distance, 
            local_current_center.y + render_distance,
            local_current_center.z + render_distance};

        map(vec3int, char8_t) chunks_to_remove;
        init(&chunks_to_remove);

        for_each(layer, key, _){
            insert(&chunks_to_remove, *key, 0);
        }

        for (int32_t x = lower_bounding_box.x; x < upper_bounding_box.x; x++){
            for (int32_t y = lower_bounding_box.y; y < upper_bounding_box.y; y++){
                for (int32_t z = lower_bounding_box.z; z < upper_bounding_box.z; z++){



                    vec3int wrapped = {x, y, z};
                    wrap_coordinates(local_world_size, &wrapped);
                    erase(&chunks_to_remove, wrapped);

                    Chunk* chunk = chunk_find(wrapped, layer);
                    if (!chunk) {
                        chunk_add(
                            wrapped,
                            texture_handles,
                            0,
                            layer,
                            local_world_size
                        );
                    }
                }
            }
        }

        if (iter != 0){
                vec3int inner_lower_bounding_box = {
                local_current_center.x - log(render_distance), 
                local_current_center.y - log(render_distance), 
                local_current_center.z - log(render_distance)};
            vec3int inner_upper_bounding_box = {
                local_current_center.x + log(render_distance),
                local_current_center.y + log(render_distance),
                local_current_center.z + log(render_distance)};

            for (int32_t x = inner_lower_bounding_box.x; x < inner_upper_bounding_box.x; x++){
                for (int32_t y = inner_lower_bounding_box.y; y < inner_upper_bounding_box.y; y++){
                    for (int32_t z = inner_lower_bounding_box.z; z < inner_upper_bounding_box.z; z++){
                        vec3int wrapped = {x, y, z};
                        wrap_coordinates(local_world_size, &wrapped);

                        char8_t *chunk = get(&chunks_to_remove, wrapped);

                        if (!chunk) {
                            insert(&chunks_to_remove, wrapped, 0);
                        }
                    }
                }
            }
        }
        


        for_each(&chunks_to_remove, key, _){
            chunk_remove(*key, layer);
        }
        cleanup(&chunks_to_remove );
        iter++;
    }
}