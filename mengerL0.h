vertex mengerL0_vtcs[] = {
    {{ SU,  SU, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
    {{ SU, -SU, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
    {{-SU,  SU, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
    {{-SU, -SU, -SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
    {{ SU,  SU,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
    {{ SU, -SU,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
    {{-SU,  SU,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
    {{-SU, -SU,  SU, 1}, {0.3f, 0.3f, 0.3f, 1.0f}},
};


layer* L0_layers[2] = {
    &((layer){
        .vtxcount = 4,
        .idxCount = 6,
        .indexmap = ((unsigned int[6]) {0, 2, 1, 1, 2, 3}), // Negative layer - counter
    }),
    &((layer){
        .vtxcount = 4,
        .idxCount = 6,
        .indexmap = ((unsigned int[6]) {0, 1, 2, 1, 3, 2}), // Positive layer - clock
    }),
};

void L0_completeLayers() {}

shape mengerL0 = {
    .vertices = mengerL0_vtcs,
    .vertexSize = sizeof mengerL0_vtcs,
    .vertexCount = sizeof mengerL0_vtcs / sizeof mengerL0_vtcs[0],
    .layers = L0_layers,
    .layerCount = sizeof L0_layers / sizeof L0_layers[0],
    .compileLayers = &L0_completeLayers
};
