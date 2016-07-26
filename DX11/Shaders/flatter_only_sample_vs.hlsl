cbuffer Buf0
{
    float4 ScreenRect : packoffset( c0 );
    bool is_CutTexcoord : packoffset( c1 );
}

void VS( in uint id : SV_VertexID, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0 )
{
    float x = ScreenRect[ id & 2 ];
    float y = ScreenRect[ 3 >> (id & 1) ];
    if( is_CutTexcoord )
    {
        o_tex.x = x * 0.5f + 0.5f;
        o_tex.y = -y * 0.5f + 0.5f;
    }
    else
    {
	    o_tex = float2( id >> 1, ~id & 1 );
    }
    o_pos = float4( x, y, 0, 1 );
}