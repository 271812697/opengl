struct VertexIn {
    float3 PosL : POSITION;
    float3 normal : NORMAL;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // �������е�λ��
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
    vOut.PosH = mul(float4(vIn.PosL, 1.0f), g_World);  // mul ���Ǿ���˷�, �����*Ҫ���������Ϊ
    vOut.normal = normalize(mul(vIn.normal, (float3x3) g_World));
    vOut.PosH = mul(vOut.PosH, g_View);               // ��������ȵ��������󣬽��Ϊ
    vOut.PosH = mul(vOut.PosH, g_Proj);               // Cij = Aij * Bij
    vOut.Color = vIn.Color;                         // ����alphaͨ����ֵĬ��Ϊ1.0
    vOut.PosW = mul(float4(vIn.PosL, 1.0f), g_World).xyz;
   
    return vOut;
}
