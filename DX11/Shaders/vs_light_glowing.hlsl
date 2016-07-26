cbuffer Test : register(b0)
{
	float4x4 WVP : packoffset(c0);
	float4x4 W : packoffset(c4);
	float4x4 WIT : packoffset(c8);
}

cbuffer Mat : register(b1)
{
	float2x2 Rot : packoffset(c0);
	float2 TexRotCenter : packoffset(c0.z);
	float2 TexOffset : packoffset(c1.z);
}

void VS( in float4 i_pos : POSITION0, in float2 i_tex : TEXCOORD0, in float3 i_nor : NORMAL0, in float3 i_tan : TANGENT0, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0, out float3 o_posW : TEXCOORD1, out float3x3 o_TBN : TBN )
{
	o_posW = mul( i_pos, W ).xyz;
	o_pos = mul( i_pos, WVP );
	o_tex = mul( i_tex - TexRotCenter, Rot ) + TexOffset;
	float3 N = normalize( mul( i_nor, WIT ) );
	float3 T = normalize( mul( i_tan, (float3x3)W ) );
	T = normalize( T - dot( T, N ) * N );
	float3 B = cross( N, T );
	o_TBN = float3x3( T, B, N );
}