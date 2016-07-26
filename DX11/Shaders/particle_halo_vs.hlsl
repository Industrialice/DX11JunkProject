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
	nointerpolation float4 color : COLOR;
	float2 texCoord : TEXCOORD;
};

Buffer < float > tMask : register(t0);

static const float2 Texcoords[ 4 ] =
{
	float2( 0.f, 1.f ),
	float2( 0.f, 0.f ),
	float2( 1.f, 0.f ),
	float2( 1.f, 1.f )
};

SVsOut VS( uint vertexId : SV_VertexID )
{
	SVsOut vertex;
	uint index = (3 + 4 + 1) * (vertexId / 4);
	uint id = vertexId % 4;
	float3 start_pos = float3( tMask[ index + 0 ], tMask[ index + 1 ], tMask[ index + 2 ] );
	float start_size = tMask[ index + 7 ];
	
	float3 up = normalize( YVec );
	float3 look = normalize( start_pos - EyePos );
	float3 right = cross( up, look );
	up = cross( look, right );
	
	float4x4 o_w = { right.x, right.y, right.z, 0.f,
				     up.x, up.y, up.z, 0.f,
					 look.x, look.y, look.z, 0.f,
					 start_pos.x, start_pos.y, start_pos.z, 1.f };
					 
	//o_w = transpose( o_w );
	//o_w *= VP;
	
	float2 o_pos;
	if( id == 0 )
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
	vertex.color = float4( tMask[ index + 3 ], tMask[ index + 4 ], tMask[ index + 5 ], tMask[ index + 6 ] );
	vertex.texCoord = Texcoords[ id ];
	
	return vertex;
}