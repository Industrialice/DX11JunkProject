cbuffer Buf
{
    float4 ColorMult;
}

float4 PS( float4 posH : SV_Position ) : SV_Target
{
	return ColorMult;
}