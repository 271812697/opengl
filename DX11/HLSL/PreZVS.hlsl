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
    vOut.PosH = mul(float4(vIn.PosL, 1.0f), g_World);  // mul ���Ǿ���˷�, �����*Ҫ���������Ϊ
  
    vOut.PosH = mul(vOut.PosH, g_View);               // ��������ȵ��������󣬽��Ϊ
  
    vOut.PosH = mul(vOut.PosH, g_Proj);               // Cij = Aij * Bij
    return vOut;
}
