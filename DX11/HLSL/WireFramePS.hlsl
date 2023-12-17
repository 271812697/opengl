struct GSOutput{
   float4 positionCS  : SV_POSITION;
   noperspective float3 _distance: TEXCOORD0;
};

cbuffer WireFrameConstantBuffer : register(b2)
{
        float4 _LineColor;
        float2 _ScreenParams;
        float  _Thickness;
}


float4 PS(GSOutput input) : SV_Target
{    


    float t = smoothstep(_Thickness-1.5, _Thickness+2, min(min(input._distance.x, input._distance.y), input._distance.z));
    if(t>=0.01)discard;           
    return float4(_LineColor.rgb,1.0);
}
