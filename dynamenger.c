#include "dynamenger.h"
#include <windows.h>

// D3D11 GEOMETRY REMINDERS:
//
// DEFAULT POV: "ZPOV"
// X axis: left to right
// Y axis down to up
// Z axis: back to forth
//
// Left hand rule: x-middle, y-thumb, z-index
//
// triangle front face has clockwise vertices
//
// 0: X
// 1: Y
// 2: Z


shape* currentShape;


// The sort comparators, one for each axis. Currently sorting from negative to positive
int xpovCompare (void* context, const void* a, const void* b) {
    
    if (((vertex*) context)[*(unsigned int*) a].x > ((vertex*) context)[*(unsigned int*) b].x) return 1;
    if (((vertex*) context)[*(unsigned int*) a].x < ((vertex*) context)[*(unsigned int*) b].x) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].z > ((vertex*) context)[*(unsigned int*) b].z) return 1;
    if (((vertex*) context)[*(unsigned int*) a].z < ((vertex*) context)[*(unsigned int*) b].z) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].y > ((vertex*) context)[*(unsigned int*) b].y) return 1;
    if (((vertex*) context)[*(unsigned int*) a].y < ((vertex*) context)[*(unsigned int*) b].y) return -1;

    return 0;
}

int ypovCompare (void* context, const void* a, const void* b) {
    
    if (((vertex*) context)[*(unsigned int*) a].y > ((vertex*) context)[*(unsigned int*) b].y) return 1;
    if (((vertex*) context)[*(unsigned int*) a].y < ((vertex*) context)[*(unsigned int*) b].y) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].x > ((vertex*) context)[*(unsigned int*) b].x) return 1;
    if (((vertex*) context)[*(unsigned int*) a].x < ((vertex*) context)[*(unsigned int*) b].x) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].z > ((vertex*) context)[*(unsigned int*) b].z) return 1;
    if (((vertex*) context)[*(unsigned int*) a].z < ((vertex*) context)[*(unsigned int*) b].z) return -1;

    return 0;
}

int zpovCompare (void* context, const void* a, const void* b) {
    
    if (((vertex*) context)[*(unsigned int*) a].z > ((vertex*) context)[*(unsigned int*) b].z) return 1;
    if (((vertex*) context)[*(unsigned int*) a].z < ((vertex*) context)[*(unsigned int*) b].z) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].y > ((vertex*) context)[*(unsigned int*) b].y) return 1;
    if (((vertex*) context)[*(unsigned int*) a].y < ((vertex*) context)[*(unsigned int*) b].y) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].x > ((vertex*) context)[*(unsigned int*) b].x) return 1;
    if (((vertex*) context)[*(unsigned int*) a].x < ((vertex*) context)[*(unsigned int*) b].x) return -1;

    return 0;
}

int (*comparators[3])(void*, const void*, const void*) = {xpovCompare, ypovCompare, zpovCompare};




// POV is through the selected axis, facing forward
// Vertices are sorted in arabic reading order (right-to-left, top-to-bottom)
// Get the vertices on the current layer

// Rationale: get all the vertices on current layer, unordered. Then, use qsort with the right comparator

// Exclude vertices out of the current layer

// "Left hand rule"
// ZPOV: positive Y is up, positive X is right

