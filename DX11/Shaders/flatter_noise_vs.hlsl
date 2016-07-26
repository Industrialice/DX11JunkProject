cbuffer Test : register(b0)
{
	float2x2 TexTransform : packoffset(c0.x);
	float2 TexOffset : packoffset(c0.z);
}

void VS( in uint id : SV_VertexID, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0 )
{
	o_tex = float2( (id << 1) & 2, id & 2 );
	o_pos = float4( o_tex * float2( 2, -2 ) + float2( -1, 1 ), 0, 1 );
	o_tex = mul( o_tex, TexTransform ) + TexOffset;
}