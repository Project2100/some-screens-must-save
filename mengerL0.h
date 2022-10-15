#define LCL0 2

void L0_completeLayers() {}


layerInfo mengerL0 = {
    .layerCount = LCL0,
    .layers = (layer*[LCL0]){
        &((layer){
            .vtxcount = 4,
            .idxCount = 3 * 2,
            .indexmap = ((unsigned int[3 * 2]) {0, 2, 1, 1, 2, 3}), // Negative layer - counter
        }),
        &((layer){
            .vtxcount = 4,
            .idxCount = 3 * 2,
            .indexmap = ((unsigned int[3 * 2]) {0, 1, 2, 1, 3, 2}), // Positive layer - clock
        }),
    },
    .compileLayers = &L0_completeLayers
};
