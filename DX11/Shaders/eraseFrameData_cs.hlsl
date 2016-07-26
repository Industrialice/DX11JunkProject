struct ParticleFrame
{
	float4x4 trans;
};

RWStructuredBuffer < ParticleFrame > DataToProcFrame : register(u0);

inline void ProcessParticle( uint index )
{
    DataToProcFrame[ index ].trans = 
		float4x4( 1, 0, 0, 0,
				  0, 1, 0, 0,
			      0, 0, 1, 0,
			      0, 0, 0, 1 );

	/*DataToProcFrame[ index ].trans[0][0] = 1;
	DataToProcFrame[ index ].trans[0][1] = 0;
	DataToProcFrame[ index ].trans[0][2] = 0;
	DataToProcFrame[ index ].trans[0][3] = 0;

	DataToProcFrame[ index ].trans[1][0] = 0;
	DataToProcFrame[ index ].trans[1][1] = 1;
	DataToProcFrame[ index ].trans[1][2] = 0;
	DataToProcFrame[ index ].trans[1][3] = 0;

	DataToProcFrame[ index ].trans[2][0] = 0;
	DataToProcFrame[ index ].trans[2][1] = 0;
	DataToProcFrame[ index ].trans[2][2] = 1;
	DataToProcFrame[ index ].trans[2][3] = 0;

	DataToProcFrame[ index ].trans[3][0] = 0;
	DataToProcFrame[ index ].trans[3][1] = 0;
	DataToProcFrame[ index ].trans[3][2] = 0;
	DataToProcFrame[ index ].trans[3][3] = 1;*/
}

[numthreads(64, 1, 1)]
void Main( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	uint threadIndex = dispatchThreadID.x;

    ProcessParticle( threadIndex );
}