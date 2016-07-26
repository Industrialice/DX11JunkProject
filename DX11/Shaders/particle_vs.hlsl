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
	float2 texCoord : TEXCOORD0;
};

Buffer < float > tMask : register(t0);

static const float2 Texcoords[ 6 ] =
{
	float2( 0.f, 1.f ),
	float2( 0.f, 0.f ),
	float2( 1.f, 0.f ),
	float2( 1.f, 1.f ),
	float2( 0.f, 1.f ),
	float2( 1.f, 0.f )
};

SVsOut VS( uint vertexId : SV_VertexID )
{
	SVsOut vertex;
	uint index = (3 + 3 + 2) * (vertexId / 6);
	uint id = vertexId % 6;
	float3 start_pos = float3( tMask[ index + 0 ], tMask[ index + 1 ], tMask[ index + 2 ] );
	float2 start_size = float2( tMask[ index + 6 ], tMask[ index + 7 ] );
	
	float3 up = normalize( YVec );
	float3 look = normalize( EyePos - start_pos );
	float3 right = cross( up, look );
	up = cross( look, right );
	
	float4x4 o_w = { right.x, right.y, right.z, 0.f,
				     up.x, up.y, up.z, 0.f,
					 look.x, look.y, look.z, 0.f,
					 start_pos.x, start_pos.y, start_pos.z, 1.f };
					 
	//o_w = transpose( o_w );
	//o_w *= VP;
	
	float2 o_pos;
	if( id == 0 || id == 4 )
	{
		o_pos = float2( -0.5f, -0.5f );
	}
	else if( id == 1 )
	{
		o_pos = float2( -0.5f, 0.5f );
	}
	else if( id == 3 )
	{
		o_pos = float2( 0.5f, -0.5f );
	}
	else  //  if( id == 2 || id == 5 )
	{
		o_pos = float2( 0.5f, 0.5f );
	}
	
	float4 o_wpos = mul( float4( o_pos * start_size, 0.f, 1.f ), o_w );
	vertex.posH = mul( o_wpos, VP );
	vertex.texCoord = Texcoords[ id ];
	
	return vertex;
}