

struct GSOutput
{
	float4 PosH : SV_POSITION;//裁剪的齐次坐标
	float3 PosW : POSITION;     // 在世界中的位置
	float3 Nor :NORMAL;
	float4 Color : COLOR;
    float4 texcoord: TEXCOORD;
};


float4 main(GSOutput pIn) : SV_Target
{
   
   return pIn.Color;
}