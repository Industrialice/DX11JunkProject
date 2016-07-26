Texture2D < float4 > Tex : register(t0);
SamplerState Samp : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
	float4 color = Tex.Sample( Samp, i_tex );
	color.r = color.g = color.b = (color.r + color.g + color.b) / 3.f;
	return color;
}