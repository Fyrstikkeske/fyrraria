#pragma once

#include "utils.h"
#include <string.h>
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
//99.9% GEORGE DROID method sucks
static inline void generate_mesh_for_chunk(
    Chunk *chunk,
    const uint64_t* handles
    ){
        int meshableBlocks = 0;
        for (int blockiter = 0; blockiter < CHUNK_VOLUME; blockiter++){
            enum blocktype block = chunk->blocks[blockiter].type;
            if (block == AIR){continue;}
            meshableBlocks += 1;
        }

        
        chunk->vertices = blockSize * meshableBlocks;
        if (meshableBlocks == 0){
            return;
        }

        float worldmeshes[blockSize * meshableBlocks];


        int offset = 0;
        for (int blockiter = 0; blockiter < CHUNK_VOLUME; blockiter++){
            enum blocktype block = chunk->blocks[blockiter].type;
            if (block == AIR) continue;

            uint64_t textHandleToUse = handles[block];

            // Calculate local block coordinates within the chunk
            int localx = blockiter % CHUNK_SIZE;
            int localy = (blockiter / CHUNK_SIZE) % CHUNK_SIZE;
            int localz = blockiter / (CHUNK_SIZE * CHUNK_SIZE);

            // Calculate global coordinates
            int globalx = chunk->Key.x * CHUNK_SIZE + localx;
            int globaly = chunk->Key.y * CHUNK_SIZE + localy;
            int globalz = chunk->Key.z * CHUNK_SIZE + localz;

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

        glBindVertexArray(chunk->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, chunk->VBO);

        
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