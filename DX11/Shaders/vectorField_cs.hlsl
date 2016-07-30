cbuffer FrameData : register(b13)
{
	float3 EyePos : packoffset(c0);
	float DT : packoffset(c0.w);
	float4x4 VP : packoffset(c1);
	float3 XVec : packoffset(c5);
	float3 YVec : packoffset(c6);
	float3 ZVec : packoffset(c7);
}

cbuffer ConstBuffer : register(b0)
{
	uint StartParticle : packoffset(c0);
	float3 VectorFieldStrength : packoffset(c0.y);
	float4x4 VectorFieldWorldInverse : packoffset(c1);
	float4x4 VectorFieldWorld : packoffset(c5);
	float3 VectorFieldDirectionAddition : packoffset(c9);
	float VectorFieldStrictness : packoffset(c9.w);
}

/*  don't forget to change the same struct in vs.hlsl  */
struct Particle
{
	float3 position;
	float _padding0;
	float4 color;
	float2 size_or_texcoord;
	float2 _padding1;
};

RWStructuredBuffer < Particle > DataToProc : register(u0);

struct ParticleFrame
{
	float4x4 trans;
	float2 particleScale;
	float _padding0;
	float _padding1;
};

RWStructuredBuffer < ParticleFrame > DataToProcFrame : register(u1);

struct SVectorFieldData
{
	float3 velocity;
	float drag;
	float3 velocityMults;
	float _padding0;
};

RWStructuredBuffer < SVectorFieldData > VectorFieldData : register(u2);

Texture3D VectorFieldTexture : register(t0);
SamplerState VectorFieldSamplerState : register(s0);

inline void ProcessParticle( uint index, uint localIndex )
{
	float3 worldPos = DataToProc[ index ].position + DataToProcFrame[ index ].trans[3].xyz;

	float3 localPos = mul( float4( worldPos, 1 ), VectorFieldWorldInverse ).xyz;

	if( localPos.x >= 0.0f && localPos.x <= 1.0f )
	{
		if( localPos.y >= 0.0f && localPos.y <= 1.0f )
		{
			if( localPos.z >= 0.0f && localPos.z <= 1.0f )
			{
				float4 sampled = VectorFieldTexture.SampleLevel( VectorFieldSamplerState, localPos, 0.0f );

				float3 vec = normalize( mul( float4( sampled.xyz + VectorFieldDirectionAddition, 1 ), VectorFieldWorld ).xyz );

				float force = sampled.a;

				VectorFieldData[ localIndex ].velocity += vec * (DT * force * VectorFieldStrength);

				DataToProcFrame[ index ].trans[3].xyz += vec * (VectorFieldStrictness * DT);
			}
		}
	}
				
	VectorFieldData[ localIndex ].velocity -= VectorFieldData[ localIndex ].velocity * (DT * VectorFieldData[ localIndex ].drag);

	DataToProcFrame[ index ].trans[3].xyz += VectorFieldData[ localIndex ].velocity * VectorFieldData[ localIndex ].velocityMults;
}

[numthreads(64, 1, 1)]
void Main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	uint threadIndex = dispatchThreadID.x;

    ProcessParticle( threadIndex + StartParticle, threadIndex );
}