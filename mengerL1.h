#define LCL1 LAYER_UP(LCL0)
#define TCL1 2

Layer L1_templates[TCL1] = {
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

Layer L1_templates_flipped[TCL1] = {0};


IndexingKit mengerL1 = {
    .layerCount = LCL1,
    .templates = L1_templates,
    .templatesFlipped = L1_templates_flipped,
    .templateSequence = (Layer*[LCL1]) {
        L1_templates,

        L1_templates_flipped + 1,
        L1_templates + 1,
        
        L1_templates_flipped
    },
};
