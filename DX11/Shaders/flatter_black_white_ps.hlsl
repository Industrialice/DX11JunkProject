cbuffer None : register(b0)
{
	float3 ColorLow : packoffset(c0.x);
	float3 ColorHigh : packoffset(c1.x);
};

Texture2D < float4 > Tex : register(t0);
SamplerState Samp : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
	float4 color = Tex.Sample( Samp, i_tex );
	if( color.r + color.g + color.b > 1.5f )
	{
		color.rgb = ColorHigh;
	}
	else
	{
		color.rgb = ColorLow;
	}
	return color;
}