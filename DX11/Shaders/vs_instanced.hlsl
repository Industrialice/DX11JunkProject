#define LIGHTS_COUNT 1

struct SLight
{
	float4 pos;
	float3 col;
	float power;
};

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

cbuffer LightData : register(b12)
{
	SLight Lights[ LIGHTS_COUNT ] : packoffset(c0);
}

void VS( float4 i_pos : POSITION, float2 i_tex : TEXCOORD0, float3 i_nor : NORMAL0, float3 i_tan : TANGENT0, float3 i_wrow0 : W_ROW0, float3 i_wrow1 : W_ROW1, float3 i_wrow2 : W_ROW2, float3 i_wrow3 : W_ROW3, float4 wit_col0 : WIT_COL0, float4 wit_col1 : WIT_COL1, float4 wit_col2 : WIT_COL2, float4 wit_col3 : WIT_COL3, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0, out float3 o_posW : POSITION, out float3x3 o_TBN : TBN )
{
	row_major float4x4 o_w = float4x4( i_wrow0, 0,
						               i_wrow1, 0,
							           i_wrow2, 0,
							           i_wrow3, 1 );
	float4x4 o_wvp = mul( o_w, VP );
	column_major float4x4 o_wit = float4x4( wit_col0.x, wit_col1.x, wit_col2.x, wit_col3.x,
                                            wit_col0.y, wit_col1.y, wit_col2.y, wit_col3.y,
                                            wit_col0.z, wit_col1.z, wit_col2.z, wit_col3.z,
                                            wit_col0.w, wit_col1.w, wit_col2.w, wit_col3.w );
	o_posW = mul( i_pos, o_w );
	o_pos = mul( i_pos, o_wvp );
    o_tex = i_tex;
	float3 N = normalize( mul( i_nor, o_wit ) );
	float3 T = normalize( mul( i_tan, (row_major float3x3)o_w ) );
	T = normalize( T - dot( T, N ) * N );
	float3 B = cross( N, T );
	o_TBN = float3x3( T, B, N );
}