#define LIGHTS_COUNT 2

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
	float4x4 P : packoffset(c8);
}

Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);
Texture2D NMap : register(t1);
SamplerState NMapSampler : register(s1);
Texture2D SMap : register(t2);
SamplerState SMapSampler : register(s2);

float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD0, float3 i_posW : POSITION, float3x3 i_TBN : TBN ) : SV_Target
{
	float3 toEye = normalize( EyePos.xyz - i_posW );
	
	float3 texNormal = NMap.Sample( NMapSampler, i_tex ).xyz * 2.f - 1.f;
	float4 diffuse = CMap.Sample( CMapSampler, i_tex ) * DiffuseMaterial;
	float3 specCol = SMap.Sample( SMapSampler, i_tex ).rgb * SpecularMaterial.rgb;

    //return diffuse;
	
	float3 normal = normalize( mul( texNormal, i_TBN ) );
	
	float3 finDiffuse = 0;
	float finSpecular = 0;

	[unroll]
	for( int light = 0; light < LIGHTS_COUNT; ++light )
	{
		float3 toLight = Lights[ light ].pos.xyz - i_posW;
		float lightDist = 1.f / length( toLight );
		toLight *= lightDist;
		//float lightDistSquareInv = saturate( Lights[ light ].power * (lightDist * lightDist) );
        float lightDistSquareInv = Lights[ light ].power * lightDist;
		
		float mult = saturate( dot( toLight, normal ) );
		mult *= lightDistSquareInv;
		finDiffuse += Lights[ light ].col * mult;
	
		float3 v   = reflect( -toLight, normal );
		float spec = pow( saturate( dot( v, toEye ) ), 16.f );
		spec *= lightDistSquareInv;
		finSpecular += spec;
	}
	
	float3 finColor = finDiffuse * diffuse.rgb + finSpecular * specCol;
	
	return float4( finColor + AmbientMaterial.rgb + diffuse.rgb * ExternalLightPower, diffuse.a );
}