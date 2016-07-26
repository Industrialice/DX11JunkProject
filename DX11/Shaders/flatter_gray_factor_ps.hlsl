cbuffer None : register(b0)
{
	uint Colors : packoffset(c0.x);
}

Texture2D < float4 > Tex : register(t0);
SamplerState Samp : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
	float4 color = Tex.Sample( Samp, i_tex );
	float factor = 1.f / Colors;
	float scaled = (color.r + color.g + color.b) / 3.f;
	scaled /= factor;
	scaled = floor( scaled + 0.5f );
	scaled *= factor;
	color.r = color.g = color.b = scaled;
	return color;
}