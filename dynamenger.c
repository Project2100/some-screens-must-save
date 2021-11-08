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
    fprintf(instanceLog, "Prepared for index construction\nVertex count: %u\nIndex map size: %u elements\n", vtxcount, *indexMapCount);
#endif

    unsigned int idxcaret = 0;

    // For each of the three axes
    for (unsigned int axis = 0; axis < 3; axis++) {

        // Construct the idx permutation by sorting the mirroring array with the right comparator
        qsort_s(mirror, vtxcount, sizeof mirror[0], comparators[axis], vertices);

#ifdef DEBUG
        fprintf(instanceLog, "Axis %u permutation:\nLog.  ->Phys.\n", axis);
        for (unsigned int i = 0; i < vtxcount; i++) {
            fprintf(instanceLog, "%5u -> %5u\n", i, mirror[i]);
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

    if (size == 0) return NULL;

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
    unsigned int innerCount;
    unsigned int planeCount;

} replica;


replica base = {0};


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
        fprintf(instanceLog, "Vertex %5d: % f, % f, % f, % f", *caret + i, dest[*caret + i].position[0], dest[*caret + i].position[1], dest[*caret + i].position[2], dest[*caret + i].position[3]);
#endif
        applyTransform(dest[*caret + i].position, transform);
#ifdef DEBUG
        fprintf(instanceLog, " -> % f, % f, % f, % f\n", dest[*caret + i].position[0], dest[*caret + i].position[1], dest[*caret + i].position[2], dest[*caret + i].position[3]);
#endif
    }
    *caret += count;
}


