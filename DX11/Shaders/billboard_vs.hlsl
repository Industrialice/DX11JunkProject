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

struct SVsOut
{
	float4 posH : SV_Position;
	float2 texCoord : TEXCOORD;
};

SVsOut VS( float4 pos : POSITION, float2 texcoord : TEXCOORD )
{
	SVsOut vertex;

	float3 up = YVec;
	float3 look = normalize( POS - EyePos );
	float3 right = cross( up, look );
	
	float4x3 o_w = float4x3( right.x, right.y, right.z,
				             up.x, up.y, up.z,
				             0, 0, 0,
				             POS.x, POS.y, POS.z );
	
    float3 wpos = mul( float4( pos.xy * SIZE.xy, 0.f, 1.f ), o_w );

	vertex.posH = mul( float4( wpos, 1.f ), VP );
	vertex.texCoord = texcoord;
	
	return vertex;
}