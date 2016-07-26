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
	float2x2 Rot : packoffset(c0);
	float2 TexRotCenter : packoffset(c0.z);
	float2 TexOffset : packoffset(c1.z);
}

void VS( float4 i_pos : POSITION, float2 i_tex : TEXCOORD0, out float4 o_pos : SV_Position, out float2 o_tex : TEXCOORD0 )
{
	o_pos = mul( i_pos, WVP );
	o_tex = mul( i_tex - TexRotCenter, Rot ) + TexOffset;
}