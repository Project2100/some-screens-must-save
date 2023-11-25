
cbuffer colora : register(b0) {
    float4 colorb;
};

float4 PShader(float4 position : SV_Position, float4 color : COLOR, bool front : SV_IsFrontFace) : SV_Target {

    float4 white = {1.0f, 1.0f, 1.0f, 1.0f};
    float4 black = {0.0f, 0.0f, 0.0f, 1.0f};
    
    if (!front) return colorb;

    return black;
}
