float Gimme( int index )
{
	const float scale = 0.2f;
	static const float consts_basic[ 13 ] = { 0.7f, 0.65f, 0.5f, 0.4f, 0.325f, 0.26f, 0.21f, 0.175f, 0.15f, 0.135f, 0.12f, 0.095f, 0.075f };
	static const float consts_using[ 24 ] =
	{
		consts_basic[ 12 ],
		consts_basic[ 11 ],
		consts_basic[ 10 ],
		consts_basic[ 9 ],
		consts_basic[ 8 ],
		consts_basic[ 7 ],
		consts_basic[ 6 ],
		consts_basic[ 5 ],
		consts_basic[ 4 ],
		consts_basic[ 3 ],
		consts_basic[ 2 ],
		consts_basic[ 1 ],
		consts_basic[ 0 ],
		consts_basic[ 1 ],
		consts_basic[ 2 ],
		consts_basic[ 3 ],
		consts_basic[ 4 ],
		consts_basic[ 5 ],
		consts_basic[ 6 ],
		consts_basic[ 7 ],
		consts_basic[ 8 ],
		consts_basic[ 9 ],
		consts_basic[ 10 ],
		consts_basic[ 11 ],
		//consts_basic[ 12 ]
	};
	return consts_using[ index ] * scale;
}

void VS( uint id : SV_VertexID, out float4 o_pos : SV_Position, out float4 datas[ 6 ] : DATAS, out float texx : TEXCOORD )
{
	float2 tex = float2( (id << 1) & 2, id & 2 );
	o_pos = float4( tex * float2( 2, -2 ) + float2( -1, 1 ), 0, 1 );
	texx = tex.x;
	
	const float pixel = PIXEL_SIZE_X * 4;
	float offset = tex.y - 12 * pixel;
	
	float2 imm;
	
	[loop]
	for( uint pixelIndex = 0; ; pixelIndex += 2 )
	{		
		float w0 = Gimme( pixelIndex );
		float w1 = Gimme( pixelIndex + 1 );
		
		float o0 = offset;
		float o1 = offset + pixel;
		float w01 = w0 + w1;
		float o12 = (w0 * o0 + w1 * o1) / w01;
		
		if( pixelIndex & 2 )
		{
			datas[ pixelIndex / 4 ] = float4( imm, o12, w01 );
			if( pixelIndex == 22 )
			{
				break;
			}
		}
		else
		{
			imm = float2( o12, w01 );
		}
		
		offset += pixel * 2;
	}
}