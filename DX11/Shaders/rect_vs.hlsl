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

SVsOut VS( float4 pos : POSITION, float2 texcoord : TEXCOORD, float4 transform : TRANSFORM, float2 location : LOCATION )
{
	SVsOut vertex;

    float2 newpos = float2( dot( pos.xy, transform.xz ), dot( pos.xy, transform.yw ) );
    newpos += location;
    newpos.x *= PIXEL_SIZE_X;
    newpos.y *= PIXEL_SIZE_Y;

	vertex.posH = float4( newpos, 0.f, 1.f );
	vertex.texCoord = texcoord;
	
	return vertex;
}