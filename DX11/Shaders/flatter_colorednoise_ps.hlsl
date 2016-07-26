cbuffer Test : register(b0)
{
	float2 TexSizeF : packoffset(c0.x);
	float NoiseScale : packoffset(c0.z);
}

Texture2D < float4 > Tex : register(t0);
SamplerState Samp : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{	
	float4 color = Tex.Sample( Samp, i_tex );
	color.a *= NoiseScale;
	return color;
}