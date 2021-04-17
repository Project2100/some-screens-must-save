struct VOut {
    float4 position : SV_Position;
    float4 color : COLOR;
};

cbuffer transforms : register(b0) {
    matrix orient;
    matrix point_translate;
    matrix rotate;
    matrix translate;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR) {
    VOut output;

    // Translate, then rotate
    // matrix oper = mul(translate, rotate);

    // // Rotate, then translate
    // matrix oper = mul(rotate, translate);
    
    // Orient, then rotate, then translate
    // matrix oper1 = mul(orient, rotate);
    // matrix oper = mul(oper1, translate);
    
    matrix oper1 = mul(orient, point_translate);
    matrix oper2 = mul(oper1, rotate);
    matrix oper = mul(oper2, translate);

    // Vectors auto-adjust for multiplication
    output.position = mul(position, oper);
    output.color = color;

    return output;
}
