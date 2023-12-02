struct VertexIn {
    float3 PosL : POSITION;
    float3 normal : NORMAL;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // 在世界中的位置
    float3 normal : NORMAL;
    float4 Color : COLOR;
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
    vOut.normal = normalize(mul(vIn.normal, (float3x3) g_World));
    vOut.PosH = mul(vOut.PosH, g_View);               // 行列数相等的两个矩阵，结果为
    vOut.PosH = mul(vOut.PosH, g_Proj);               // Cij = Aij * Bij
    vOut.Color = vIn.Color;                         // 这里alpha通道的值默认为1.0
    vOut.PosW = mul(float4(vIn.PosL, 1.0f), g_World).xyz;
   
    return vOut;
}
