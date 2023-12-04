
struct VertexIn {
    float3 PosL : POSITION;
    float3 normal : NORMAL;
    float2 texcoord: TEXCOORD;
    float3 spherepos : SPHEREPOSITION;//实例数据
    float4 spherecolor: SPHERECOLOR;//实例数据
    float4 spheretexcoord:SPHERETEXCOORD;//实例数据
};
struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;     // 在世界中的位置
    float3 normal : NORMAL;
    float4 Color : COLOR;
    float4 texcoord: TEXCOORD;
};
cbuffer VSConstantBuffer : register(b0)
{
    matrix g_World;
    matrix g_View;
    matrix g_Proj;
    matrix g_WorldInvTranspose;
}
cbuffer VSConstantBufferInstance : register(b8)
{
      float vertex_scale;
      float4 vertex_color;
      int vertex_mode;
      float vertex_clip;

}

VertexOut main( VertexIn vIn) 
{
    VertexOut vOut;
    float wi=vIn.spherecolor.x*3.14159;
    float theta = (vIn.spherecolor.y-0.5)*6.2831;
    float3 dir = float3(-sin(wi)*cos(theta),sin(wi)*sin(theta),cos(wi))*vIn.spheretexcoord.x;
    if(vertex_mode==0){
         dir=float3(0,0,0);
    }
    vOut.PosH = mul(float4(vIn.PosL*vertex_scale+vIn.spherepos+dir, 1.0f), g_World);  // mul 才是矩阵乘法, 运算符*要求操作对象为
    vOut.normal = normalize(mul(vIn.normal, (float3x3) g_World));
    vOut.PosH = mul(vOut.PosH, g_View);               // 行列数相等的两个矩阵，结果为
    vOut.PosH = mul(vOut.PosH, g_Proj);               // Cij = Aij * Bij
    if(vIn.spheretexcoord.y<vertex_clip){
    vOut.PosH.z=vOut.PosH.w*1.1;
    
    }
    vOut.Color = float4(1.0,0.0,0.0,1.0);                         // 这里alpha通道的值默认为1.0
    vOut.PosW = mul(float4(vIn.PosL, 1.0f), g_World).xyz;
    vOut.texcoord=float4(vIn.texcoord,1.0,1.0);
   
    return vOut;
}