struct PS_OUTPUT
{
    float4 Color0: SV_Target0;
    float4 Color1: SV_Target1;
};

PS_OUTPUT PS( float4 posH : SV_Position, float4 i_color : COLOR, float2 i_texCoord : TEXCOORD ) : SV_Target
{
	PS_OUTPUT output;
	
	float2 texCoord = i_texCoord;
	float alpha = distance( texCoord, float2( 0.5f, 0.5f ) );
	float sum = alpha > 0.25f ? 0.f : 1.f;
	sum *= i_color.a;
	output.Color0 = float4( sum, sum, sum, 1 );
	
	texCoord = i_texCoord;
	alpha = distance( texCoord, float2( 0.5f, 0.5f ) );
	sum = alpha > 0.5f ? 0.f : 1.f;
	sum *= i_color.a;
	float3 color = i_color.rgb * sum;
	output.Color1 = float4( color, 1 );
	
	return output;
}