#pragma once

#include "cglm/types.h"
#include "meshing.h"
#include "utils.h"

#include <X11/Xlib.h>

#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE


void setChunkInfoForNewlyGeneratedChunk(
    int Worldx,
    int Worldy,
    int Worldz,
    Chunk *nearbyChunks,
    int iter,
    int renderdistance,
    vec3 position
){

    int playerchunkx = position[0]/chunksize;
    int playerchunky = position[1]/chunksize;
    int playerchunkz = position[2]/chunksize;

    int localx = iter % renderdistance;
    int localy = (iter / renderdistance) % renderdistance;
    int localz = iter / (renderdistance * renderdistance);

    int chunkx = playerchunkx + (localx - renderdistance/2);
    int chunky = playerchunky + (localy - renderdistance/2);
    int chunkz = playerchunkz + (localz - renderdistance/2);

    
    nearbyChunks[iter].cord.x = (chunkx % Worldx + Worldx) % Worldx;
    nearbyChunks[iter].cord.y = (chunky % Worldy + Worldy) % Worldy;
    nearbyChunks[iter].cord.z = (chunkz % Worldz + Worldz) % Worldz;
    
    nearbyChunks[iter].isdirty = True;
}

static inline void chunkUpdator(
    int Worldx,
    int Worldy,
    int Worldz,
    Chunk *nearbyChunks,
    int len,
    int renderdistance,
    vec3 position
){
    int playerchunkx = position[0] / chunksize;
    int playerchunky = position[1] / chunksize;
    int playerchunkz = position[2] / chunksize;

    for (int i = 0; i < len; i++) {
        int localx = i % renderdistance;
        int localy = (i / renderdistance) % renderdistance;
        int localz = i / (renderdistance * renderdistance);

        int chunkx = playerchunkx + (localx - renderdistance/2);
        int chunky = playerchunky + (localy - renderdistance/2);
        int chunkz = playerchunkz + (localz - renderdistance/2);
        
        chunkx = (chunkx % Worldx + Worldx) % Worldx;
        chunky = (chunky % Worldy + Worldy) % Worldy;
        chunkz = (chunkz % Worldz + Worldz) % Worldz;

        // Only update if coordinates changed
        if (nearbyChunks[i].cord.x != chunkx ||
            nearbyChunks[i].cord.y != chunky ||
            nearbyChunks[i].cord.z != chunkz) 
        {
            nearbyChunks[i].cord.x = chunkx;
            nearbyChunks[i].cord.y = chunky;
            nearbyChunks[i].cord.z = chunkz;
            nearbyChunks[i].isdirty = true;
        }
    }
}




static inline void genNearbyChunks(
    int Worldx,
    int Worldy,
    int Worldz,
    Chunk *nearbyChunks,
    int len,
    const uint64_t* handles
    ){
    for (int chunkiter = 0; chunkiter < len; chunkiter++) {
        if (nearbyChunks[chunkiter].isdirty == false){continue;}
        nearbyChunks[chunkiter].isdirty = false;

        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++) {
            nearbyChunks[chunkiter].blocks[blockiter].type = air;

            int localx = blockiter % chunksize;
            int localy = (blockiter / chunksize) % chunksize;
            int localz = blockiter / (chunksize * chunksize);

            int globalx = nearbyChunks[chunkiter].cord.x * chunksize + localx;
            int globaly = nearbyChunks[chunkiter].cord.y * chunksize + localy;
            int globalz = nearbyChunks[chunkiter].cord.z * chunksize + localz;


            if (globaly == 0){
                nearbyChunks[chunkiter].blocks[blockiter].type = grass;
            }
            if (globaly == 7 && globalz == 3 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = leaf;
            }
            if (globaly == 3 && (globalz-globaly+globalx) % Worldx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = woodlog;
            }
            if (globaly == 3 && globalz == 3 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = woodlog;
            }
            if (globaly == 4 && globalz == 3 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = woodlog;
            }
            if (globaly == 5 && globalz == 3 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = woodlog;
            }
            if (globaly == 6 && globalz == 3 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 4 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 2 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 3 && globalx == 2){
                nearbyChunks[chunkiter].blocks[blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 3 && globalx == 4){
                nearbyChunks[chunkiter].blocks[blockiter].type = leaf;
            }
            if (globaly == 7 && globalz == 3 && globalx == 3){
                nearbyChunks[chunkiter].blocks[blockiter].type = leaf;
            }
        }
        generatemesh(chunkiter, handles, nearbyChunks);
    }
}

static inline void genchunk(
    int whichChunkTOEdit,
    int renderdistance,
    int chunksize,
    int chunkcoordx,
    int chunkcoordy,
    int chunkcoordz,
    struct block *world[]
    ){
    assert(world[whichChunkTOEdit]);


    for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++) {
        world[whichChunkTOEdit][blockiter].type = air;
        // Calculate local block coordinates within the chunk
        int localx = blockiter % chunksize;
        int localy = (blockiter / chunksize) % chunksize;
        int localz = blockiter / (chunksize * chunksize);
        // Calculate global coordinates
        int globalx = chunkcoordx * chunksize + localx;
        int globaly = chunkcoordy * chunksize + localy;
        int globalz = chunkcoordz * chunksize + localz;
        if (globaly == 0){
            world[whichChunkTOEdit][blockiter].type = grass;
        }
        if (globaly == 7 && globalz == 3 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = leaf;
        }
        if (globaly == 3 && (globalz-globaly+globalx) % Worldx == 3){
            world[whichChunkTOEdit][blockiter].type = woodlog;
        }
        if (globaly == 3 && globalz == 3 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = woodlog;
        }
        if (globaly == 4 && globalz == 3 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = woodlog;
        }
        if (globaly == 5 && globalz == 3 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = woodlog;
        }
        if (globaly == 6 && globalz == 3 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = leaf;
        }
        if (globaly == 6 && globalz == 4 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = leaf;
        }
        if (globaly == 6 && globalz == 2 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = leaf;
        }
        if (globaly == 6 && globalz == 3 && globalx == 2){
            world[whichChunkTOEdit][blockiter].type = leaf;
        }
        if (globaly == 6 && globalz == 3 && globalx == 4){
            world[whichChunkTOEdit][blockiter].type = leaf;
        }
        if (globaly == 7 && globalz == 3 && globalx == 3){
            world[whichChunkTOEdit][blockiter].type = leaf;
        }
    
    }
}