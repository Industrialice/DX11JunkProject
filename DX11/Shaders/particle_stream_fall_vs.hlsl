struct SVsOut
{
	float3 pos : POSITION;
	float3 startPos : STARTPOSITION;
    float2 lifeTime : LIFETIME;
};

cbuffer Data : register(b0)
{
    float4 BoxMin : packoffset(c15);
    float4 BoxMax : packoffset(c16);
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

SVsOut VS( float3 i_pos : POSITION, float3 startPos : STARTPOSITION, float2 lifeTime : LIFETIME, float4 i_speednsize : SPEEDSIZE )
{
	SVsOut vertex;

    if( lifeTime.x >= lifeTime.y )
    {
        i_pos = startPos;
        lifeTime.x = 0;
    }
    else
    {
        i_pos += i_speednsize.xyz * DT;
        lifeTime.x += DT;
    }

    vertex.pos = i_pos;
    vertex.startPos = startPos;
    vertex.lifeTime = lifeTime;

	return vertex;
}