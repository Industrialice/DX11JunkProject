cbuffer ObjData : register(b1)
{
	float4 DiffuseMaterial : packoffset(c0);
	float4 SpecularMaterial : packoffset(c1);
    float3 AmbientMaterial : packoffset(c2);
    float ExternalLightPower : packoffset(c2.w);
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

Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);
Texture2D GMap : register(t1);
SamplerState GMapSampler : register(s1);

struct SOutput
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

SOutput PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
	float4 diffuse = CMap.Sample( CMapSampler, i_tex ) * DiffuseMaterial;
	float4 glowCol = GMap.Sample( GMapSampler, i_tex );
	
	SOutput op;
	op.color0 = diffuse;
	op.color1 = glowCol;
	return op;
}