#pragma once

#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
constexpr int vertexSize = 3+2+1+1;

constexpr int faceSize = vertexSize * 6;

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

static inline void generate_face_vertices(float* buffer, int offset, int axis, 
    int globalx, int globaly, int globalz,
    float floatLo, float floatHi) {
float face[faceSize];

switch (axis) {
case 0: // +Y
memcpy(face, (float[]){
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 1.0f, 0.0f, 0.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 0.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 0.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 0.0f, globalz + 0.0f, 1.0f, 1.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi
}, sizeof(face));
break;
case 1: // -Y (fixed winding order)
memcpy(face, (float[]){
  globalx + 0.0f, globaly + 1.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 1.0f, globalz + 0.0f, 1.0f, 1.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 1.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 1.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
globalx + 0.0f, globaly + 1.0f, globalz + 1.0f, 0.0f, 0.0f, floatLo, floatHi,
globalx + 0.0f, globaly + 1.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi

}, sizeof(face));
break;
case 2: // +X (fixed texture coords)
memcpy(face, (float[]){
  globalx + 0.0f, globaly + 1.0f, globalz + 1.0f, 0.0f, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 1.0f, 0.0f, 1.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 1.0f, globalz + 0.0f, 1.0f, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 1.0f, 1.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 1.0f, globalz + 0.0f, 1.0f, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 1.0f, 0.0f, 1.0f, floatLo, floatHi
}, sizeof(face));
break;
case 3: // -X (fixed vertex/texture coords)
memcpy(face, (float[]){
globalx + 1.0f, globaly + 1.0f, globalz + 0.0f, 0.0f, 0.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 1.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 0.0f, globalz + 1.0f, 1.0f, 1.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 1.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
globalx + 1.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi
}, sizeof(face));
break;
case 4: // +Z (fixed vertex/texture coords)
memcpy(face, (float[]){
  
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 0.0f, globalz + 0.0f, 1.0f, 1.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 1.0f, globalz + 0.0f, 1.0f, 0.0f, floatLo, floatHi,
  
  globalx + 1.0f, globaly + 1.0f, globalz + 0.0f, 1.0f, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 1.0f, globalz + 0.0f, 0.0f, 0.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi
}, sizeof(face));
break;
case 5: // -Z (fixed texture coords)
memcpy(face, (float[]){
  globalx + 0.0f, globaly + 0.0f, globalz + 1.0f, 0.0f, 1.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 1.0f, globalz + 1.0f, 0.0f, 0.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 1.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 1.0f, globalz + 1.0f, 1.0f, 0.0f, floatLo, floatHi,
  globalx + 1.0f, globaly + 0.0f, globalz + 1.0f, 1.0f, 1.0f, floatLo, floatHi,
  globalx + 0.0f, globaly + 0.0f, globalz + 1.0f, 0.0f, 1.0f, floatLo, floatHi
}, sizeof(face));
break;
}

memcpy(&buffer[offset], face, sizeof(face));
}

static inline void generate_mesh_for_chunk(
Chunk *chunk,
const uint64_t* handles
) {
// Axis occlusion buffers
uint16_t axis_occlusion[3 * CHUNK_AREA] = {0};
uint16_t occlusion_face_masks[6 * CHUNK_AREA] = {0};

// Build axis occlusion arrays
for (int blockiter = 0; blockiter < CHUNK_VOLUME; blockiter++) {
const int x = blockiter % CHUNK_SIZE;
const int y = (blockiter / CHUNK_SIZE) % CHUNK_SIZE;
const int z = blockiter / (CHUNK_SIZE * CHUNK_SIZE);

if (DoesBlockOcclude[chunk->blocks[blockiter].type]) {
// Y axis (x,z plane)
axis_occlusion[x + z * CHUNK_SIZE] |= (1u << y);
// X axis (y,z plane)
axis_occlusion[CHUNK_AREA + y + z * CHUNK_SIZE] |= (1u << x);
// Z axis (x,y plane)
axis_occlusion[2*CHUNK_AREA + x + y * CHUNK_SIZE] |= (1u << z);
}
}

// Generate face masks
for (int axis = 0; axis < 3; axis++) {
for (int i = 0; i < CHUNK_AREA; i++) {
uint16_t occ = axis_occlusion[axis * CHUNK_AREA + i];
// Negative direction (left/down/back)
occlusion_face_masks[(axis * 2 + 0) * CHUNK_AREA + i] = occ & ~(occ << 1);
// Positive direction (right/up/forward)
occlusion_face_masks[(axis * 2 + 1) * CHUNK_AREA + i] = occ & ~(occ >> 1);
}
}

// Count visible faces
int meshableFaces = 0;
for (int i = 0; i < 6 * CHUNK_AREA; i++) {
meshableFaces += __builtin_popcount(occlusion_face_masks[i]);
}

chunk->vertices = faceSize * meshableFaces;
float* worldmeshes = (float*)malloc(sizeof(float) * faceSize * meshableFaces);
if (!worldmeshes) return; // Handle allocation failure

int offset = 0;

// Generate vertices for visible faces
for (int axis = 0; axis < 6; axis++) {
for (int i = 0; i < CHUNK_AREA; i++) {
uint16_t mask = occlusion_face_masks[axis * CHUNK_AREA + i];

while (mask) {
uint16_t bit = __builtin_ctz(mask);
mask &= mask - 1; // Clear lowest set bit

vec3int voxelpos;

// Determine voxel position based on axis
if (axis < 2) { // Y faces (0:+Y, 1:-Y)
int grid_x = i / CHUNK_SIZE;
int grid_z = i % CHUNK_SIZE;
voxelpos = (vec3int){grid_z, bit, grid_x};
}
else if (axis < 4) { // X faces (2:+X, 3:-X)
int grid_y = i / CHUNK_SIZE;
int grid_z = i % CHUNK_SIZE;
voxelpos = (vec3int){bit, grid_z, grid_y};
}
else { // Z faces (4:+Z, 5:-Z)
int grid_x = i / CHUNK_SIZE;
int grid_y = i % CHUNK_SIZE;
voxelpos = (vec3int){grid_y, grid_x, bit};
}

int blockidx = voxelpos.x + CHUNK_SIZE * (voxelpos.y + CHUNK_SIZE * voxelpos.z);
enum Blocktype blocktype = chunk->blocks[blockidx].type;

// Get global coordinates
int globalx = chunk->Key.x * CHUNK_SIZE + voxelpos.x;
int globaly = chunk->Key.y * CHUNK_SIZE + voxelpos.y;
int globalz = chunk->Key.z * CHUNK_SIZE + voxelpos.z;

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
  floatLo, floatHi);
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