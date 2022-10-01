layer L1_templates[2] = {
    {
        .vtxcount = 8,
        .idxCount = 3 * 8,
        .indexmap = ((unsigned int[3 * 8]) {0, 2, 3, 0, 3, 1, 1, 3, 5, 1, 5, 7, 7, 5, 4, 7, 4, 6, 6, 4, 2, 6, 2, 0}),
    },
    {
        .vtxcount = 12,
        .idxCount = 3 * 8,
        .indexmap = ((unsigned int[3 * 8]) {0, 3, 4, 0, 4, 1, 5, 4, 8, 5, 8, 9, 11, 8, 7, 11, 7, 10, 6, 7, 3, 6, 3, 2}),
    }
};

layer L1_templates_flipped[2] = {0};

void L1_completeLayers() {

    for (int i = 0; i < 2; i++) {
        L1_templates_flipped[i] = L1_templates[i];
        L1_templates_flipped[i].indexmap = flipLayer(L1_templates[i].indexmap, L1_templates[i].idxCount);
    }
}


#define LAYER_COUNT 4
layerInfo mengerL1 = {
    .layerCount = LAYER_COUNT,
    .layers = (layer*[LAYER_COUNT]) {
        L1_templates,
        L1_templates_flipped + 1,
        L1_templates + 1,
        L1_templates_flipped
    },
    .compileLayers = &L1_completeLayers
};
#undef LAYER_COUNT
