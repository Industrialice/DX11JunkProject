struct SVsOut
{
	float3 pos : POSITION;
	float4 speed : SPEEDSIZE;
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
	vertex.speed = i_speednsize;
    vertex.speed.y -= 9.8f * DT;
    const float range = 3000.f;
    if( vertex.pos.x > range )
    {
        vertex.speed.x -= 10.f;
    }
    else if( vertex.pos.x < -range )
    {
        vertex.speed.x += 10.f;
    }
    if( vertex.pos.y < -range * 5 )
    {
        vertex.speed.y = abs( vertex.speed.y );
    }
    if( vertex.pos.z > range )
    {
        vertex.speed.z -= 10.f;
    }
    else if( vertex.pos.z < -range )
    {
        vertex.speed.z += 10.f;
    }
	return vertex;
}