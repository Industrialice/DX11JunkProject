Texture2D < float4 > Tex : register(t0);
SamplerState Samp : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
	return Tex.Sample( Samp, i_tex );
}