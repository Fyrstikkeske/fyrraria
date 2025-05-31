#pragma once

#include "utils.h"
#include <string.h>

// to add 
// unsigned int VAOs[renderdistance], VBOs[renderdistance], VBOsSize[renderdistance]; and some more

const int vertexSize = 3+2+1+1;

const int faceSize = vertexSize * 6;

const int blockSize = faceSize * 6;

//MAKE MESHESHEESA DSA
// it are over 
//TODO FIX THIS PIECE OF SHIT UP

//Fuxing this piece of shit sir
//10% booting up the niggachain
//20% GEORGE DROID IS HERE
//99% ok fr though
//99.9% current method sucks
static inline void generatemesh(
    int pointe,
    const uint64_t* handles,
    Chunk *nearbyChunks
    ){
        int meshableBlocks = 0;
        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++){
            enum blocktype block = nearbyChunks[pointe].blocks[blockiter].type;
            if (block == air){continue;}
            meshableBlocks += 1;
        }

        
        nearbyChunks[pointe].vertices = blockSize * meshableBlocks;
        if (meshableBlocks == 0){
            return;
        }

        float worldmeshes[blockSize * meshableBlocks];


        int offset = 0;
        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++){
            enum blocktype block = nearbyChunks[pointe].blocks[blockiter].type;
            if (block == air) continue;

            uint64_t textHandleToUse = handles[block];

            // Calculate local block coordinates within the chunk
            int localx = blockiter % chunksize;
            int localy = (blockiter / chunksize) % chunksize;
            int localz = blockiter / (chunksize * chunksize);

            // Calculate global coordinates
            int globalx = nearbyChunks[pointe].cord.x * chunksize + localx;
            int globaly = nearbyChunks[pointe].cord.y * chunksize + localy;
            int globalz = nearbyChunks[pointe].cord.z * chunksize + localz;

            uint32_t handleLo = (uint32_t)(textHandleToUse & 0xFFFFFFFF);
            uint32_t handleHi = (uint32_t)(textHandleToUse >> 32);

            float floatLo;
            float floatHi;
            memcpy(&floatLo, &handleLo, sizeof(uint32_t));
            memcpy(&floatHi, &handleHi, sizeof(uint32_t));

            float ypvertice[] = {
                globalx + 0.0f, globaly +  1.0f, globalz + 0.0f, 0.0f, 1.0f, floatLo, floatHi,
                globalx + 1.0f, globaly +  1.0f, globalz + 0.0f, 1.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f, globaly +  1.0f, globalz +  1.0f, 1.0f, 0.0f,floatLo,floatHi,
                globalx +1.0f, globaly +  1.0f, globalz +  1.0f, 1.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f, globaly +  1.0f, globalz +  1.0f, 0.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f, globaly +  1.0f, globalz + 0.0f, 0.0f, 1.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset] = ypvertice[localiter];
            }
            float ynvertice[] = {
               globalx + 0.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
               globalx +0.0f, globaly +0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx +0.0f, globaly +0.0f,  globalz +1.0f,  0.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize ] = ynvertice[localiter];
            }
            float xpvertice[] = {
                globalx +1.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
                globalx +1.0f,globaly +  1.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +1.0f,globaly + 0.0f,  globalz +1.0f,  0.0f, 0.0f,floatLo,floatHi,
                globalx +1.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize * 2] = xpvertice[localiter];
            }
            float xnvertice[] = {
                globalx +0.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +0.0f,globaly +  1.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
                globalx +0.0f,globaly + 0.0f, globalz +0.0f,  0.0f, 1.0f,floatLo,floatHi,
                globalx +0.0f,globaly +  1.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
                globalx +0.0f,globaly + 0.0f,  globalz +1.0f,  0.0f, 0.0f,floatLo,floatHi,

            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize * 3] = xnvertice[localiter];
            }


            float zpvertice[] = {
              globalx +  0.0f, globaly +0.0f, globalz + 1.0f,  1.0f, 0.0f,floatLo,floatHi,
              globalx +  1.0f, globaly + 1.0f,globalz + 1.0f,  0.0f, 1.0f,floatLo,floatHi,
              globalx +  1.0f, globaly +0.0f, globalz + 1.0f,  1.0f, 1.0f,floatLo,floatHi,
              globalx +  1.0f, globaly + 1.0f,globalz + 1.0f,  0.0f, 1.0f,floatLo,floatHi,
              globalx + 0.0f, globaly +0.0f,  globalz +1.0f,  1.0f, 0.0f,floatLo,floatHi,
              globalx + 0.0f,  globaly +1.0f, globalz + 1.0f,  0.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + faceSize * 4] = zpvertice[localiter];
            }
            float znvertice[] = {
               globalx + 0.0f,globaly + 0.0f, globalz +0.0f,  1.0f, 0.0f,floatLo,floatHi,
               globalx + 1.0f,globaly + 0.0f, globalz +0.0f,  1.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly +  1.0f,globalz + 0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx + 1.0f,globaly +  1.0f,globalz + 0.0f,  0.0f, 1.0f,floatLo,floatHi,
               globalx +0.0f, globaly + 1.0f, globalz +0.0f,  0.0f, 0.0f,floatLo,floatHi,
               globalx + 0.0f, globaly +0.0f, globalz +0.0f,  1.0f, 0.0f,floatLo,floatHi,
            };
            for (int localiter = 0; localiter < faceSize; localiter++){
                worldmeshes[localiter + offset + (faceSize * 5)] = znvertice[localiter];
            }
        
            offset += blockSize;
        }

        glBindVertexArray(nearbyChunks[pointe].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, nearbyChunks[pointe].VBO);

        
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * blockSize * meshableBlocks, worldmeshes, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);  

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3* sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 7 * sizeof(float), (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, 7 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);

    
}