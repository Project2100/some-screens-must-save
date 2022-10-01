// Macro defining the length of a cube's side, along with a shorthand for vertex maps
#define SIDE_UNIT 0.4f
#define SU SIDE_UNIT


// The data structure for a 3D RGBA vertex
typedef struct {
    float position[4];
    float colour[4];
} vertex;

// The data structure for a "layer": a layer models the vertices and triangles that occur on a common, orthogonal 2D plane
typedef struct {
    unsigned int vtxcount;
    unsigned int idxCount;
    unsigned int* indexmap;
} layer;

// The data structure for a "shape", a Menger sponge of a specific level
typedef struct {
    vertex* vertices;
    unsigned int* indices;
    unsigned int vertexSize;
    unsigned int indexSize;
    unsigned int indexCount;
    unsigned int vertexCount;
    layer** layers;
    unsigned int layerCount;
    void (*compileLayers)();
} shape;


extern shape* currentShape;


void buildShape();
