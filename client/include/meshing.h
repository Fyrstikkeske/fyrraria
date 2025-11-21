#pragma once

#include "utils.h"


void generate_chunk(Chunk* chunk, int chunk_x, int chunk_y, int chunk_z, uint8_t lod);

void generate_mesh_for_chunk(Chunk *chunk, const uint64_t* handles, vec3int world_size, int chunk_x, int chunk_y, int chunk_z, uint8_t lod);