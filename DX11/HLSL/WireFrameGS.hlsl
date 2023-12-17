
struct Vertex
{
    float4 positionCS : SV_POSITION;

};

struct GSOutput
{
    float4 positionCS  : SV_POSITION;
    noperspective float3 _distance: TEXCOORD0;
};

cbuffer WireFrameConstantBuffer : register(b2)
{
        float4 _LineColor;
        float2 _ScreenParams;
        float _Thickness;
}


[maxvertexcount(3)]
void GS(
	triangle Vertex p[3],
	inout TriangleStream< GSOutput >triStream
)
{

				//NDC坐标，范围[-1,1]，然后*0.5+0.5后转为[0,1],最后乘以_ScreenParams转换为最终的屏幕空间位置
                float2 p0 = _ScreenParams.xy * (p[0].positionCS.xy / p[0].positionCS.w * 0.5 + 0.5);
                float2 p1 = _ScreenParams.xy * (p[1].positionCS.xy / p[1].positionCS.w * 0.5 + 0.5);
                float2 p2 = _ScreenParams.xy * (p[2].positionCS.xy / p[2].positionCS.w * 0.5 + 0.5);
               
                //edge vectors
                float2 v0 = p2 - p1;
                float2 v1 = p2 - p0;
                float2 v2 = p1 - p0;

                //area of the triangle
                float area = abs(v1.x*v2.y - v1.y * v2.x);

                //values based on distance to the edges
                float dist0 = area / length(v0);
                float dist1 = area / length(v1);
                float dist2 = area / length(v2);
               
				GSOutput pIn;
               
                //add the first point
                pIn.positionCS = p[0].positionCS;
                pIn._distance = float3(dist0,0,0);
                triStream.Append(pIn);

                //add the second point
                pIn.positionCS =  p[1].positionCS;
                pIn._distance = float3(0,dist1,0);
                triStream.Append(pIn);
               
                //add the third point
                pIn.positionCS = p[2].positionCS;
                pIn._distance = float3(0,0,dist2);
                triStream.Append(pIn);
}