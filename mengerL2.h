layer L2_template0 = {
    .vtxcount = 40,
    .idxCount = 3 * 28,
    .indexmap = (unsigned int[3 * 28]) {
         0,  2,  7,  0,  7,  1,  1,  7, 37,  1, 37, 39, 39, 37, 32, 39, 32, 38, 38, 32,  2, 38,  2,  0,
         5, 11, 12,  5, 12,  6, 23, 22, 30, 23, 30, 31, 34, 28, 27, 34, 27, 33, 16, 17,  9, 16,  9,  8,
         3, 14, 15,  3, 10,  4, 10, 15, 13, 13, 15, 25, 13, 18, 19, 18, 25, 36, 36, 25, 24, 36, 29, 35, 29, 24, 26, 26, 24, 14, 26, 21, 20, 21, 14,  3}
};

layer L2_template1 = {
    .vtxcount = 64,
    .idxCount = 3 * 48,
    .indexmap = (unsigned int[3 * 48]) {
         0,  7,  8,  0,  8,  1,  2,  9, 10,  2, 10,  3,  4, 11, 12,  4, 12,  5,
        13, 12, 20, 13, 20, 21, 31, 30, 38, 31, 38, 39, 49, 48, 56, 49, 56, 57,
        63, 56, 55, 63, 55, 62, 61, 54, 53, 61, 53, 60, 59, 52, 51, 59, 51, 58,
        50, 51, 43, 50, 43, 42, 32, 33, 25, 32, 25, 24, 14, 15,  7, 14,  7,  6,
         8, 16, 17,  8, 17,  9, 10, 18, 19, 10, 19, 11, 20, 19, 29, 20, 29, 30, 38, 37, 47, 38, 47, 48, 55, 47, 46, 55, 46, 54, 53, 45, 44, 53, 44, 52, 43, 44, 34, 43, 34, 33, 25, 26, 16, 25, 16, 15,
        17, 22, 23, 17, 23, 18, 29, 28, 36, 29, 36, 37, 46, 41, 40, 46, 40, 45, 34, 35, 27, 34, 27, 26}
};

layer L2_template2 = {
    .vtxcount = 28,
    .idxCount = 3 * 32,
    .indexmap = (unsigned int[3 * 32]) {
         0,  2,  3,  0,  3,  1,  1,  3,  5,  1,  5,  8,  8,  5,  4,  8,  4,  7,  7,  4,  2,  7,  2,  0,
         9, 13, 17,  9, 17, 21, 21, 17, 16, 21, 16, 20, 20, 16, 12, 20, 12,  8,  8, 12, 13,  8, 13,  9,
        27, 25, 24, 27, 24, 26, 26, 24, 22, 26, 22, 19, 19, 22, 23, 19, 23, 20, 20, 23, 25, 20, 25, 27,
        18, 14, 10, 18, 10,  6,  6, 10, 11,  6, 11,  7,  7, 11, 15,  7, 15, 19, 19, 15, 14, 19, 14, 18}
};

layer L2_template3 = {
    .vtxcount = 48,
    .idxCount = 3 * 32,
    .indexmap = (unsigned int[3 * 32]) {
         0,  5,  6,  0,  6,  1,  7,  6, 14,  7, 14, 15, 21, 14, 13, 21, 13, 20, 12, 13,  5, 12,  5,  4,
        11, 10, 18, 11, 18, 19, 23, 18, 17, 23, 17, 22, 16, 17,  9, 16,  9,  8,  2,  9, 10,  2, 10,  3,
        47, 42, 41, 47, 41, 46, 40, 41, 33, 40, 33, 32, 26, 33, 34, 26, 34, 27, 35, 34, 42, 35, 42, 43,
        36, 37, 29, 36, 29, 28, 24, 29, 30, 24, 30, 25, 31, 30, 38, 31, 38, 39, 45, 38, 37, 45, 37, 44}
};

layer L2_template0_flipped = {0};
layer L2_template1_flipped = {0};
layer L2_template2_flipped = {0};
layer L2_template3_flipped = {0};

void L2_completeLayers() {

    L2_template0_flipped = L2_template0;
    L2_template0_flipped.indexmap = flipLayer(L2_template0.indexmap, L2_template0.idxCount);

    L2_template1_flipped = L2_template1;
    L2_template1_flipped.indexmap = flipLayer(L2_template1.indexmap, L2_template1.idxCount);

    L2_template2_flipped = L2_template2;
    L2_template2_flipped.indexmap = flipLayer(L2_template2.indexmap, L2_template2.idxCount);

    L2_template3_flipped = L2_template3;
    L2_template3_flipped.indexmap = flipLayer(L2_template3.indexmap, L2_template3.idxCount);

}


#define LAYER_COUNT 10
shape mengerL2 = {
    .layerCount = LAYER_COUNT,
    .layers = (layer*[LAYER_COUNT]) {
        &L2_template0,
        &L2_template1_flipped,
        &L2_template1,
        &L2_template2_flipped,
        &L2_template3_flipped,
        &L2_template3,
        &L2_template2,
        &L2_template1_flipped,
        &L2_template1,
        &L2_template0_flipped
    },
    .compileLayers = &L2_completeLayers
};
#undef LAYER_COUNT
