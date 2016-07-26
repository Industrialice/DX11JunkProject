#define LIGHTS_COUNT 1

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
    float3 AmbientMaterial : packoffset(c2);
    float ExternalLightPower : packoffset(c2.w);
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
}

Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0, float3 i_posW : POSITION, float3 i_normal : NORMAL0 ) : SV_Target
{
    float3 toEye = normalize( EyePos.xyz - i_posW );
	
	float4 diffuse = CMap.Sample( CMapSampler, i_tex ) * DiffuseMaterial;
	
	float3 normal = normalize( i_normal );
	
	float3 finDiffuse = 0;

	[unroll]
	for( int light_l2 = 0; light_l2 < LIGHTS_COUNT; ++light_l2 )
	{
		float3 toLight = Lights[ light_l2 ].pos.xyz - i_posW;
		float lightDist = 1.f / length( toLight );
		toLight *= lightDist;
		float lightDistSquareInv = saturate( Lights[ light_l2 ].power * (lightDist * lightDist) );
		
		float mult = saturate( dot( toLight, normal ) );
		mult *= lightDistSquareInv;
		finDiffuse += Lights[ light_l2 ].col * mult;
	}
	
	float3 finColor = finDiffuse * diffuse.rgb;
	
	return float4( finColor + AmbientMaterial.rgb + diffuse.rgb * ExternalLightPower, diffuse.a );
}