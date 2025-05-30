#pragma once

#include "cglm/types.h"
#include "utils.h"

#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>



static inline void setChunkVectors(
    int Worldx,
    int Worldy,
    int Worldz,
    Chunk *nearbyChunks,
    int len,
    int renderdistance,
    vec3 position
){
    for (int chunkiter = 0; chunkiter < len; chunkiter++) {
        int localx = chunkiter % renderdistance;
        int localy = (chunkiter / renderdistance) % renderdistance;
        int localz = chunkiter / (renderdistance * renderdistance);

        nearbyChunks[chunkiter].cord[0] = (localx - (int)(renderdistance/2)) % Worldx;
        nearbyChunks[chunkiter].cord[1] = (localy - (int)(renderdistance/2)) % Worldy;
        nearbyChunks[chunkiter].cord[2] = (localz - (int)(renderdistance/2)) % Worldz;
    }
}


static inline void genNearbyChunks(
    int Worldx,
    int Worldy,
    int Worldz,
    Chunk *nearbyChunks,
    int len
    ){
    for (int chunkiter = 0; chunkiter < len; chunkiter++) {
        if (nearbyChunks[chunkiter].isdirty == False){continue;}

        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++) {
            nearbyChunks[chunkiter].blocks[blockiter].type = air;

            int localx = blockiter % chunksize;
            int localy = (blockiter / chunksize) % chunksize;
            int localz = blockiter / (chunksize * chunksize);

            int globalx = nearbyChunks[chunkiter].cord[0] * chunksize + localx;
            int globaly = nearbyChunks[chunkiter].cord[1] * chunksize + localy;
            int globalz = nearbyChunks[chunkiter].cord[2] * chunksize + localz;


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