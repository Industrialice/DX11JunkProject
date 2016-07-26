cbuffer ObjData : register(b0)
{
    float4x4 EWM : packoffset(c0);
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

void VS( float2 pos : POSITION, float2 tex : TEXCOORD, float3 loc : LOCATION, float rad : RADIUS, out float4 posT : SV_Position, out float2 o_tex : TEXCOORD )
{	
    loc = mul( float4( loc, 1.f ), EWM );

	float3 up = YVec;
	float3 look = normalize( loc - EyePos );
	float3 right = cross( up, look );
	up = cross( look, right );
	
	float4x3 o_w = float4x3( right.x, right.y, right.z,
				             up.x, up.y, up.z,
				             0, 0, 0,
				             loc.x, loc.y, loc.z );
	
    float3 wpos = mul( float4( pos * rad, 0.f, 1.f ), o_w );
	posT = mul( float4( wpos, 1.f ), VP );
	o_tex = tex;
}