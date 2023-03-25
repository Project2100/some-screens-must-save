#define LCL0 2


IndexingKit mengerL0 = {
    .layerCount = LCL0,
    .templates = NULL,
    .templatesFlipped = NULL,
    .templateSequence = (Layer*[LCL0]){
        &((Layer){
            .vtxcount = 4,
            .idxCount = 3 * 2,
            .indexmap = ((unsigned int[3 * 2]) {0, 2, 1, 1, 2, 3}), // Negative layer - counter
        }),
        &((Layer){
            .vtxcount = 4,
            .idxCount = 3 * 2,
            .indexmap = ((unsigned int[3 * 2]) {0, 1, 2, 1, 3, 2}), // Positive layer - clock
        }),
    },
};
