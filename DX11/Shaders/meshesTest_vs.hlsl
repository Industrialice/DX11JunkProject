cbuffer MeshData : register(b0)
{
	uint Start : packoffset(c0.x);
	float Thickness : packoffset(c0.y);
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

struct SVsOut
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
    float3 color : COLOR;
};

struct ParticleFrame
{
	float4x4 trans;
	float2 particleScale;
	float _padding0;
	float _padding1;
};

StructuredBuffer < ParticleFrame > ParticleBufferFrame : register(t1);

SVsOut VS( uint index : SV_VertexID, float4 position : POSITION, float4 color : COLOR, float2 texCoord : TEXCOORD )
{
	SVsOut outputVertex;

	bool is_add = index % 2 == 1;

	index += Start;

	float3 loc = position.xyz + ParticleBufferFrame[ index ].trans[3].xyz;

	if( is_add )
	{	
		float3 up = YVec;
		float3 toCamera = normalize( EyePos - loc );
		float3 right = cross( up, toCamera );
		up = cross( toCamera, right );
	
		float4x3 o_w = float4x3( right.x, right.y, right.z,
								 up.x, up.y, up.z,
								 toCamera.x, toCamera.y, toCamera.z,
								 loc.x, loc.y, loc.z );
							 
		loc = mul( float4( float3( Thickness, Thickness, 0 ), 1 ), o_w );
	}

	outputVertex.position = mul( float4( loc, 1 ), VP );

    outputVertex.texCoord = texCoord;
    outputVertex.color = color.rgb * color.a;
	
	return outputVertex;
}