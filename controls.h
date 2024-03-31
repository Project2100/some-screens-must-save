#define LABEL_SPONGE 101
#define LABEL_COLOUR 102
#define CTRL_SPONGE_LEVEL_1 103
#define CTRL_SPONGE_LEVEL_2 104
#define CTRL_SPONGE_LEVEL_3 105
#define CTRL_RAINBOW_SPEED 106

// REMINDER: TEXT is defined in winnt.h, not strictly necessary to include here because this header gets always included after windows.h in other files
#define REGNAME_SPONGE_LEVEL TEXT("SpongeLevel")
#define REGNAME_RAINBOW_SPEED TEXT("RainbowSpeed")
#define REGNAME_COLOUR_MODE TEXT("ColourMode")

// 0: White
// 1: Rainbow (colour shift speed tunable by "RainbowSpeed")
