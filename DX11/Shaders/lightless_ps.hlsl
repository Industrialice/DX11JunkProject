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

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0 ) : SV_Target
{
	float4 sampled = CMap.Sample( CMapSampler, i_tex );
    sampled.rgb *= ExternalLightPower;
    return sampled;
}