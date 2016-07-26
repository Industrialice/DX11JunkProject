#define DIVYL 15.f
#define DEVIATION 1.2f
#define KERNEL 40

float Gaussian( float x, float deviation )
{
	const float scale = 0.085f;
	return (1.f / sqrt(2.f * 3.141592f * deviation)) * exp(-((x * x) / (2.f * deviation))) * scale;
}

void VS( uint id : SV_VertexID, out float4 o_pos : SV_Position, out float4 datas[ KERNEL / 2 ] : DATAS, out float texy : TEXCOORD )
{
	float2 tex = float2( (id << 1) & 2, id & 2 );
	o_pos = float4( tex * float2( 2, -2 ) + float2( -1, 1 ), 0, 1 );
	texy = tex.y;
	
	const float pixel = PIXEL_SIZE_X * 2;
	float offset = tex.x - KERNEL * pixel;
	
	float2 imm;
	float x = -KERNEL;
	[loop]
	for( uint index = 0; index < KERNEL; ++index, x += 2.f )
	{
		float w0 = Gaussian( (x) / DIVYL, DEVIATION );
		float w1 = Gaussian( (x + 1.f) / DIVYL, DEVIATION );		
		float o0 = offset;
		float o1 = offset + pixel;
		float w01 = w0 + w1;
		float o01 = (w0 * o0 + w1 * o1) / w01;
		
		if( index % 2 )
		{
			datas[ index / 2 ] = float4( imm, o01, w01 );
		}
		else
		{
			imm = float2( o01, w01 );
		}
		
		offset += pixel * 2;
	}
}