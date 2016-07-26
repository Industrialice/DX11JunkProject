float4 PS( float4 posH : SV_Position, float4 i_color : COLOR, float2 i_texCoord : TEXCOORD ) : SV_Target
{
	float2 texCoord = i_texCoord;
	float alpha = distance( texCoord, float2( 0.5f, 0.5f ) );
	/*if( alpha > 0.5f )
	{
		discard();
	}*/
	float invDistWhite = saturate( 0.25f - alpha ) * 1.5f;
	float invDistLight = saturate( 0.5f - alpha );
	float3 color = i_color.rgb * i_color.a * invDistLight;
	color += float3( invDistWhite, invDistWhite, invDistWhite );
	return float4( color, 1.f );
}