#include "dynamenger.h"
#include <windows.h>

#include "debug.h"




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////////INDICES//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////





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
    
    if (((vertex*) context)[*(unsigned int*) a].position[0] > ((vertex*) context)[*(unsigned int*) b].position[0]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[0] < ((vertex*) context)[*(unsigned int*) b].position[0]) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].position[2] > ((vertex*) context)[*(unsigned int*) b].position[2]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[2] < ((vertex*) context)[*(unsigned int*) b].position[2]) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].position[1] > ((vertex*) context)[*(unsigned int*) b].position[1]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[1] < ((vertex*) context)[*(unsigned int*) b].position[1]) return -1;

    return 0;
}

int ypovCompare (void* context, const void* a, const void* b) {
    
    if (((vertex*) context)[*(unsigned int*) a].position[1] > ((vertex*) context)[*(unsigned int*) b].position[1]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[1] < ((vertex*) context)[*(unsigned int*) b].position[1]) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].position[0] > ((vertex*) context)[*(unsigned int*) b].position[0]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[0] < ((vertex*) context)[*(unsigned int*) b].position[0]) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].position[2] > ((vertex*) context)[*(unsigned int*) b].position[2]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[2] < ((vertex*) context)[*(unsigned int*) b].position[2]) return -1;

    return 0;
}

int zpovCompare (void* context, const void* a, const void* b) {
    
    if (((vertex*) context)[*(unsigned int*) a].position[2] > ((vertex*) context)[*(unsigned int*) b].position[2]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[2] < ((vertex*) context)[*(unsigned int*) b].position[2]) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].position[1] > ((vertex*) context)[*(unsigned int*) b].position[1]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[1] < ((vertex*) context)[*(unsigned int*) b].position[1]) return -1;
    
    if (((vertex*) context)[*(unsigned int*) a].position[0] > ((vertex*) context)[*(unsigned int*) b].position[0]) return 1;
    if (((vertex*) context)[*(unsigned int*) a].position[0] < ((vertex*) context)[*(unsigned int*) b].position[0]) return -1;

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


unsigned int* flipLayer(unsigned int* source, unsigned int size) {
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
















#include "mengerL0.h"
#include "mengerL1.h"
#include "mengerL2.h"
#include "mengerL3.h"




////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////////VERTICES/////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


typedef struct {
    vertex* inner;
    vertex* planeXp;
    vertex* planeXn;
    vertex* planeYp;
    vertex* planeYn;
    vertex* planeZp;
    vertex* planeZn;
    vertex* outer;
    unsigned int innerCount;
    unsigned int planeCount;
    unsigned int outerCount;

} replica;


replica base = {
    .outer = mengerL0_vtcs,
    .outerCount = 8,
};


float pppTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU,  2.f/3 * SU,  2.f/3 * SU, 1},
};
float ppnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU,  2.f/3 * SU, -2.f/3 * SU, 1},
};
float pnpTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU, -2.f/3 * SU,  2.f/3 * SU, 1},
};
float pnnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU, -2.f/3 * SU, -2.f/3 * SU, 1},
};

float nppTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU,  2.f/3 * SU,  2.f/3 * SU, 1},
};
float npnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU,  2.f/3 * SU, -2.f/3 * SU, 1},
};
float nnpTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU, -2.f/3 * SU,  2.f/3 * SU, 1},
};
float nnnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU, -2.f/3 * SU, -2.f/3 * SU, 1},
};



float cppTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {          0,  2.f/3 * SU,  2.f/3 * SU, 1},
};
float cpnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {          0,  2.f/3 * SU, -2.f/3 * SU, 1},
};
float cnpTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {          0, -2.f/3 * SU,  2.f/3 * SU, 1},
};
float cnnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {          0, -2.f/3 * SU, -2.f/3 * SU, 1},
};

float pcpTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU,           0,  2.f/3 * SU, 1},
};
float pcnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU,           0, -2.f/3 * SU, 1},
};
float ncpTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU,           0,  2.f/3 * SU, 1},
};
float ncnTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU,           0, -2.f/3 * SU, 1},
};

float ppcTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU,  2.f/3 * SU,           0, 1},
};
float pncTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    { 2.f/3 * SU, -2.f/3 * SU,           0, 1},
};
float npcTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU,  2.f/3 * SU,           0, 1},
};
float nncTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {-2.f/3 * SU, -2.f/3 * SU,           0, 1},
};


// For the 8 grid vertices that must be added to inner
float cccTransform[4][4] = {
    {1.f/3, 0, 0, 0},
    {0, 1.f/3, 0, 0},
    {0, 0, 1.f/3, 0},
    {0, 0, 0, 1},
};




void applyTransform(float* vector, float transform[4][4]) {

    float x = vector[0] * transform[0][0] + vector[1] * transform[1][0] + vector[2] * transform[2][0] + vector[3] * transform[3][0]; 
    float y = vector[0] * transform[0][1] + vector[1] * transform[1][1] + vector[2] * transform[2][1] + vector[3] * transform[3][1]; 
    float z = vector[0] * transform[0][2] + vector[1] * transform[1][2] + vector[2] * transform[2][2] + vector[3] * transform[3][2]; 
    float w = vector[0] * transform[0][3] + vector[1] * transform[1][3] + vector[2] * transform[2][3] + vector[3] * transform[3][3];

    vector[0] = x;
    vector[1] = y;
    vector[2] = z;
    vector[3] = w;
}


void cloneVertices(vertex* dest, unsigned int* caret, vertex* plane, const unsigned int count, float transform[4][4]) {

#ifdef DEBUG
    fprintf(instanceLog, "Cloning %u vertices on caret %u; source %p, dest %p\n", count, *caret, plane, dest);
#endif
    memcpy(dest + *caret, plane, count * sizeof *plane);
    for (unsigned int i = 0; i < count; i++) {
#ifdef DEBUG
        fprintf(instanceLog, "input: %f, %f, %f, %f\n", dest[*caret + i].position[0], dest[*caret + i].position[1], dest[*caret + i].position[2], dest[*caret + i].position[3]);
#endif
        applyTransform(dest[*caret + i].position, transform);
#ifdef DEBUG
        fprintf(instanceLog, "trans: %f, %f, %f, %f\n", dest[*caret + i].position[0], dest[*caret + i].position[1], dest[*caret + i].position[2], dest[*caret + i].position[3]);
#endif
    }
    *caret += count;
}


replica* levelUp(replica* block) {

    replica* big = malloc(sizeof *big);

    big->innerCount = block->innerCount * 20 + block->planeCount * 24 + 8;
    big->planeCount = block->planeCount * 8 + 4;
    big->outerCount = 8;

    
#ifdef DEBUG
    fprintf(instanceLog, "About to level up; expected vertex counts: %u outer + 6 * %u side + %u inner = %u total\n", big->outerCount, big->planeCount, big->innerCount, big->outerCount + 6 * big->planeCount + big->innerCount);
    fprintf(instanceLog, "block addresses: %p %p %p %p %p %p %p %p\n", block->inner, block->planeXp, block->planeXn, block->planeYp, block->planeYn, block->planeZp, block->planeZn, block->outer);
#endif

    // Guaranteed not to invoke malloc(0), as the sizeof coefficients have an additive, positive constant
    big->inner = malloc(big->innerCount * sizeof *(big->inner));
    big->planeXp = malloc(big->planeCount * sizeof *(big->planeXp));
    big->planeXn = malloc(big->planeCount * sizeof *(big->planeXn));
    big->planeYp = malloc(big->planeCount * sizeof *(big->planeYp));
    big->planeYn = malloc(big->planeCount * sizeof *(big->planeYn));
    big->planeZp = malloc(big->planeCount * sizeof *(big->planeZp));
    big->planeZn = malloc(big->planeCount * sizeof *(big->planeZn));
    big->outer = malloc(big->outerCount * sizeof *(big->outer));

#ifdef DEBUG
    fprintf(instanceLog, "clone addresses: %p %p %p %p %p %p %p %p\n", big->inner, big->planeXp, big->planeXn, big->planeYp, big->planeYn, big->planeZp, big->planeZn, big->outer);
    
    fprintf(instanceLog, "buffers allocated\n");
#endif

    // OUTER
    // Copy outmost vertices, they will be always present
    memcpy(big->outer, block->outer, block->outerCount * sizeof *(block->outer));



    // INNER
    unsigned int innerCaret = 0;

    // The 8 grid vertices
    memcpy(big->inner, block->outer, block->outerCount * sizeof *(block->outer));
    for (0 ; innerCaret < block->outerCount; innerCaret++) {
        applyTransform(big->inner[innerCaret].position, cccTransform);
    }
    


    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, pppTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, ppnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, pnpTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, pnnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, nppTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, npnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, nnpTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, nnnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, cppTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, cpnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, cnpTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, cnnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, pcpTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, pcnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, ncpTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, ncnTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, ppcTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, pncTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, npcTransform);
    cloneVertices(big->inner, &innerCaret, block->inner, block->innerCount, nncTransform);
    
