#define KERNEL 24

Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float4 PS( float4 posH : SV_Position, in float4 datas[ KERNEL / 2 ] : DATAS, in float texx : TEXCOORD ) : SV_Target
{	
	float2 datas2[ KERNEL ] = (float2[ KERNEL ])datas;
	float3 color = 0;
	[unroll]
	for( int index = 0; index < KERNEL; ++index )
	{
		float2 tex = float2( texx, datas2[ index ].x );
		float3 sample = CMap.Sample( CMapSampler, tex ).xyz;
		color += sample * datas2[ index ].y;
	}
	return float4( color, 1.f );
}