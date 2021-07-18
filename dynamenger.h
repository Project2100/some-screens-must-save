#define SIDE_UNIT 0.4f

// Shortened name for vertex maps
#define SU SIDE_UNIT


// The struct that models a 3D RGBA vertex, 
typedef struct {
    float x, y, z;
    float colour[4];
} vertex;


typedef struct {
    unsigned int vtxcount;
    unsigned int idxCount;
    unsigned int* indexmap;
} layer;



typedef struct {
    vertex* vertices;
    unsigned int* indices;
    unsigned int vertexSize;
    unsigned int indexSize;
    unsigned int indexCount;
    unsigned int vertexCount;
    layer** layers;
    unsigned int layerCount;
} shape;





extern shape mengerL0;
extern shape mengerL1;
extern shape mengerL2;
extern shape* currentShape;

void L1_completeLayers();
void L2_completeLayers();

void buildShape(shape* crShape);
