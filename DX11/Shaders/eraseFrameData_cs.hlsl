struct ParticleFrame
{
	float4x4 trans;
	float2 particleScale;
	float _padding0;
	float _padding1;
};

RWStructuredBuffer < ParticleFrame > DataToProcFrame : register(u0);

inline void ProcessParticle( uint index )
{
    DataToProcFrame[ index ].trans = 
		float4x4( 1, 0, 0, 0,
				  0, 1, 0, 0,
			      0, 0, 1, 0,
			      0, 0, 0, 1 );

	DataToProcFrame[ index ].particleScale = float2( 1, 1 );
}

[numthreads(64, 1, 1)]
void Main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	uint threadIndex = dispatchThreadID.x;

    ProcessParticle( threadIndex );
}