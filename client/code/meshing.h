#pragma once

#include "utils.h"
#include "worldgen.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
constexpr int vertexSize = 3+2+1+1;

constexpr int faceSize = vertexSize * 6;


//should be in worldgen.h but screw that due to circular dependencies. fuck this shit
static inline void generate_chunk(
  Chunk* chunk, int chunk_x, int chunk_y, int chunk_z 
){
  

  memset(chunk->blocks, AIR, CHUNK_VOLUME * sizeof(enum Blocktype));

  // Generate terrain features
  const int base_x = chunk_x * CHUNK_SIZE;
  const int base_y = chunk_y * CHUNK_SIZE;
  const int base_z = chunk_z * CHUNK_SIZE;

  // Configure noise generator
  fnl_state noise = fnlCreateState();
  noise.seed = 1337;
  noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
  noise.frequency = 0.01;

  for (int x = 0; x < CHUNK_SIZE; ++x) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
          int global_x = base_x + x;
          int global_z = base_z + z;

          // Generate height using noise
          float height_noise = fnlGetNoise2D(&noise, (int)global_x, (int)global_z);
          

          float ridge = 1.0 + ((1.0 + -1 * (fabsf(height_noise) / 1.0)) * -1.0);
          
          int height = (int)(ridge * 20.0f) - 12; // Adjust height scale and offset

          for (int y = 0; y < CHUNK_SIZE; ++y) {
              int global_y = base_y + y;

              int block_idx = x + CHUNK_SIZE * (y + CHUNK_SIZE * z);

              if (global_y <= height) {
                  chunk->blocks[block_idx].type = GRASS;
              }
              if (global_x == 0 && global_z == 0){
                chunk->blocks[block_idx].type = WOOD_LOG;
              }
              if (global_y == 0){chunk->blocks[block_idx].type = WATER;}
          }
      }
  }
}

//MAKE MESHESHEESA DSA
// it are over 
//TODO FIX THIS PIECE OF SHIT UP

//Fuxing this piece of shit sir
//10% booting up the niggachain
//20% GEORGE DROID IS HERE
//99% ok fr though
//99.9% GEORGE DROID method sucks
//99.99% Only called when needed now, But no lods :sad:
//99.999% RAAAAAHHHH
//100% its fucking DONE
//102% WE ARE SO BACK, beetwen chunks done(not counting the world border but fuck that for now) and performance is shit since world gen is run 6x more

static inline void generate_face_vertices(float* buffer, int offset, int axis, 
    int globalx, int globaly, int globalz,
    float floatLo, float floatHi, char lod) {
float face[faceSize];
    float thelod = pow(2, lod);
switch (axis) {
case 0: // +Y
memcpy(face, (float[]){
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + thelod, 0.0f, 0.0f, floatLo, floatHi,
  globalx + thelod, globaly + 0.0f, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
  globalx + thelod, globaly + 0.0f, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
  globalx + thelod, globaly + 0.0f, globalz + 0.0f, thelod, thelod, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi
}, sizeof(face));
break;
case 1: // -Y (fixed winding order)
memcpy(face, (float[]){
  globalx + 0.0f, globaly + thelod, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi,
globalx + thelod, globaly + thelod, globalz + 0.0f, thelod, thelod, floatLo, floatHi,
globalx + thelod, globaly + thelod, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
globalx + thelod, globaly + thelod, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
globalx + 0.0f, globaly + thelod, globalz + thelod, 0.0f, 0.0f, floatLo, floatHi,
globalx + 0.0f, globaly + thelod, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi

}, sizeof(face));
break;
case 2: // +X (fixed texture coords)
memcpy(face, (float[]){
  globalx + 0.0f, globaly + thelod, globalz + thelod, 0.0f, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + thelod, 0.0f, thelod, floatLo, floatHi,
  globalx + 0.0f, globaly + thelod, globalz + 0.0f, thelod, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, thelod, thelod, floatLo, floatHi,
  globalx + 0.0f, globaly + thelod, globalz + 0.0f, thelod, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + thelod, 0.0f, thelod, floatLo, floatHi
}, sizeof(face));
break;
case 3: // -X (fixed vertex/texture coords)
memcpy(face, (float[]){
globalx + thelod, globaly + thelod, globalz + 0.0f, 0.0f, 0.0f, floatLo, floatHi,
globalx + thelod, globaly + 0.0f, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi,
globalx + thelod, globaly + thelod, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
globalx + thelod, globaly + 0.0f, globalz + thelod, thelod, thelod, floatLo, floatHi,
globalx + thelod, globaly + thelod, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
globalx + thelod, globaly + 0.0f, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi
}, sizeof(face));
break;
case 4: // +Z (fixed vertex/texture coords)
memcpy(face, (float[]){
  
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi,
  globalx + thelod, globaly + 0.0f, globalz + 0.0f, thelod, thelod, floatLo, floatHi,
  globalx + thelod, globaly + thelod, globalz + 0.0f, thelod, 0.0f, floatLo, floatHi,
  
  globalx + thelod, globaly + thelod, globalz + 0.0f, thelod, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + thelod, globalz + 0.0f, 0.0f, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, thelod, floatLo, floatHi
}, sizeof(face));
break;
case 5: // -Z (fixed texture coords)
memcpy(face, (float[]){
  globalx + 0.0f, globaly + 0.0f, globalz + thelod, 0.0f, thelod, floatLo, floatHi,
  globalx + 0.0f, globaly + thelod, globalz + thelod, 0.0f, 0.0f, floatLo, floatHi,
  globalx + thelod, globaly + thelod, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
  globalx + thelod, globaly + thelod, globalz + thelod, thelod, 0.0f, floatLo, floatHi,
  globalx + thelod, globaly + 0.0f, globalz + thelod, thelod, thelod, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + thelod, 0.0f, thelod, floatLo, floatHi
}, sizeof(face));
break;
}

memcpy(&buffer[offset], face, sizeof(face));
}



