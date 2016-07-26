cbuffer Buf0
{
    float4 ScreenRect : packoffset( c0 );
	float2x2 TexMatrix : packoffset(c1);
	float2 TexRotCenter : packoffset(c1.z);
	float2 TexOffset : packoffset(c2.z);
}

void VS( in uint id : SV_VertexID, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0 )
{
    float2 tex = float2( id >> 1, ~id & 1 );
	o_tex = mul( tex - TexRotCenter, TexMatrix ) + TexOffset;
    o_pos = float4( ScreenRect[ id & 2 ], ScreenRect[ 3 >> (id & 1) ], 0, 1 );
}