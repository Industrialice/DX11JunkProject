cbuffer Test : register(b0)
{
	float4x4 WVP : packoffset(c0);
	float4x3 W : packoffset(c4);
	float4x3 WROT : packoffset(c7);
	float4x3 WIT : packoffset(c10);
	float4 POS : packoffset(c13);
	float4 SIZE : packoffset(c14);
}

cbuffer FrameData : register(b13)
{
	float3 EyePos : packoffset(c0);
	float DT : packoffset(c0.w);
	float4x4 VP : packoffset(c1);
	float3 XVec : packoffset(c5);
	float3 YVec : packoffset(c6);
	float3 ZVec : packoffset(c7);
	float4x4 P : packoffset(c8);
}

void VS( float4 i_pos : POSITION, float2 i_tex : TEXCOORD0, float3 pos : LOCATION, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0 )
{
    float4 wrows[ 3 ] = (float4[ 3 ])W;
    float4 pos4 = float4( pos, 1 );

    pos = mul( float4( pos, 1 ), WROT );

    float4x3 w = W;
    w[ 3 ][ 0 ] += pos.x;
    w[ 3 ][ 1 ] += pos.y;
    w[ 3 ][ 2 ] += pos.z;

    float3 posW = mul( i_pos, w );

    o_pos = mul( float4( posW, 1 ), VP );

    o_tex = i_tex;
}