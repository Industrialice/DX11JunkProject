#define LIGHTS_COUNT 16

struct SLight
{
	float4 pos;
	float3 col;
	float power;
};

cbuffer ObjData : register(b1)
{
	float4 DiffuseMaterial : packoffset(c0);
	float4 SpecularMaterial : packoffset(c1);
}

cbuffer LightData : register(b12)
{
	SLight Lights[ LIGHTS_COUNT ] : packoffset(c0);
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

Texture2D NMap0 : register(t0);
SamplerState NMap0Sampler : register(s0);
Texture2D NMap1 : register(t1);
SamplerState NMap1Sampler : register(s1);

float4 PS( float4 posH : SV_Position, float4 i_tex : TEXCOORD0, float3 i_posW : POSITION, float3x3 i_TBN : TBN ) : SV_Target
{
	float3 toEye = normalize( EyePos - i_posW );
	
	float3 texnor0 = NMap0.Sample( NMap0Sampler, i_tex.xy ).xyz;
	float3 texnor1 = NMap1.Sample( NMap1Sampler, i_tex.zw ).xyz;
	float3 nor = normalize( (texnor0 + texnor1) - 1.f );
	
	float3 normal = normalize( mul( nor, i_TBN ) );
	
	float3 finColor = 0;

	[unroll]
	for( int light = 0; light < LIGHTS_COUNT; ++light )
	{
		float3 toLight = Lights[ light ].pos.xyz - i_posW.xyz;
		float lightDist = 1.f / length( toLight );
		toLight *= lightDist;
		float lightDistSquareInv = saturate( Lights[ light ].power * (lightDist * lightDist) );
	
		float3 v   = reflect( -toLight, nor );
		float spec = pow( saturate( dot( v, toEye ) ), 16.f );
		spec *= lightDistSquareInv;
		finColor += spec * Lights[ light ].col;
	}
	
	finColor *= SpecularMaterial.rgb;
	
	return float4( finColor + DiffuseMaterial.rgb, DiffuseMaterial.a );
}