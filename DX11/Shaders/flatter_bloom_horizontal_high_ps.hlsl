Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float Gaussian( float x, float deviation )
{
	return (1.f / sqrt(2.f * 3.141592f * deviation)) * exp(-((x * x) / (2.f * deviation)));
}

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{	
	float3 color = 0;
	[unroll]
	for( int index = -50; index <= 50; ++index )
	{
		float x = abs( index ) * (1.f / 15.f);
		float gauss = Gaussian( x, 1.f );
		float3 sample = CMap.Sample( CMapSampler, float2( index * PIXEL_SIZE_X + i_tex.x, i_tex.y ) ).xyz;
		color += sample * gauss;
	}
	color /= 10;
	return float4( color, 1.f );
}