Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD ) : SV_Target
{
	return CMap.Sample( CMapSampler, i_tex );
}