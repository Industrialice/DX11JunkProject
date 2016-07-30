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

/*  don't forget to change the same struct in vs.hlsl  */
struct Particle
{
	float3 position;
	float _padding0;
	float4 color;
	float2 size_or_texcoord;
	float2 _padding1;
};

StructuredBuffer < Particle > ParticleBuffer : register(t0);

struct ParticleFrame
{
	float4x4 trans;
	float2 particleScale;
	float _padding0;
	float _padding1;
};

StructuredBuffer < ParticleFrame > ParticleBufferFrame : register(t1);

SVsOut VS( uint vertexId : SV_VertexID )
{
	SVsOut outputVertex;
	
	uint index = vertexId / 3;
    uint vertex = vertexId % 3;

    float2 tex = float2( vertex >> 1, vertex & 1 );
    float2 pos = float2( -0.5f, -0.5f ) + tex;
	
	float2 size = ParticleBuffer[ index ].size_or_texcoord * ParticleBufferFrame[ index ].particleScale;
	float3 loc = ParticleBuffer[ index ].position + ParticleBufferFrame[ index ].trans[3].xyz;
	float3 up = YVec;
	float3 toCamera = normalize( EyePos - loc );
	float3 right = cross( up, toCamera );
    up = cross( toCamera, right );
	
	float4x3 o_w = float4x3( right.x, right.y, right.z,
				             up.x, up.y, up.z,
				             toCamera.x, toCamera.y, toCamera.z,
				             loc.x, loc.y, loc.z );
							 
	float3 posW = mul( float4( pos * size, 0, 1 ), o_w );
	
	outputVertex.position = mul( float4( posW, 1 ), VP );
    outputVertex.texCoord = tex;
    outputVertex.color = ParticleBuffer[ index ].color.rgb * ParticleBuffer[ index ].color.a;
	
	return outputVertex;
}