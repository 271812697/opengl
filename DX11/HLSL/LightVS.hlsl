struct VertexIn {
    float3 PosL : POSITION;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;

};
cbuffer VSConstantBuffer : register(b0)
{
    matrix g_World;
    matrix g_View;
    matrix g_Proj;
    matrix g_WorldInvTranspose;
}
VertexOut VS(VertexIn vIn)
{
    VertexOut vOut;
    vOut.PosH = mul(float4(vIn.PosL, 1.0f), g_World);  // mul 才是矩阵乘法, 运算符*要求操作对象为
  
    vOut.PosH = mul(vOut.PosH, g_View);               // 行列数相等的两个矩阵，结果为
  
    vOut.PosH = mul(vOut.PosH, g_Proj);               // Cij = Aij * Bij
    return vOut;
}
