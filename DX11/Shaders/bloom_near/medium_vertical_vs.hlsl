#define DIVYL 15.f
#define DEVIATION 0.4f
#define KERNEL 24

float Gaussian( float x, float deviation )
{
	const float scale = 0.2f;
	return (1.f / sqrt(2.f * 3.141592f * deviation)) * exp(-((x * x) / (2.f * deviation))) * scale;
}

void VS( uint id : SV_VertexID, out float4 o_pos : SV_Position, out float4 datas[ KERNEL / 2 ] : DATAS, out float texx : TEXCOORD )
{
	float2 datas2[ KERNEL ];
	float2 tex = float2( (id << 1) & 2, id & 2 );
	o_pos = float4( tex * float2( 2, -2 ) + float2( -1, 1 ), 0, 1 );
	const float pixel = PIXEL_SIZE_X * 2;
	float offset = tex.y - KERNEL * pixel;
	[loop]
	for( int index = -KERNEL; index < KERNEL; index += 2 )
	{
		float w0 = Gaussian( abs( index ) / DIVYL, DEVIATION );
		float w1 = Gaussian( abs( index + 1 ) / DIVYL, DEVIATION );		
		float o0 = offset;
		float o1 = offset + pixel;
		float w01 = w0 + w1;
		float o01 = (w0 * o0 + w1 * o1) / w01;
		
		datas2[ (index + KERNEL) / 2 ].x = o01;
		datas2[ (index + KERNEL) / 2 ].y = w01;
		
		offset += pixel * 2;
	}
	texx = tex.x;
	datas = (float4[ KERNEL / 2 ])datas2;
}