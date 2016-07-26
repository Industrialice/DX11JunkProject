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
}

void VS( float4 i_pos : POSITION, out float4 o_pos : SV_Position, out float3 o_tex : TEXCOORD )
{
    i_pos = float4( mul( i_pos, W ), 1 );
	o_pos = mul( i_pos, VP ).xyww;
    o_tex = i_pos.xyz;
}