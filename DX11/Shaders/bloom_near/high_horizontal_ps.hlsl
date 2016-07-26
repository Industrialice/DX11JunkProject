#define KERNEL 40

Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float4 PS( float4 posH : SV_Position, float4 datas[ KERNEL / 2 ] : DATAS, float texy : TEXCOORD ) : SV_Target0
{	
	float3 color = 0;
	float2 datas2[ KERNEL ] = (float2[ KERNEL ])datas;
	[unroll]
	for( int index = 0; index < KERNEL; ++index )
	{
		float2 tex = float2( datas2[ index ].x, texy );
		float3 sample = CMap.Sample( CMapSampler, tex ).xyz;
		color += sample * datas2[ index ].y;
	}
	return float4( color, 1.f );
}