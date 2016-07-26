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

void VS( float3 pos : LOCATION, float2 ampSpeedRange : AMPPARAMS, out float3 o_pos : LOCATION, out float2 o_ampSpeedRange : AMPPARAMS )
{
    if( ampSpeedRange.x < 0.f )
    {
        if( pos.y <= -ampSpeedRange.y )
        {
            float dif = ampSpeedRange.y - abs( pos.y );
            pos.y += dif * 2.f;
            ampSpeedRange.x = -ampSpeedRange.x;
        }
    }
    else if( pos.y >= ampSpeedRange.y )
    {
        float dif = pos.y - ampSpeedRange.y;
        pos.y -= dif * 2.f;
        ampSpeedRange.x = -ampSpeedRange.x;
    }
    pos.y += ampSpeedRange.x * DT;

    o_pos = pos;
    o_ampSpeedRange = ampSpeedRange;
}