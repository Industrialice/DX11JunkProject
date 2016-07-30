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
	float3 Position : packoffset(c1);
	float Concentration : packoffset(c1.w);
	float3 Direction : packoffset(c2);
	float Force : packoffset(c2.w);
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

struct SWindData
{
	float3 shiftedPosition;
	float weigthRev;
};

RWStructuredBuffer < SWindData > WindData : register(u2);

inline void ProcessParticle( uint index, uint localIndex )
{
	float3 realPos = DataToProc[ index ].position + DataToProcFrame[ index ].trans[3].xyz;
	
	float3 towards = realPos - Position;

	float dist = length( towards );

	float normDist = 1 / exp( dist );

	float power = saturate( dot( -towards, Direction ) );
	if( power < Concentration * 0.5f )
	{
		power *= power * power;
	}
	else if( power < Concentration )
	{
		power *= power;
	}

	WindData[ localIndex ].shiftedPosition += Direction * Force * normDist * DT * power * WindData[ localIndex ].weigthRev;

	float backLength = length( WindData[ localIndex ].shiftedPosition );

	WindData[ localIndex ].shiftedPosition -= WindData[ localIndex ].shiftedPosition * (backLength * 0.1f * DT);

	DataToProcFrame[ index ].trans[3].xyz += WindData[ localIndex ].shiftedPosition;
}

[numthreads(64, 1, 1)]
void Main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	uint threadIndex = dispatchThreadID.x;

    ProcessParticle( threadIndex + StartParticle, threadIndex );
}