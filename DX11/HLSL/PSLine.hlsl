

struct GSOutput
{
	float4 PosH : SV_POSITION;//�ü����������
	float3 PosW : POSITION;     // �������е�λ��
	float3 Nor :NORMAL;
	float4 Color : COLOR;
    float4 texcoord: TEXCOORD;
};


float4 main(GSOutput pIn) : SV_Target
{
   
   return pIn.Color;
}