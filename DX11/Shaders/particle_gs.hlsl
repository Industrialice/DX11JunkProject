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
	float3 pos : POSITION;
	float3 speed : SPEED;
	float2 size : SIZE;
};

struct SGSOut
{
	float4 posH : SV_Position;
	float2 texCoord : TEXCOORD0;
};

[maxvertexcount(4)]
void GS( point SVsOut vert_in[ 1 ], inout TriangleStream < SGSOut > vert_out )
{
	float3 up = float3( 0.f, 1.f, 0.f );
	float3 look = EyePos - vert_in[ 0 ].pos;
	look.y = 0.f;
	look = normalize( look );
	float3 right = cross( up, look );
	
	float2 size = 0.5f * vert_in[ 0 ].size;
	
	right *= size.x;
	up = float3( 0.f, size.y, 0.f );
	
	SGSOut v;
	v.posH = mul( float4( vert_in[ 0 ].pos + right - up, 1.f ), VP );
	v.texCoord = float2( 0.f, 1.f );
	vert_out.Append( v );
	
	v.posH = mul( float4( vert_in[ 0 ].pos + right + up, 1.f ), VP );
	v.texCoord = float2( 0.f, 0.f );
	vert_out.Append( v );
	
	v.posH = mul( float4( vert_in[ 0 ].pos - right - up, 1.f ), VP );
	v.texCoord = float2( 1.f, 1.f );
	vert_out.Append( v );
	
	v.posH = mul( float4( vert_in[ 0 ].pos - right + up, 1.f ), VP );
	v.texCoord = float2( 1.f, 0.f );
	vert_out.Append( v );
}