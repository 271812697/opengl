struct Vertex
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION; // 在世界中的位置
    float3 normal : NORMAL;
    float4 Color : COLOR;
    float4 texcoord: TEXCOORD;
};

struct GSOutput
{
	float4 PosH : SV_POSITION;//裁剪的齐次坐标
	float3 PosW : POSITION;     // 在世界中的位置
	float3 Nor :NORMAL;
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

[maxvertexcount(6)]
void GS(
	triangle Vertex input[3],
	inout LineStream< GSOutput > output
)
{
	//float3 N = cross(input[1].PosW-input[0].PosW, input[2].PosW - input[0].PosW);
	//N = normalize(N);
	for (uint i = 0; i < 3; i++)
	{
		GSOutput element;
		element.PosH = input[i].PosH;
		element.PosW = input[i].PosW;
		element.Color = input[i].Color;
        element.Nor = input[i].normal;
        element.texcoord=input[i].texcoord;
		output.Append(element);
        float wi=input[i].Color.x*3.14159;
        float theta = (input[i].Color.y-0.5)*6.2831;
        float3 dir = normalize(mul(float3(-sin(wi)*cos(theta),sin(wi)*sin(theta),cos(wi)), (float3x3) g_World));
        element.PosH = mul( mul(float4(element.PosW+dir*input[i].texcoord.x,1.0), g_View),g_Proj);
        element.PosW = element.PosW+dir*input[i].texcoord.x;
        output.Append(element);
        output.RestartStrip();
	}
}