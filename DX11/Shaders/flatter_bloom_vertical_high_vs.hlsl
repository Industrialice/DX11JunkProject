void VS( in uint id : SV_VertexID, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0 )
{
	o_tex = float2( (id << 1) & 2, id & 2 );
	o_pos = float4( o_tex * float2( 2, -2 ) + float2( -1, 1 ), 0, 1 );
}