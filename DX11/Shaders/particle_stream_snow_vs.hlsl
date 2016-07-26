struct SVsOut
{
	float3 pos : POSITION;
};

cbuffer FrameData : register(b13)
{
	float3 EyePos : packoffset(c0);
	float DT : packoffset(c0.w);
	float4x4 VP : packoffset(c1);
	float3 XVec : packoffset(c5);
	float3 YVec : packoffset(c6);
	float3 ZVec : packoffset(c7);
}

SVsOut VS( float3 i_pos : POSITION, float4 i_speednsize : SPEEDSIZE )
{
	SVsOut vertex;
	vertex.pos = i_pos + i_speednsize.xyz * DT;
    const float range = 2000.f;
    if( vertex.pos.x > range )
    {
        vertex.pos.x = -range;
    }
    else if( vertex.pos.x < -range )
    {
        vertex.pos.x = range;
    }
    if( vertex.pos.y < 0 )
    {
        vertex.pos.y = range;
    }
    if( vertex.pos.z > range )
    {
        vertex.pos.z = -range;
    }
    else if( vertex.pos.z < -range )
    {
        vertex.pos.z = range;
    }
	return vertex;
}