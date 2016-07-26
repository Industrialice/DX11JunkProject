cbuffer FrameData : register(b13)
{
	float3 EyePos : packoffset(c0);
	float DT : packoffset(c0.w);
	float4x4 VP : packoffset(c1);
}

struct SVsOut
{
	float3 pos : POSITION0;
	float3 speed : SPEED;
	float2 size : SIZE;
};

struct SGSOut
{
	float3 pos : POSITION0;
	float3 speed : SPEED;
	float2 size : SIZE;
};

[maxvertexcount(1)]
void GS( point SVsOut vert_in[ 1 ], uint primID : SV_PrimitiveID, inout PointStream < SGSOut > vert_out )
{
	SGSOut v;
	v.pos = vert_in[ 0 ].pos + vert_in[ 0 ].speed * DT;
	v.speed = float3( vert_in[ 0 ].speed.x, vert_in[ 0 ].speed.y - 9.8f * DT, vert_in[ 0 ].speed.z );
	v.size = vert_in[ 0 ].size;
	vert_out.Append( v );
	
	/*if( primID < 4095 )
	{
		v.pos.x = 0;
		vert_out.Append( v );
	}*/
}