void levelUp(replica* block) {

    unsigned int biginnerCount = block->innerCount * 20 + block->planeCount * 24 + 8;
    unsigned int bigplaneCount = block->planeCount * 8 + 4;

#ifdef DEBUG
    fprintf(instanceLog, "About to level up; expected vertex counts: 8 outer + 6 * %u side + %u inner = %u total\n", bigplaneCount, biginnerCount, 8 + 6 * bigplaneCount + biginnerCount);
    fprintf(instanceLog, "block addresses: %p %p %p %p %p %p %p\n", block->inner, block->planeXp, block->planeXn, block->planeYp, block->planeYn, block->planeZp, block->planeZn);
#endif

    // Guaranteed not to invoke malloc(0), as the sizeof coefficients have an additive, positive constant
    vertex* biginner = malloc(biginnerCount * sizeof *(biginner));
    vertex* bigplaneXp = malloc(bigplaneCount * sizeof *(bigplaneXp));
    vertex* bigplaneXn = malloc(bigplaneCount * sizeof *(bigplaneXn));
    vertex* bigplaneYp = malloc(bigplaneCount * sizeof *(bigplaneYp));
    vertex* bigplaneYn = malloc(bigplaneCount * sizeof *(bigplaneYn));
    vertex* bigplaneZp = malloc(bigplaneCount * sizeof *(bigplaneZp));
    vertex* bigplaneZn = malloc(bigplaneCount * sizeof *(bigplaneZn));

#ifdef DEBUG
    fprintf(instanceLog, "clone addresses: %p %p %p %p %p %p %p\n", biginner, bigplaneXp, bigplaneXn, bigplaneYp, bigplaneYn, bigplaneZp, bigplaneZn);
    
    fprintf(instanceLog, "buffers allocated\n");
#endif


    // INNER
    unsigned int innerCaret = 0;

    // The 8 grid vertices
    biginner[innerCaret++] = (vertex) {{ SU/3,  SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    biginner[innerCaret++] = (vertex) {{ SU/3, -SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    biginner[innerCaret++] = (vertex) {{-SU/3,  SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    biginner[innerCaret++] = (vertex) {{-SU/3, -SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    biginner[innerCaret++] = (vertex) {{ SU/3,  SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    biginner[innerCaret++] = (vertex) {{ SU/3, -SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    biginner[innerCaret++] = (vertex) {{-SU/3,  SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    biginner[innerCaret++] = (vertex) {{-SU/3, -SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    
    // The 20 blocks
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, pppTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, ppnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, pnpTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, pnnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, nppTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, npnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, nnpTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, nnnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, cppTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, cpnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, cnpTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, cnnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, pcpTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, pcnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, ncpTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, ncnTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, ppcTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, pncTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, npcTransform);
    cloneVertices(biginner, &innerCaret, block->inner, block->innerCount, nncTransform);
    
#ifdef DEBUG
    fprintf(instanceLog, "inner inners done\n");
#endif

    // Inner sides - X
    cloneVertices(biginner, &innerCaret, block->planeYn, block->planeCount, cppTransform);
    cloneVertices(biginner, &innerCaret, block->planeZn, block->planeCount, cppTransform);
    cloneVertices(biginner, &innerCaret, block->planeYn, block->planeCount, cpnTransform);
    cloneVertices(biginner, &innerCaret, block->planeZp, block->planeCount, cpnTransform);
    cloneVertices(biginner, &innerCaret, block->planeYp, block->planeCount, cnpTransform);
    cloneVertices(biginner, &innerCaret, block->planeZn, block->planeCount, cnpTransform);
    cloneVertices(biginner, &innerCaret, block->planeYp, block->planeCount, cnnTransform);
    cloneVertices(biginner, &innerCaret, block->planeZp, block->planeCount, cnnTransform);

    // Inner sides - Y
    cloneVertices(biginner, &innerCaret, block->planeXn, block->planeCount, pcpTransform);
    cloneVertices(biginner, &innerCaret, block->planeZn, block->planeCount, pcpTransform);
    cloneVertices(biginner, &innerCaret, block->planeXn, block->planeCount, pcnTransform);
    cloneVertices(biginner, &innerCaret, block->planeZp, block->planeCount, pcnTransform);
    cloneVertices(biginner, &innerCaret, block->planeXp, block->planeCount, ncpTransform);
    cloneVertices(biginner, &innerCaret, block->planeZn, block->planeCount, ncpTransform);
    cloneVertices(biginner, &innerCaret, block->planeXp, block->planeCount, ncnTransform);
    cloneVertices(biginner, &innerCaret, block->planeZp, block->planeCount, ncnTransform);

    // Inner sides - Z
    cloneVertices(biginner, &innerCaret, block->planeXn, block->planeCount, ppcTransform);
    cloneVertices(biginner, &innerCaret, block->planeYn, block->planeCount, ppcTransform);
    cloneVertices(biginner, &innerCaret, block->planeXn, block->planeCount, pncTransform);
    cloneVertices(biginner, &innerCaret, block->planeYp, block->planeCount, pncTransform);
    cloneVertices(biginner, &innerCaret, block->planeXp, block->planeCount, npcTransform);
    cloneVertices(biginner, &innerCaret, block->planeYn, block->planeCount, npcTransform);
    cloneVertices(biginner, &innerCaret, block->planeXp, block->planeCount, nncTransform);
    cloneVertices(biginner, &innerCaret, block->planeYp, block->planeCount, nncTransform);

#ifdef DEBUG
    fprintf(instanceLog, "inner done\n");
#endif

    // PLANES

#ifdef DEBUG
    fprintf(instanceLog, "Xp\n");
#endif
    // Xp
    innerCaret = 0;
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, pppTransform);
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, pcpTransform);
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, pnpTransform);
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, pncTransform);
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, pnnTransform);
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, pcnTransform);
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, ppnTransform);
    cloneVertices(bigplaneXp, &innerCaret, block->planeXp, block->planeCount, ppcTransform);
    bigplaneXp[innerCaret++] = (vertex) {{ SU,  SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneXp[innerCaret++] = (vertex) {{ SU,  SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneXp[innerCaret++] = (vertex) {{ SU, -SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneXp[innerCaret++] = (vertex) {{ SU, -SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

#ifdef DEBUG
    fprintf(instanceLog, "Xn\n");
#endif
    // Xn
    innerCaret = 0;
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, nppTransform);
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, ncpTransform);
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, nnpTransform);
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, nncTransform);
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, nnnTransform);
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, ncnTransform);
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, npnTransform);
    cloneVertices(bigplaneXn, &innerCaret, block->planeXn, block->planeCount, npcTransform);
    bigplaneXn[innerCaret++] = (vertex) {{-SU,  SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneXn[innerCaret++] = (vertex) {{-SU,  SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneXn[innerCaret++] = (vertex) {{-SU, -SU/3,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneXn[innerCaret++] = (vertex) {{-SU, -SU/3, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};


#ifdef DEBUG
    fprintf(instanceLog, "Yp\n");
#endif
    // Yp
    innerCaret = 0;
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, pppTransform);
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, cppTransform);
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, nppTransform);
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, npcTransform);
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, npnTransform);
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, cpnTransform);
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, ppnTransform);
    cloneVertices(bigplaneYp, &innerCaret, block->planeYp, block->planeCount, ppcTransform);
    bigplaneYp[innerCaret++] = (vertex) {{ SU/3,  SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneYp[innerCaret++] = (vertex) {{ SU/3,  SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneYp[innerCaret++] = (vertex) {{-SU/3,  SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneYp[innerCaret++] = (vertex) {{-SU/3,  SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

#ifdef DEBUG
    fprintf(instanceLog, "Yn\n");
#endif
    // Yn
    innerCaret = 0;
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, pnpTransform);
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, cnpTransform);
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, nnpTransform);
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, nncTransform);
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, nnnTransform);
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, cnnTransform);
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, pnnTransform);
    cloneVertices(bigplaneYn, &innerCaret, block->planeYn, block->planeCount, pncTransform);
    bigplaneYn[innerCaret++] = (vertex) {{ SU/3, -SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneYn[innerCaret++] = (vertex) {{ SU/3, -SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneYn[innerCaret++] = (vertex) {{-SU/3, -SU,  SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneYn[innerCaret++] = (vertex) {{-SU/3, -SU, -SU/3, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};


#ifdef DEBUG
    fprintf(instanceLog, "Zp\n");
#endif
    // Zp
    innerCaret = 0;
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, pppTransform);
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, cppTransform);
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, nppTransform);
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, ncpTransform);
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, nnpTransform);
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, cnpTransform);
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, pnpTransform);
    cloneVertices(bigplaneZp, &innerCaret, block->planeZp, block->planeCount, pcpTransform);
    bigplaneZp[innerCaret++] = (vertex) {{ SU/3,  SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneZp[innerCaret++] = (vertex) {{ SU/3, -SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneZp[innerCaret++] = (vertex) {{-SU/3,  SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneZp[innerCaret++] = (vertex) {{-SU/3, -SU/3,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

#ifdef DEBUG
    fprintf(instanceLog, "Zn\n");
#endif
    // Zn
    innerCaret = 0;
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, ppnTransform);
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, cpnTransform);
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, npnTransform);
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, ncnTransform);
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, nnnTransform);
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, cnnTransform);
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, pnnTransform);
    cloneVertices(bigplaneZn, &innerCaret, block->planeZn, block->planeCount, pcnTransform);
    bigplaneZn[innerCaret++] = (vertex) {{ SU/3,  SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneZn[innerCaret++] = (vertex) {{ SU/3, -SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneZn[innerCaret++] = (vertex) {{-SU/3,  SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};
    bigplaneZn[innerCaret++] = (vertex) {{-SU/3, -SU/3, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}};

    
#ifdef DEBUG
    fprintf(instanceLog, "sides done\n");
#endif

    free(block->inner);
    free(block->planeXp);
    free(block->planeXn);
    free(block->planeYp);
    free(block->planeYn);
    free(block->planeZp);
    free(block->planeZn);

    block->inner = biginner;
    block->planeXp = bigplaneXp;
    block->planeXn = bigplaneXn;
    block->planeYp = bigplaneYp;
    block->planeYn = bigplaneYn;
    block->planeZp = bigplaneZp;
    block->planeZn = bigplaneZn;

    block->innerCount = biginnerCount;
    block->planeCount = bigplaneCount;

}


#define SSMS_EPSILON 0.00001f
void adjust(vertex* vertices, unsigned int count, int level) {

    // Create the array of canonical values
    int divisor = 1;
    for (int i = 0; i < level; divisor *= 3, i++);

    float* canons = malloc((divisor + 1) * sizeof *canons);

    int dividend = divisor;
    for (int i = 0; i <= divisor; i++) {
        canons[i] = SU * ((float) dividend / divisor);
        dividend -= 2;
    }

#ifdef DEBUG
    fprintf(instanceLog, "Canonical values: \n");
    for (int i = 0; i <= divisor; i++) {
        fprintf(instanceLog, "% f\n", canons[i]);
    }
#endif

    // Adjust all the vertices
    for (unsigned int i = 0; i < count; i++) {
        for (unsigned int dim = 0; dim < 3; dim++) {
            for (int canon = 0; canon <= divisor; canon++) {
                if (vertices[i].position[dim] < canons[canon] + SSMS_EPSILON && vertices[i].position[dim] > canons[canon] - SSMS_EPSILON) {
                    vertices[i].position[dim] = canons[canon];
                }
            }
        }
    }

}


vertex* flatten(replica* sponge) {

    vertex* vtx = malloc((8 + sponge->innerCount + 6 * sponge->planeCount) * sizeof *(sponge->inner));

    memcpy(vtx                                                  , mengerL0_vtcs  , 8                  * sizeof *(sponge->inner));
    memcpy(vtx + 8                                              , sponge->inner  , sponge->innerCount * sizeof *(sponge->inner));
    memcpy(vtx + 8 + sponge->innerCount                         , sponge->planeXp, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + 8 + sponge->innerCount + sponge->planeCount    , sponge->planeXn, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + 8 + sponge->innerCount + sponge->planeCount * 2, sponge->planeYp, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + 8 + sponge->innerCount + sponge->planeCount * 3, sponge->planeYn, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + 8 + sponge->innerCount + sponge->planeCount * 4, sponge->planeZp, sponge->planeCount * sizeof *(sponge->inner));
    memcpy(vtx + 8 + sponge->innerCount + sponge->planeCount * 5, sponge->planeZn, sponge->planeCount * sizeof *(sponge->inner));

    free(sponge->inner);
    free(sponge->planeXp);
    free(sponge->planeXn);
    free(sponge->planeYp);
    free(sponge->planeYn);
    free(sponge->planeZp);
    free(sponge->planeZn);


    return vtx;
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
        break;

    case 2:
        currentShape = &mengerL2;
        break;

    case 3:
        currentShape = &mengerL3;
        break;
    }

    // Write down the vertices
    for (int i = 0; i < spongeLevel; levelUp(&base), i++);

    
#ifdef DEBUG
    fprintf(instanceLog, "Flattening\n");
#endif

    currentShape->vertices = flatten(&base);
    currentShape->vertexCount = base.innerCount + 8 + 6 * base.planeCount;
    currentShape->vertexSize = currentShape->vertexCount * sizeof *(currentShape->vertices);

#ifdef DEBUG
    fprintf(instanceLog, "Adjusting\n");
#endif

    adjust(currentShape->vertices, currentShape->vertexCount, spongeLevel);
    
#ifdef DEBUG
    fprintf(instanceLog, "Flipping templates\n");
#endif

    (*(currentShape->compileLayers))();

#ifdef DEBUG
    fprintf(instanceLog, "Building index map\n");
#endif


    buildIndexMap(currentShape->vertices, currentShape->vertexCount, currentShape->layers, currentShape->layerCount, &(currentShape->indices), &(currentShape->indexCount));

#ifdef DEBUG
    fprintf(instanceLog, "Map built, size: %u\n", currentShape->indexCount);

    for (unsigned int i = 0; i < currentShape->indexCount; i++){
        fprintf(instanceLog, "%u\n", currentShape->indices[i]);
    }
#endif

    currentShape->indexSize = (currentShape->indexCount) * sizeof *(currentShape->indices);

#ifdef DEBUG
    fprintf(instanceLog, "Shape: vtxbytes %u, idxbytes %u, idxcount %u\n", mengerL0.vertexSize, mengerL0.indexSize, mengerL0.indexCount);
#endif
}


