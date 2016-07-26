/*cbuffer CubesData : register(b0)
{
    float4 W0 : packoffset(c0);
    float4 W1 : packoffset(c1);
    float4 W2 : packoffset(c2);
    float3 WIT0 : packoffset(c3);
    float3 WIT1 : packoffset(c4);
    float3 WIT2 : packoffset(c5);
}*/

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

/*void M3x3InverseTranspose4x3( out float3 orow0, out float3 orow1, out float3 orow2, float3 icol0, float3 icol1, float3 icol2 )
{
	orow0[0] = + (icol1[1] * icol2[2] - icol2[1] * icol1[2]);
	orow0[1] = - (icol0[1] * icol2[2] - icol2[1] * icol0[2]);
	orow0[2] = + (icol0[1] * icol1[2] - icol1[1] * icol0[2]);

	orow1[0] = - (icol1[0] * icol2[2] - icol2[0] * icol1[2]);
	orow1[1] = + (icol0[0] * icol2[2] - icol2[0] * icol0[2]);
	orow1[2] = - (icol0[0] * icol1[2] - icol1[0] * icol0[2]);

	orow2[0] = + (icol1[0] * icol2[1] - icol2[0] * icol1[1]);
	orow2[1] = - (icol0[0] * icol2[1] - icol2[0] * icol0[1]);
	orow2[2] = + (icol0[0] * icol1[1] - icol1[0] * icol0[1]);

	float det = + icol0[0] * orow0[0] + icol1[0] * orow0[1] + icol2[0] * orow0[2];
    float revDet = 1.f / det;

    orow0 *= revDet;
    orow1 *= revDet;
    orow2 *= revDet;
}*/

void VS( float4 i_pos : POSITION, float2 i_tex : TEXCOORD0, float3 i_nor : NORMAL0, float3 i_tan : TANGENT0, float3 pos : LOCATION, float2 ampSpeedRange : AMPPARAMS, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0, out float3 o_posW : POSITION, out float3x3 o_TBN : TBN )
{
    /*float4 w0 = W0;
    float4 w1 = W1;
    float4 w2 = W2;

    w0.w += pos.x;
    w1.w += pos.y;
    w2.w += pos.z;
        
	o_posW = float3( dot( i_pos, w0 ), dot( i_pos, w1 ), dot( i_pos, w2 ) );

    o_tex = i_tex;
    float3 norProcd = float3( dot( i_nor, WIT0 ), dot( i_nor, WIT1 ), dot( i_nor, WIT2 ) );
	float3 N = normalize( norProcd );
	float3 T = normalize( float3( dot( i_tan, W0 ), dot( i_tan, W1 ), dot( i_tan, W2 ) ) );
	T = normalize( T - dot( T, N ) * N );
	float3 B = cross( N, T );
	o_TBN = float3x3( T, B, N );

    o_pos = mul( float4( o_posW, 1 ), VP );*/

    float4 wrows[ 3 ] = (float4[ 3 ])W;
    float4 pos4 = float4( pos, 1 );

    pos = mul( float4( pos, 1 ), WROT );

    float4x3 w = W;
    w[ 3 ][ 0 ] += pos.x;
    w[ 3 ][ 1 ] += pos.y;
    w[ 3 ][ 2 ] += pos.z;

    o_posW = mul( i_pos, w );

    o_tex = i_tex;
    float3 norProcd = mul( i_nor, WIT );
	float3 N = normalize( norProcd );
	float3 T = normalize( mul( i_tan, (float3x3)W ) );
	T = normalize( T - dot( T, N ) * N );
	float3 B = cross( N, T );
	o_TBN = float3x3( T, B, N );

    o_pos = mul( float4( o_posW, 1 ), VP );
}