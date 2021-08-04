layer L1_template0 = {
    .vtxcount = 8,
    .idxCount = 3 * 8,
    .indexmap = ((unsigned int[3 * 8]) {0, 2, 3, 0, 3, 1, 1, 3, 5, 1, 5, 7, 7, 5, 4, 7, 4, 6, 6, 4, 2, 6, 2, 0}),
};

layer L1_template1 = {
    .vtxcount = 12,
    .idxCount = 3 * 8,
    .indexmap = ((unsigned int[3 * 8]) {0, 3, 4, 0, 4, 1, 5, 4, 8, 5, 8, 9, 11, 8, 7, 11, 7, 10, 6, 7, 3, 6, 3, 2}),
};

layer L1_template0_flipped = {0};
layer L1_template1_flipped = {0};

void L1_completeLayers() {

    L1_template0_flipped = L1_template0;
    L1_template0_flipped.indexmap = flipLayer(L1_template0.indexmap, L1_template0.idxCount);

    L1_template1_flipped = L1_template1;
    L1_template1_flipped.indexmap = flipLayer(L1_template1.indexmap, L1_template1.idxCount);
}


#define LAYER_COUNT 4
shape mengerL1 = {
    .layerCount = LAYER_COUNT,
    .layers = (layer*[LAYER_COUNT]) {
        &L1_template0,
        &L1_template1_flipped,
        &L1_template1,
        &L1_template0_flipped
    },
    .compileLayers = &L1_completeLayers
};
#undef LAYER_COUNT
