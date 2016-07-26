cbuffer Buf0
{
    float4 ScreenRect : packoffset( c0 );
    float3x3 Transformation : packoffset( c1 );
}

void VS( in uint id : SV_VertexID, out float4 o_pos : SV_Position )
{
    float2 v = float2( -0.5 + (id >> 1), -0.5 + (id & 1) );

    v = mul( float3( v, 1 ), Transformation ).xy;

    float x = ScreenRect[ id & 2 ];
    float y = ScreenRect[ 3 >> (id & 1) ];

    o_pos = float4( x, y, 0, 1 );
}