#ifdef DEBUG
    fprintf(instanceLog, "inner inners done\n");
#endif

    // Inner sides - X
    cloneVertices(big->inner, &innerCaret, block->planeYn, block->planeCount, cppTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZn, block->planeCount, cppTransform);
    cloneVertices(big->inner, &innerCaret, block->planeYn, block->planeCount, cpnTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZp, block->planeCount, cpnTransform);
    cloneVertices(big->inner, &innerCaret, block->planeYp, block->planeCount, cnpTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZn, block->planeCount, cnpTransform);
    cloneVertices(big->inner, &innerCaret, block->planeYp, block->planeCount, cnnTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZp, block->planeCount, cnnTransform);

    // Inner sides - Y
    cloneVertices(big->inner, &innerCaret, block->planeXn, block->planeCount, pcpTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZn, block->planeCount, pcpTransform);
    cloneVertices(big->inner, &innerCaret, block->planeXn, block->planeCount, pcnTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZp, block->planeCount, pcnTransform);
    cloneVertices(big->inner, &innerCaret, block->planeXp, block->planeCount, ncpTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZn, block->planeCount, ncpTransform);
    cloneVertices(big->inner, &innerCaret, block->planeXp, block->planeCount, ncnTransform);
    cloneVertices(big->inner, &innerCaret, block->planeZp, block->planeCount, ncnTransform);

    // Inner sides - Z
    cloneVertices(big->inner, &innerCaret, block->planeXn, block->planeCount, ppcTransform);
    cloneVertices(big->inner, &innerCaret, block->planeYn, block->planeCount, ppcTransform);
    cloneVertices(big->inner, &innerCaret, block->planeXn, block->planeCount, pncTransform);
    cloneVertices(big->inner, &innerCaret, block->planeYp, block->planeCount, pncTransform);
    cloneVertices(big->inner, &innerCaret, block->planeXp, block->planeCount, npcTransform);
    cloneVertices(big->inner, &innerCaret, block->planeYn, block->planeCount, npcTransform);
    cloneVertices(big->inner, &innerCaret, block->planeXp, block->planeCount, nncTransform);
    cloneVertices(big->inner, &innerCaret, block->planeYp, block->planeCount, nncTransform);

#ifdef DEBUG
    fprintf(instanceLog, "inner done\n");
#endif

    // PLANES

#ifdef DEBUG
    fprintf(instanceLog, "Xp\n");
