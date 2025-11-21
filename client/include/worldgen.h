#pragma once


#include "utils.h"
#include "cc.h"

// GG, down is old idk what it was for.
// the Ide straight up reports this as an error if it isnt there, but of course the compiler marks this as a warning. fucked regardless

void update_nearby_chunks(vec( map(vec3int, Chunk) ) *lod_layers, vec3u8 world_size, int render_distance,vec3 position,vec3int* previous_chunk_center,const uint64_t* texture_handles);


//WHAT THE FUCK IS THIS CODEBASE.
//look at the other files, holy hell