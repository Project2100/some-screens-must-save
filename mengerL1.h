vertex mengerL1_vtcs[] = {

    { SU,    SU,   -SU,   {0.3f, 0.3f, 0.3f, 1.0f}},
    { SU,   -SU,   -SU,   {0.3f, 0.3f, 0.3f, 1.0f}},
    {-SU,    SU,   -SU,   {0.3f, 0.3f, 0.3f, 1.0f}},
    {-SU,   -SU,   -SU,   {0.3f, 0.3f, 0.3f, 1.0f}},
    { SU,    SU,    SU,   {0.3f, 0.3f, 0.3f, 1.0f}},
    { SU,   -SU,    SU,   {0.3f, 0.3f, 0.3f, 1.0f}},
    {-SU,    SU,    SU,   {0.3f, 0.3f, 0.3f, 1.0f}},
    {-SU,   -SU,    SU,   {0.3f, 0.3f, 0.3f, 1.0f}},

    { SU,    SU/3, -SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    { SU,   -SU/3, -SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-SU,    SU/3, -SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-SU,   -SU/3, -SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    { SU,    SU/3,  SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    { SU,   -SU/3,  SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-SU,    SU/3,  SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},
    {-SU,   -SU/3,  SU/3, {0.5f, 0.5f, 0.0f, 1.0f}},

    { SU/3,  SU,   -SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    { SU/3, -SU,   -SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-SU/3,  SU,   -SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-SU/3, -SU,   -SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    { SU/3,  SU,    SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    { SU/3, -SU,    SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-SU/3,  SU,    SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},
    {-SU/3, -SU,    SU/3, {0.5f, 0.0f, 0.5f, 1.0f}},

    { SU/3,  SU/3, -SU,   {0.0f, 0.5f, 0.5f, 1.0f}},
    { SU/3, -SU/3, -SU,   {0.0f, 0.5f, 0.5f, 1.0f}},
    {-SU/3,  SU/3, -SU,   {0.0f, 0.5f, 0.5f, 1.0f}},
    {-SU/3, -SU/3, -SU,   {0.0f, 0.5f, 0.5f, 1.0f}},
    { SU/3,  SU/3,  SU,   {0.0f, 0.5f, 0.5f, 1.0f}},
    { SU/3, -SU/3,  SU,   {0.0f, 0.5f, 0.5f, 1.0f}},
    {-SU/3,  SU/3,  SU,   {0.0f, 0.5f, 0.5f, 1.0f}},
    {-SU/3, -SU/3,  SU,   {0.0f, 0.5f, 0.5f, 1.0f}},

    { SU/3,  SU/3, -SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    { SU/3, -SU/3, -SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-SU/3,  SU/3, -SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-SU/3, -SU/3, -SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    { SU/3,  SU/3,  SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    { SU/3, -SU/3,  SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-SU/3,  SU/3,  SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},
    {-SU/3, -SU/3,  SU/3, {1.0f, 1.0f, 1.0f, 1.0f}},

};



layer* L1_layers[] = {
    &((layer) {
        .vtxcount = 8,
        .idxCount = 24,
        .indexmap = ((unsigned int[24]) {0, 2, 3, 0, 3, 1, 1, 3, 5, 1, 5, 7, 7, 5, 4, 7, 4, 6, 6, 4, 2, 6, 2, 0}), // counter, reading order (it works?!?)
    }),
    &((layer) {
        .vtxcount = 12,
        .idxCount = 24,
        // .indexmap = ((unsigned int[24]) {0, 1, 2, 1, 3, 2}), // Negative layer
    }),
    &((layer) {
        .vtxcount = 12,
        .idxCount = 24,
        .indexmap = ((unsigned int[24]) {0, 3, 4, 0, 4, 1, 5, 4, 8, 5, 8, 9, 11, 8, 7, 11, 7, 10, 6, 7, 3, 6, 3, 2}), // counter, reading order (it works?!?)
    }),
    &((layer) {
        .vtxcount = 8,
        .idxCount = 24,
        // .indexmap = ((unsigned int[24]) {0, 1, 2, 1, 3, 2}), // Positive layer
    }),
};


void L1_completeLayers() {
    L1_layers[1]->indexmap = flipClone(L1_layers[2]->indexmap, 24);
    L1_layers[3]->indexmap = flipClone(L1_layers[0]->indexmap, 24);
}



shape mengerL1 = {
    .vertices = mengerL1_vtcs,
    .vertexSize = sizeof mengerL1_vtcs,
    .vertexCount = sizeof mengerL1_vtcs / sizeof *mengerL1_vtcs,
    .layers = L1_layers,
    .layerCount = sizeof L1_layers / sizeof L1_layers[0],
};
