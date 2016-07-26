Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float4 PS( float4 posH : SV_Position, float4 datas[ 5 ] : DATAS, float texy : TEXCOORD ) : SV_Target0
{	
	float2 datas2[ 10 ] = (float2[ 10 ])datas;
	float3 color = 0;
	[unroll]
	for( int index = 0; index < 10; ++index )
	{
		float3 sample = CMap.Sample( CMapSampler, float2( datas2[ index ].x, texy ) ).xyz;
		color += sample * datas2[ index ].y;
	}
	return float4( color, 1.f );
}