static inline void generate_mesh_for_chunk(
Chunk *chunk,
const uint64_t* handles,
int chunk_x, 
int chunk_y, 
int chunk_z
) {

  //this stinks, legit rots
  Chunk chunkup;
  generate_chunk(&chunkup, chunk_x, CLAMP(chunk_z + 1, 0, WORLD_Y), chunk_z);
  Chunk chunkdown;
  generate_chunk(&chunkdown, chunk_x, CLAMP(chunk_z - 1, 0, WORLD_Y), chunk_z);
  Chunk chunknorth;
  generate_chunk(&chunknorth, chunk_x, chunk_y, EUCLID_MODULO(chunk_z + 1, WORLD_Z));
  Chunk chunksouth;
  generate_chunk(&chunksouth, chunk_x, chunk_y, EUCLID_MODULO(chunk_z - 1, WORLD_Z));
  Chunk chunkwest;
  generate_chunk(&chunkwest, EUCLID_MODULO(chunk_x - 1, WORLD_X), chunk_y, chunk_z);
  Chunk chunkeast;
  generate_chunk(&chunkeast, EUCLID_MODULO(chunk_x + 1, WORLD_X), chunk_y, chunk_z);
  

  struct block padded_chunk_blocks[CHUNK_VOLUME_P] = {0};

    for (int iter = 0; iter < CHUNK_VOLUME_P; iter++) {
        const int x = iter % CHUNK_SIZE_P;
        const int y = (iter / CHUNK_SIZE_P) % CHUNK_SIZE_P;
        const int z = iter / (CHUNK_SIZE_P * CHUNK_SIZE_P);
        
        // Core chunk (current chunk)
        if (x >= 1 && x < CHUNK_SIZE_P-1 && 
            y >= 1 && y < CHUNK_SIZE_P-1 && 
            z >= 1 && z < CHUNK_SIZE_P-1) 
        {
            const int core_x = x - 1;
            const int core_y = y - 1;
            const int core_z = z - 1;
            const int idx = core_x + CHUNK_SIZE * (core_y + CHUNK_SIZE * core_z);
            padded_chunk_blocks[iter] = chunk->blocks[idx];
        }
        // West face (x=0)
        else if (x == 0) {
            const int west_x = CHUNK_SIZE - 1; // West neighbor's east face
            const int west_y = y - 1;
            const int west_z = z - 1;
            if (west_y >= 0 && west_y < CHUNK_SIZE && west_z >= 0 && west_z < CHUNK_SIZE) {
                const int idx = west_x + CHUNK_SIZE * (west_y + CHUNK_SIZE * west_z);
                padded_chunk_blocks[iter] = chunkwest.blocks[idx];
            }
        }
        // East face (x=CHUNK_SIZE+1)
        else if (x == CHUNK_SIZE_P - 1) {
            const int east_x = 0; // East neighbor's west face
            const int east_y = y - 1;
            const int east_z = z - 1;
            if (east_y >= 0 && east_y < CHUNK_SIZE && east_z >= 0 && east_z < CHUNK_SIZE) {
                const int idx = east_x + CHUNK_SIZE * (east_y + CHUNK_SIZE * east_z);
                padded_chunk_blocks[iter] = chunkeast.blocks[idx];
            }
        }
        // Down face (y=0)
        else if (y == 0) {
            const int down_x = x - 1;
            const int down_y = CHUNK_SIZE - 1; // Down neighbor's top face
            const int down_z = z - 1;
            if (down_x >= 0 && down_x < CHUNK_SIZE && down_z >= 0 && down_z < CHUNK_SIZE) {
                const int idx = down_x + CHUNK_SIZE * (down_y + CHUNK_SIZE * down_z);
                padded_chunk_blocks[iter] = chunkdown.blocks[idx];
            }
        }
        // Up face (y=CHUNK_SIZE+1)
        else if (y == CHUNK_SIZE_P - 1) {
            const int up_x = x - 1;
            const int up_y = 0; // Up neighbor's bottom face
            const int up_z = z - 1;
            if (up_x >= 0 && up_x < CHUNK_SIZE && up_z >= 0 && up_z < CHUNK_SIZE) {
                const int idx = up_x + CHUNK_SIZE * (up_y + CHUNK_SIZE * up_z);
                padded_chunk_blocks[iter] = chunkup.blocks[idx];
            }
        }
        // South face (z=0)
        else if (z == 0) {
            const int south_x = x - 1;
            const int south_y = y - 1;
            const int south_z = CHUNK_SIZE - 1; // South neighbor's north face
            if (south_x >= 0 && south_x < CHUNK_SIZE && south_y >= 0 && south_y < CHUNK_SIZE) {
                const int idx = south_x + CHUNK_SIZE * (south_y + CHUNK_SIZE * south_z);
                padded_chunk_blocks[iter] = chunksouth.blocks[idx];
            }
        }
        // North face (z=CHUNK_SIZE+1)
        else if (z == CHUNK_SIZE_P - 1) {
            const int north_x = x - 1;
            const int north_y = y - 1;
            const int north_z = 0; // North neighbor's south face
            if (north_x >= 0 && north_x < CHUNK_SIZE && north_y >= 0 && north_y < CHUNK_SIZE) {
                const int idx = north_x + CHUNK_SIZE * (north_y + CHUNK_SIZE * north_z);
                padded_chunk_blocks[iter] = chunknorth.blocks[idx];
            }
        }
    }


// Axis occlusion buffers
// Axis occlusion buffers - now using padded dimensions
uint64_t axis_occlusion[3 * CHUNK_AREA_P] = {0};
uint64_t occlusion_face_masks[6 * CHUNK_AREA_P] = {0};

// Build axis occlusion arrays using PADDED chunks
for (int blockiter = 0; blockiter < CHUNK_VOLUME_P; blockiter++) {
    const int x = blockiter % CHUNK_SIZE_P;
    const int y = (blockiter / CHUNK_SIZE_P) % CHUNK_SIZE_P;
    const int z = blockiter / (CHUNK_SIZE_P * CHUNK_SIZE_P);
    
    if (DoesBlockOcclude[padded_chunk_blocks[blockiter].type]) {
        // Y axis (x,z plane) - use padded coordinates
        axis_occlusion[x + z * CHUNK_SIZE_P] |= (1ULL << y);
        // X axis (y,z plane)
        axis_occlusion[CHUNK_AREA_P + y + z * CHUNK_SIZE_P] |= (1ULL << x);
        // Z axis (x,y plane)
        axis_occlusion[2*CHUNK_AREA_P + x + y * CHUNK_SIZE_P] |= (1ULL << z);
    }
}

// Generate face masks using PADDED dimensions
for (int axis = 0; axis < 3; axis++) {
    for (int i = 0; i < CHUNK_AREA_P; i++) {
        uint64_t occ = axis_occlusion[axis * CHUNK_AREA_P + i];
        // Negative direction (left/down/back)
        occlusion_face_masks[(axis * 2 + 0) * CHUNK_AREA_P + i] = occ & ~(occ << 1);
        // Positive direction (right/up/forward)
        occlusion_face_masks[(axis * 2 + 1) * CHUNK_AREA_P + i] = occ & ~(occ >> 1);
    }
}

// Count visible faces - skip padding (inner CHUNK_SIZE x CHUNK_SIZE area)
int meshableFaces = 0;
for (int axis = 0; axis < 6; axis++) {
    for (int i = CHUNK_SIZE_P; i < CHUNK_AREA_P - CHUNK_SIZE_P; i++) {
        // Skip padding rows (first and last in each dimension)
        if (i % CHUNK_SIZE_P == 0 || i % CHUNK_SIZE_P == CHUNK_SIZE_P - 1) continue;
        
        uint64_t mask = occlusion_face_masks[axis * CHUNK_AREA_P + i];
        // Remove padding bits from the mask
        mask = (mask >> 1) & ~(1ULL << CHUNK_SIZE);
        meshableFaces += __builtin_popcountll(mask);
    }
}

chunk->vertices = faceSize * meshableFaces;
float* worldmeshes = (float*)malloc(sizeof(float) * faceSize * meshableFaces);
if (!worldmeshes) return; // Handle allocation failure

int offset = 0;

// Generate vertices for visible faces (inner area only)
for (int axis = 0; axis < 6; axis++) {
    for (int i = CHUNK_SIZE_P; i < CHUNK_AREA_P - CHUNK_SIZE_P; i++) {
        // Skip padding rows
        if (i % CHUNK_SIZE_P == 0 || i % CHUNK_SIZE_P == CHUNK_SIZE_P - 1) continue;
        
        uint64_t mask = occlusion_face_masks[axis * CHUNK_AREA_P + i];
        // Remove padding bits from the mask
        mask = (mask >> 1) & ~(1ULL << CHUNK_SIZE);
        
        while (mask) {
            uint64_t bit = __builtin_ctzll(mask);
            mask &= mask - 1; // Clear lowest set bit

            vec3int voxelpos;
            int grid_x, grid_z;

            // Convert flat index to 2D coordinates
            grid_x = i % CHUNK_SIZE_P;
            grid_z = i / CHUNK_SIZE_P;

            // Adjust for padding offset (-1)
            grid_x--;
            grid_z--;

            // Determine voxel position based on axis
            if (axis < 2) { // Y faces (0:+Y, 1:-Y)
                voxelpos = (vec3int){grid_x, bit, grid_z};
            }
            else if (axis < 4) { // X faces (2:+X, 3:-X)
                voxelpos = (vec3int){bit, grid_x, grid_z};
            }
            else { // Z faces (4:+Z, 5:-Z)
                voxelpos = (vec3int){grid_x, grid_z, bit};
            }

            // Calculate block index in ORIGINAL chunk (without padding)
            int blockidx = voxelpos.x + CHUNK_SIZE * (voxelpos.y + CHUNK_SIZE * voxelpos.z);
            enum Blocktype blocktype = chunk->blocks[blockidx].type;

            // Get global coordinates
            int globalx = chunk_x * CHUNK_SIZE + voxelpos.x * pow(2, chunk->lod);
            int globaly = chunk_y * CHUNK_SIZE + voxelpos.y * pow(2, chunk->lod);
            int globalz = chunk_z * CHUNK_SIZE + voxelpos.z * pow(2, chunk->lod);

            // Handle texture
            uint64_t texHandle = handles[blocktype];
            uint32_t handleLo = (uint32_t)(texHandle);
            uint32_t handleHi = (uint32_t)(texHandle >> 32);
            float floatLo, floatHi;
            memcpy(&floatLo, &handleLo, sizeof(float));
            memcpy(&floatHi, &handleHi, sizeof(float));

            // Generate face vertices
            generate_face_vertices(worldmeshes, offset, axis, 
                globalx, globaly, globalz, 
                floatLo, floatHi, chunk->lod);
            offset += faceSize;
        }
    }
}

// Upload to GPU
glBindVertexArray(chunk->VAO);
glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(float) * faceSize * meshableFaces, 
worldmeshes, GL_STATIC_DRAW);
free(worldmeshes);

// Vertex attributes
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
glEnableVertexAttribArray(0);  
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
glEnableVertexAttribArray(2);
glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(6 * sizeof(float)));
glEnableVertexAttribArray(3);   
}