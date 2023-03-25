// Macro defining the length of a cube's side
#define SIDE_UNIT 0.4f


// The data structure for a 3D RGBA vertex
typedef struct {
    float position[4];
    float colour[4];
} vertex;

// The data structure for a "shape", a Menger sponge of a specific level
typedef struct {
    vertex* vertices;
    unsigned int* indices;
    unsigned int indexCount;
    unsigned int vertexCount;
} shape;


extern shape* sponge;


void buildShape(int spongeLevel);