// CAUTION: layers is an array of pointers, fullindexmap is a pointer to an array!
//
// NOTE: is it possible to enforce const on vertices without warnings on qsort?
void buildIndexMap(vertex* vertices, unsigned int vtxcount, layer** layers, unsigned int layerCount, unsigned int** fullIndexMap, unsigned int* indexMapCount) {

    // Compute the index map total size: sum all the layer counts, then multiply by the three axes. Then, allocate the index map
    *indexMapCount = 0;
    for (unsigned int layerIdx = 0; layerIdx < layerCount; layerIdx++) {
        *indexMapCount += layers[layerIdx]->idxCount;
    }
    *indexMapCount *= 3;
    *fullIndexMap = calloc(*indexMapCount, sizeof **fullIndexMap);
    
    // Create an array that permutes the vertex order in memory with a sorted order by point of view
    unsigned int* mirror = malloc(vtxcount * sizeof *mirror);
    for (unsigned int i = 0; i < vtxcount; i++) {
        mirror[i] = i;
    }

#ifdef DEBUG
    fprintf(instanceLog, "Prepared for index construction\nVertex count: %zu\nIndex map size: %u elements\nMirror array:\n", vtxcount, *indexMapCount);

    for (unsigned int i = 0; i < vtxcount; i++) {
        fprintf(instanceLog, "%u\n", mirror[i]);
    }
#endif

    unsigned int idxcaret = 0;

    // For each of the three axes
    for (unsigned int axis = 0; axis < 3; axis++) {

        // Construct the idx permutation by sorting the mirroring array with the right comparator
        qsort_s(mirror, vtxcount, sizeof mirror[0], comparators[axis], vertices);

#ifdef DEBUG
        fprintf(instanceLog, "permutation sorted:\n");
        for (unsigned int i = 0; i < vtxcount; i++) {
            fprintf(instanceLog, "%u\n", mirror[i]);
        }
#endif

        unsigned int vtxcaret = 0;

        // POV permutation is ready; for each layer...
        for (unsigned int layerIdx = 0; layerIdx < layerCount; layerIdx++) {

#ifdef DEBUG
            fprintf(instanceLog, "Building layer %u\n", layerIdx);
#endif
            // ... and for each index in the layer template...
            for (unsigned int i = 0; i < layers[layerIdx]->idxCount; i++, idxcaret++) {

#ifdef DEBUG
                fprintf(instanceLog, "Writing index %u, should be mapped in position %u + layers[%u]->indexmap[%u]\n", idxcaret, vtxcaret, layerIdx, i);
                fprintf(instanceLog, "Index in template: %u\n", layers[layerIdx]->indexmap[i]);
#endif
                // ....apply the magic
                (*fullIndexMap)[idxcaret] = mirror[vtxcaret + layers[layerIdx]->indexmap[i]];

#ifdef DEBUG
                fprintf(instanceLog, "Index %u written, result: %u\n", idxcaret, (*fullIndexMap)[idxcaret]);
#endif
            }

            vtxcaret += layers[layerIdx]->vtxcount;

        }
        
    }

    free(mirror);
}


unsigned int* flipClone(unsigned int* source, unsigned int size) {
    unsigned int* clone = malloc(size * sizeof *clone);

    for (unsigned int i = 0; i < size; i++) {
        switch (i % 3) {
            case 0:
            clone[i] = source[i];
            break;
            case 1:
            clone[i+1] = source[i];
            break;
            case 2:
            clone[i-1] = source[i];
            break;
        }
    }

    return clone;
}














void buildShape(shape* crShape) {

#ifdef DEBUG
    fprintf(instanceLog, "Building L0\n");
#endif

    unsigned int* idxmap;
    unsigned int mapcount;

    buildIndexMap(crShape->vertices, crShape->vertexCount, crShape->layers, crShape->layerCount, &idxmap, &mapcount);

#ifdef DEBUG
    fprintf(instanceLog, "L0 index built, size: %u\n", mapcount);

    for (unsigned int i = 0; i < mapcount; i++){
        fprintf(instanceLog, "%u\n", idxmap[i]);
    }
#endif

    crShape->indices = idxmap;
    crShape->indexSize = (mapcount) * sizeof *idxmap;
    crShape->indexCount = mapcount;

#ifdef DEBUG
    fprintf(instanceLog, "L0 shape: vtxbytes %u, idxbytes %u, idxcount %u\n", mengerL0.vertexSize, mengerL0.indexSize, mengerL0.indexCount);
#endif
}







#include "mengerL0.h"
#include "mengerL1.h"
#include "mengerL2.h"




