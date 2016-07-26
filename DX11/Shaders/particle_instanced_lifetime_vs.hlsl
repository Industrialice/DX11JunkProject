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
	float4 posH : SV_Position;
	float4 texCoord : TEXCOORD_AGE_DIE;
    float4 color : PCOLOR;
};

SVsOut VS( float4 pos : POSITION, float2 texcoord : TEXCOORD, float3 ppos : PPOSITION, float2 lifeTime : LIFETIME, float4 pspeednsize : PSPEEDSIZE, float4 pcolor : PCOLOR )
{
	SVsOut vertex;

    uint isize = asuint(pspeednsize.w);
    float2 psize;
    psize.x = asfloat((isize & 0xFFFF0000) >> 1);
    psize.y = asfloat((isize & 0xFFFF) << 15);

    if( lifeTime.x < 1 )
    {
        psize.x *= lifeTime.x;
        psize.y *= lifeTime.x;
    }
    else if( lifeTime.y - lifeTime.x < 1 )
    {
        psize.x *= lifeTime.y - lifeTime.x;
        psize.y *= lifeTime.y - lifeTime.x;
    }

	float3 up = YVec;
	float3 look = normalize( ppos.xyz - EyePos );
	float3 right = cross( up, look );
	
	float4x3 o_w = float4x3( right.x, right.y, right.z,
				             up.x, up.y, up.z,
				             0, 0, 0,
				             ppos.x, ppos.y, ppos.z );
	
    float3 wpos = mul( float4( pos * psize, 0.f, 1.f ), o_w );
    
	vertex.posH = mul( float4( wpos, 1.f ), VP );
    vertex.texCoord = float4( texcoord, lifeTime );
    vertex.color = pcolor;
	
	return vertex;
}