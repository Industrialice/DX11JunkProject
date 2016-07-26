float4 PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD, nointerpolation float4 i_color : COLOR ) : SV_Target
{
	float alpha = distance( i_tex, float2( 0.5f, 0.5f ) );
    alpha = saturate( alpha * 2 );
	/*if( alpha > 0.5f )
	{
		discard();
	}*/
	float3 color = i_color.rgb * i_color.a;
	float invDistLight = 1.f - alpha;
    invDistLight *= invDistLight * invDistLight;

    return float4( color * invDistLight * 0.5, 1.f );
}