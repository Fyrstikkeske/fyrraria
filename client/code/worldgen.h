#ifndef WORLD_GEN_H
#define WORLD_GEN_H

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
static inline void genworld(
    int Worldx,
    int Worldy,
    int Worldz,
    int chunksize,
    struct block *world[Worldx*Worldy*Worldz]
    ){
    for (int chunkiter = 0; chunkiter < Worldx * Worldy * Worldz; chunkiter++) {
        world[chunkiter] = malloc(sizeof(struct block) * chunksize * chunksize * chunksize);
        if (!world[chunkiter]) {
            fprintf(stderr, "Memory allocation failed!\n");
            exit(EXIT_FAILURE);
        }

        for (int blockiter = 0; blockiter < chunksize * chunksize * chunksize; blockiter++) {
            world[chunkiter][blockiter].type = air;

            // Calculate chunk coordinates
            int chunkx = chunkiter % Worldx;
            int chunky = (chunkiter / Worldx) % Worldy;
            int chunkz = chunkiter / (Worldx * Worldy);

            // Calculate local block coordinates within the chunk
            int localx = blockiter % chunksize;
            int localy = (blockiter / chunksize) % chunksize;
            int localz = blockiter / (chunksize * chunksize);

            // Calculate global coordinates
            int globalx = chunkx * chunksize + localx;
            int globaly = chunky * chunksize + localy;
            int globalz = chunkz * chunksize + localz;


            if (globaly == 0){
                world[chunkiter][blockiter].type = grass;
            }
            if (globaly == 7 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 3 && (globalz-globaly+globalx) % Worldx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 3 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 4 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 5 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = woodlog;
            }
            if (globaly == 6 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 4 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 2 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 3 && globalx == 2){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 6 && globalz == 3 && globalx == 4){
                world[chunkiter][blockiter].type = leaf;
            }
            if (globaly == 7 && globalz == 3 && globalx == 3){
                world[chunkiter][blockiter].type = leaf;
            }
        }
    }
}
#endif