Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);
Texture2D GMap : register(t1);
SamplerState GMapSampler : register(s1);

struct SOutput
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

SOutput PS( float4 posH : SV_Position, float4 o_texCoord_age_die : TEXCOORD_AGE_DIE, float4 pcolor : PCOLOR ) : SV_Target
{
    if( o_texCoord_age_die.x > 1.f || o_texCoord_age_die.y > 1.f )
    {
        discard;
    }

	float4 dif = CMap.Sample( CMapSampler, o_texCoord_age_die.xy );
    float4 gmap = GMap.Sample( GMapSampler, o_texCoord_age_die.xy );

    if( o_texCoord_age_die.z < 1 )
    {
        dif.a *= o_texCoord_age_die.z;
        gmap.a *= o_texCoord_age_die.z;
    }
    else if( o_texCoord_age_die.w - o_texCoord_age_die.z < 1 )
    {
        dif.a *= o_texCoord_age_die.w - o_texCoord_age_die.z;
        gmap.a *= o_texCoord_age_die.w - o_texCoord_age_die.z;
    }
    
    dif.rgb *= dif.a;
    dif.a = 1.f;
    gmap.rgb *= gmap.a;
    gmap.a = 1.f;
	
	SOutput op;
	op.color0 = dif * pcolor;
	op.color1 = gmap * pcolor;
	return op;
}