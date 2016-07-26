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

struct SVsOut
{
	float4 posH : SV_Position;
	float2 texCoord : TEXCOORD;
};

SVsOut VS( float4 pos : POSITION, float2 texcoord : TEXCOORD, float2 ppos : LOCATION, float radius : RADIUS )
{
	SVsOut vertex;

    float2 newpos = pos * radius;
    newpos += ppos;

	vertex.posH = mul( float4( newpos, 0.f, 1.f ), P );
	vertex.texCoord = texcoord;
	
	return vertex;
}