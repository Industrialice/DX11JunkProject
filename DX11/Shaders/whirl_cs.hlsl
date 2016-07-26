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
	uint startParticle;
}

/*  don't forget to change the same struct in vs.hlsl  */
struct Particle
{
	float3 position;
	float currentSize;
	float4 color;
};

RWStructuredBuffer < Particle > DataToProc : register(u0);

struct ParticleFrame
{
	float4x4 trans;
};

RWStructuredBuffer < ParticleFrame > DataToProcFrame : register(u1);

struct SWhirlData
{
	float3 rotVec;
	float padding0;
	float3 rotSpeeds;
	float padding1;
	float3 curRot;
	float padding2;
};

RWStructuredBuffer < SWhirlData > WhirlData : register(u2);

inline void AddMatrix( inout ParticleFrame target, float3 rotation, float3 origin )
{
	float cx, sx;
	sincos( rotation.x, sx, cx );

	float cy, sy;
	sincos( rotation.y, sy, cy );

	float cz, sz;
	sincos( rotation.z, sz, cz );

	float4x4 curTrans;

	curTrans[0][0] = cy * cz;
	curTrans[0][1] = cy * sz;
	curTrans[0][2] = -sy;
	curTrans[0][3] = 0;

	curTrans[1][0] = sx * sy * cz - cx * sz;
	curTrans[1][1] = sx * sy * sz + cx * cz;
	curTrans[1][2] = sx * cy;
	curTrans[1][3] = 0;

	curTrans[2][0] = cx * sy * cz + sx * sz;
	curTrans[2][1] = cx * sy * sz - sx * cz;
	curTrans[2][2] = cx * cy;
	curTrans[2][3] = 0;

	curTrans[3][0] = 0;
	curTrans[3][1] = 0;
	curTrans[3][2] = 0;
	curTrans[3][3] = 1;

	target.trans = mul( curTrans, target.trans );

	origin = mul( float4( origin, 1 ), target.trans );

	target.trans[3].xyz = origin;
}

inline void ProcessParticle( uint index, uint localIndex )
{
	AddMatrix( DataToProcFrame[ index ], WhirlData[ localIndex ].curRot, WhirlData[ localIndex ].rotVec );
	WhirlData[ localIndex ].curRot += WhirlData[ localIndex ].rotSpeeds * DT;
}

[numthreads(64, 1, 1)]
void Main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	uint threadIndex = dispatchThreadID.x;

    ProcessParticle( threadIndex + startParticle, threadIndex );
}