#endif
    // Xp
    innerCaret = 0;
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, pppTransform);
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, pcpTransform);
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, pnpTransform);
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, pncTransform);
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, pnnTransform);
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, pcnTransform);
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, ppnTransform);
    cloneVertices(big->planeXp, &innerCaret, block->planeXp, block->planeCount, ppcTransform);
    big->planeXp[innerCaret + 0] = (vertex) {{ SU,  SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeXp[innerCaret + 1] = (vertex) {{ SU,  SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeXp[innerCaret + 2] = (vertex) {{ SU, -SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeXp[innerCaret + 3] = (vertex) {{ SU, -SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

#ifdef DEBUG
    fprintf(instanceLog, "Xn\n");
#endif
    // Xn
    innerCaret = 0;
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, nppTransform);
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, ncpTransform);
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, nnpTransform);
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, nncTransform);
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, nnnTransform);
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, ncnTransform);
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, npnTransform);
    cloneVertices(big->planeXn, &innerCaret, block->planeXn, block->planeCount, npcTransform);
    big->planeXn[innerCaret + 0] = (vertex) {{-SU,  SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeXn[innerCaret + 1] = (vertex) {{-SU,  SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeXn[innerCaret + 2] = (vertex) {{-SU, -SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeXn[innerCaret + 3] = (vertex) {{-SU, -SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};


#ifdef DEBUG
    fprintf(instanceLog, "Yp\n");
#endif
    // Yp
    innerCaret = 0;
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, pppTransform);
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, cppTransform);
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, nppTransform);
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, npcTransform);
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, npnTransform);
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, cpnTransform);
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, ppnTransform);
    cloneVertices(big->planeYp, &innerCaret, block->planeYp, block->planeCount, ppcTransform);
    big->planeYp[innerCaret + 0] = (vertex) {{ SU/3,  SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeYp[innerCaret + 1] = (vertex) {{ SU/3,  SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeYp[innerCaret + 2] = (vertex) {{-SU/3,  SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeYp[innerCaret + 3] = (vertex) {{-SU/3,  SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

#ifdef DEBUG
    fprintf(instanceLog, "Yn\n");
#endif
    // Yn
    innerCaret = 0;
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, pnpTransform);
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, cnpTransform);
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, nnpTransform);
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, nncTransform);
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, nnnTransform);
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, cnnTransform);
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, pnnTransform);
    cloneVertices(big->planeYn, &innerCaret, block->planeYn, block->planeCount, pncTransform);
    big->planeYn[innerCaret + 0] = (vertex) {{ SU/3, -SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeYn[innerCaret + 1] = (vertex) {{ SU/3, -SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeYn[innerCaret + 2] = (vertex) {{-SU/3, -SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeYn[innerCaret + 3] = (vertex) {{-SU/3, -SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};


#ifdef DEBUG
    fprintf(instanceLog, "Zp\n");
#endif
    // Zp
    innerCaret = 0;
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, pppTransform);
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, cppTransform);
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, nppTransform);
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, ncpTransform);
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, nnpTransform);
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, cnpTransform);
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, pnpTransform);
    cloneVertices(big->planeZp, &innerCaret, block->planeZp, block->planeCount, pcpTransform);
    big->planeZp[innerCaret + 0] = (vertex) {{ SU/3,  SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeZp[innerCaret + 1] = (vertex) {{ SU/3, -SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeZp[innerCaret + 2] = (vertex) {{-SU/3,  SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeZp[innerCaret + 3] = (vertex) {{-SU/3, -SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

#ifdef DEBUG
    fprintf(instanceLog, "Zn\n");
#endif
    // Zn
    innerCaret = 0;
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, ppnTransform);
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, cpnTransform);
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, npnTransform);
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, ncnTransform);
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, nnnTransform);
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, cnnTransform);
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, pnnTransform);
    cloneVertices(big->planeZn, &innerCaret, block->planeZn, block->planeCount, pcnTransform);
    big->planeZn[innerCaret + 0] = (vertex) {{ SU/3,  SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeZn[innerCaret + 1] = (vertex) {{ SU/3, -SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeZn[innerCaret + 2] = (vertex) {{-SU/3,  SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    big->planeZn[innerCaret + 3] = (vertex) {{-SU/3, -SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

    
#ifdef DEBUG
    fprintf(instanceLog, "sides done\n");
#endif

    return big;
}


float L2canonicValues[] = {SU, SU * 7/9, SU * 5/9, SU * 3/9, SU * 1/9, -SU * 1/9, -SU * 3/9, -SU * 5/9, -SU * 7/9, -SU};
float epsilon = 0.0001f;

void adjust(vertex* vertices, unsigned int count) {
    for (unsigned int i = 0; i < count; i++) {
        for (unsigned int dim = 0; dim < 3; dim++) {
            for (int canon = 0; canon < 10; canon++) {
                if (vertices[i].position[dim] < L2canonicValues[canon] + epsilon && vertices[i].position[dim] > L2canonicValues[canon] - epsilon) {
                    vertices[i].position[dim] = L2canonicValues[canon];
                }
            }
        }
    }
}


vertex* flatten(replica* sponge) {

    vertex* vtx = malloc((sponge->innerCount + sponge->outerCount + 6 * sponge->planeCount) * sizeof *(sponge->inner));

    memcpy(vtx                                                                   , sponge->inner  , sponge->innerCount * sizeof *(sponge->inner));
    memcpy(vtx + sponge->innerCount                                              , sponge->outer  , sponge->outerCount * sizeof *(sponge->inner));
    memcpy(vtx + sponge->innerCount + sponge->outerCount                         , sponge->planeXp, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + sponge->innerCount + sponge->outerCount + sponge->planeCount    , sponge->planeXn, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + sponge->innerCount + sponge->outerCount + sponge->planeCount * 2, sponge->planeYp, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + sponge->innerCount + sponge->outerCount + sponge->planeCount * 3, sponge->planeYn, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + sponge->innerCount + sponge->outerCount + sponge->planeCount * 4, sponge->planeZp, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + sponge->innerCount + sponge->outerCount + sponge->planeCount * 5, sponge->planeZn, sponge->planeCount * sizeof *(sponge->inner));

    return vtx;
}



unsigned int buildL1vtcs(vertex** flatBuffer) {


#ifdef DEBUG
    fprintf(instanceLog, "Leveling up\n");
#endif

    replica* L1 = levelUp(&base);


#ifdef DEBUG
    fprintf(instanceLog, "Flattening\n");
#endif

    *flatBuffer = flatten(L1);

#ifdef DEBUG
    fprintf(instanceLog, "Returning\n");
#endif

    return L1->innerCount + L1->outerCount + 6 * L1->planeCount;
}



unsigned int buildL2vtcs(vertex** flatBuffer) {

#ifdef DEBUG
    fprintf(instanceLog, "Leveling up\n");
#endif

    replica* L1 = levelUp(&base);
    
#ifdef DEBUG
    fprintf(instanceLog, "Leveling up\n");
#endif

    replica* L2 = levelUp(L1);

#ifdef DEBUG
    fprintf(instanceLog, "Flattening\n");
#endif

    *flatBuffer = flatten(L2);
    

#ifdef DEBUG
    fprintf(instanceLog, "Adjusting\n");
#endif

    adjust(*flatBuffer, L2->innerCount + L2->outerCount + 6 * L2->planeCount);

#ifdef DEBUG
    fprintf(instanceLog, "Returning\n");
#endif

    return L2->innerCount + L2->outerCount + 6 * L2->planeCount;
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////////////BUILDER//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


void buildShape(int spongeLevel) {

    switch (spongeLevel) {
        case 0:
            currentShape = &mengerL0;
            break;
        case 1:
            currentShape = &mengerL1;

            currentShape->vertexCount = buildL1vtcs(&(currentShape->vertices));
            currentShape->vertexSize = currentShape->vertexCount * sizeof *(currentShape->vertices);

            break;
        case 2:
            currentShape = &mengerL2;

            currentShape->vertexCount = buildL2vtcs(&(currentShape->vertices));
            currentShape->vertexSize = currentShape->vertexCount * sizeof *(currentShape->vertices);

            break;
        }

    (*(currentShape->compileLayers))();

#ifdef DEBUG
    fprintf(instanceLog, "Building index map\n");
#endif

    unsigned int* idxmap;
    unsigned int mapcount;

    buildIndexMap(currentShape->vertices, currentShape->vertexCount, currentShape->layers, currentShape->layerCount, &idxmap, &mapcount);

#ifdef DEBUG
    fprintf(instanceLog, "Map built, size: %u\n", mapcount);

    for (unsigned int i = 0; i < mapcount; i++){
        fprintf(instanceLog, "%u\n", idxmap[i]);
    }
#endif

    currentShape->indices = idxmap;
    currentShape->indexSize = (mapcount) * sizeof *idxmap;
    currentShape->indexCount = mapcount;

#ifdef DEBUG
    fprintf(instanceLog, "Shape: vtxbytes %u, idxbytes %u, idxcount %u\n", mengerL0.vertexSize, mengerL0.indexSize, mengerL0.indexCount);
#endif
}


