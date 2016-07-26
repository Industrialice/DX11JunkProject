Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float4 PS( float4 posH : SV_Position, in float4 datas[ 6 ] : DATAS, in float texx : TEXCOORD ) : SV_Target0
{	
	float2 datas2[ 12 ] = (float2[ 12 ])datas;
	float3 color = 0;
	[unroll]
	for( int index = 0; index < 12; ++index )
	{
		float3 sample = CMap.Sample( CMapSampler, float2( texx, datas2[ index ].x ) ).xyz;
		color += sample * datas2[ index ].y;
	}
	return float4( color, 1.f );
}