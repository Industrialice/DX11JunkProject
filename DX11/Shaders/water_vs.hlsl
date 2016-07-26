cbuffer Test : register(b0)
{
	float4x4 WVP : packoffset(c0);
	float4x3 W : packoffset(c4);
	float4x3 WROT : packoffset(c7);
	float4x3 WIT : packoffset(c10);
	float4 POS : packoffset(c13);
	float4 SIZE : packoffset(c14);
}

cbuffer Mat : register(b1)
{
	float2x2 Rot0 : packoffset(c0);
	float2 TexRotCenter0 : packoffset(c0.z);
	float2 TexOffset0 : packoffset(c1.z);
	float2x2 Rot1 : packoffset(c2);
	float2 TexRotCenter1 : packoffset(c2.z);
	float2 TexOffset1 : packoffset(c3.z);
}

void VS( float4 i_pos : POSITION, float2 i_tex : TEXCOORD0, float3 i_nor : NORMAL0, float3 i_tan : TANGENT0, out float4 o_pos : SV_Position, out float4 o_tex : TEXCOORD0, out float3 o_posW : POSITION, out float3x3 o_TBN : TBN )
{
	o_posW = mul( i_pos, W );
	o_pos = mul( i_pos, WVP );
	float2 tex0 = mul( i_tex - TexRotCenter0, Rot0 ) + TexOffset0;
	float2 tex1 = mul( i_tex - TexRotCenter1, Rot1 ) + TexOffset1;
	o_tex = float4( tex0, tex1 );
	float3 N = normalize( mul( i_nor, WIT ) );
	float3 T = normalize( mul( i_tan, (float3x3)W ) );
	T = normalize( T - dot( T, N ) * N );
	float3 B = cross( N, T );
	o_TBN = float3x3( T, B, N );
}