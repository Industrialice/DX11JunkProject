struct SOutput
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

SOutput PS( float4 posH : SV_Position, float2 i_tex : TEXCOORD, nointerpolation float4 i_color : COLOR ) : SV_Target
{
	float alpha = distance( i_tex, float2( 0.5f, 0.5f ) );
	/*if( alpha > 0.5f )
	{
		discard();
	}*/
	float invDistWhite = saturate( 0.25f - alpha ) * 1.5f;
	float invDistLight = saturate( 0.5f - alpha );
	float3 color = i_color.rgb * i_color.a * invDistLight;
	color += float3( invDistWhite, invDistWhite, invDistWhite );

    float4 outColor = float4( color, 1.f );
    SOutput outColors = { outColor, outColor };
    return outColors;
}