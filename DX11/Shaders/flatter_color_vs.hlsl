void VS( in uint id : SV_VertexID, out float4 o_pos : SV_Position )
{
	o_pos = float4( float2( (id << 1) & 2, id & 2 ) * float2( 2, -2 ) + float2( -1, 1 ), 0, 1 );
}