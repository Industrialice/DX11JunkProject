cbuffer Test : register(b0)
{
	float2 TexSizeF : packoffset(c0.x);
	float NoiseScale : packoffset(c0.z);
}

Texture2D < uint > tMask : register(t0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
	uint2 coords = (i_tex - floor(i_tex)) * TexSizeF;
	
	uint value = tMask.Load( uint3( coords, 0 ) );
	if( coords.x % 2 )
	{
		value >>= 4;
	}
	else
	{
		value &= 0xF;
	}
	
	return float4( 1, 1, 1, (float)value * NoiseScale );
}