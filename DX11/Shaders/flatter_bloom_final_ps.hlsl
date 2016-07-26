cbuffer Test : register(b0)
{
	float3 Amounts : packoffset(c0);
}

Texture2D CMap0 : register(t0);
Texture2D CMap1 : register(t1);
Texture2D CMap2 : register(t2);
SamplerState Sampler : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
    float3 col0 = CMap0.Sample( Sampler, i_tex ).rgb * Amounts.x;
    float3 col1 = CMap1.Sample( Sampler, i_tex ).rgb * Amounts.y;
    float3 col2 = CMap2.Sample( Sampler, i_tex ).rgb * Amounts.z;
	
	float3 colSum = col0 + col1 + col2;
	
	/*if( colSum.r > 1.0 )
	{
		float dt = 1.0 - colSum.r;
		colSum.g += dt / 2;
		colSum.b += dt / 2;
	}
	if( colSum.g > 1.0 )
	{
		float dt = 1.0 - colSum.g;
		colSum.b += dt;
	}*/
	
    return float4( colSum, 1.f );
}