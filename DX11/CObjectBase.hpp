#ifndef __C_OBJECT_BASE_HPP__
#define __C_OBJECT_BASE_HPP__

#include "ShadersManager.hpp"
#include "CPointLight.hpp"
#include "StatesManagers.hpp"
#include "Geometry.hpp"

struct STex : CharPOD
{
    vec2 o_texOffset;
    vec2 o_texMult;
    vec2 o_rotCenter;
    f32 rotAngleRads;
    ID3D11ShaderResourceView *i_tex;
    ID3D11SamplerState *i_sampler;

	STex()
	{}

    STex( ID3D11ShaderResourceView *i_texIn )
    {
        i_tex = i_texIn;
        o_texMult = vec2( 1 );
        o_texOffset = vec2( 0 );
    }

    STex( ID3D11ShaderResourceView *i_texIn, const D3D11_SAMPLER_DESC *po_samplerDesc, const vec2 &o_texOffsetIn, const vec2 &o_texMultIn, const vec2 &o_rotCenterIn, f32 rotAngleRadsIn )
    {
        i_tex = i_texIn;
        i_sampler = SamplersManager::GetState( po_samplerDesc );
        o_texMult = o_texMultIn;
        o_texOffset = o_texOffsetIn;
        o_rotCenter = o_rotCenterIn;
        rotAngleRads = rotAngleRadsIn;
    }

    STex( STex &&source ) = default;
    STex &operator = ( STex &&source ) = default;
};

struct SMaterial : CharMovable
{
    SGeometry *po_geo;
    SGeometrySlice o_geoSlice;
    ui32 instanceCount;
    ui32 startInstance;
    sdrhdl shader;
    f128color o_specColor;
    f128color o_difColor;
    f96color o_ambColor;
    f32 externalLightPower;
    CVec < STex, void > o_textures;
    bln is_inFrustum;
    bln is_geoShaderDefined;
    bln is_enabled;
    ID3D11BlendState *i_blend;
    ID3D11RasterizerState *i_rasterizerState;
    RStates::rstate_t rstates;
    ID3D11InputLayout *i_lo;
    DBGCODE( bln is_defined; );

    SMaterial()
    {
        DBGCODE( is_defined = false; );
    }

    SMaterial( SGeometry *po_geoIn, const SGeometrySlice &o_geoSliceIn, ui32 instanceCountIn, ui32 startInstanceIn, CVec < STex, void > &&o_texturesIn, sdrhdl shaderIn, const D3D11_BLEND_DESC *cpo_blendDescIn, const D3D11_RASTERIZER_DESC *cpo_rasterizerDescIn, const f128color &o_difColorIn, const f128color &o_specColorIn, const f96color &o_ambColorIn, f32 externalLightPowerIn, RStates::rstate_t rstatesIn )
    {
        DBGCODE( is_defined = true; );
        ASSUME( po_geoIn );
        po_geo = po_geoIn;
        o_geoSlice = o_geoSliceIn;
        instanceCount = instanceCountIn;
        startInstance = startInstanceIn;
        shader = shaderIn;
        i_blend = BlendStatesManager::GetState( cpo_blendDescIn );
        i_rasterizerState = RasterizerStatesManager::GetState( cpo_rasterizerDescIn );
        o_specColor = o_specColorIn;
        o_difColor = o_difColorIn;
        o_ambColor = o_ambColorIn;
        externalLightPower = externalLightPowerIn;
        o_textures = std::move( o_texturesIn );
        rstates = rstatesIn;
        is_inFrustum = false;
        is_enabled = true;
        is_geoShaderDefined = ShadersManager::TryToBlend( po_geo->description, shader, &i_lo );
        if( !is_geoShaderDefined )
        {
            //SOFTBREAK;
        }
    }

    SMaterial( ui32 instanceCountIn, ui32 startInstanceIn, CVec < STex, void > &&o_texturesIn, sdrhdl shaderIn, const D3D11_BLEND_DESC *cpo_blendDescIn, const D3D11_RASTERIZER_DESC *cpo_rasterizerDescIn, const f128color &o_difColorIn, const f128color &o_specColorIn, const f96color &o_ambColorIn, f32 externalLightPowerIn, RStates::rstate_t rstatesIn )
    {
        DBGCODE( is_defined = true; );
        po_geo = 0;
        o_geoSlice = SGeometrySlice();
        instanceCount = instanceCountIn;
        startInstance = startInstanceIn;
        shader = shaderIn;
        i_blend = BlendStatesManager::GetState( cpo_blendDescIn );
        i_rasterizerState = RasterizerStatesManager::GetState( cpo_rasterizerDescIn );
        o_specColor = o_specColorIn;
        o_difColor = o_difColorIn;
        o_ambColor = o_ambColorIn;
        externalLightPower = externalLightPowerIn;
        o_textures = std::move( o_texturesIn );
        rstates = rstatesIn;
        is_inFrustum = false;
        is_enabled = true;
        is_geoShaderDefined = false;
    }

    SMaterial( SMaterial &&source ) = default;
    SMaterial( const SMaterial &source ) = delete;
    SMaterial &operator = ( SMaterial &&source ) = default;
};

struct SShaderData
{
    CONSTS( targetShader_t, vertex = BIT( 0 ), pixel = BIT( 1 ), geometry = BIT( 2 ), none = 0 );
    void *data;
    uiw dataSize;
    targetShader_t targetShaders;
};
CONSTS_OPS( SShaderData::targetShader_t );

class DX11_EXPORT CObjectBase : CharMovable
{
protected:
    vec3 _o_pos;
    vec3 _o_rot;  //  radians
    vec3 _o_size;
    m4x3 _o_w, _o_wrot;
    CVec < SMaterial, void > _o_mats;
    CVec < const CPointLight * > _o_lights;
    bln _is_lightable;
    bln _is_visible;
    bln _is_inFrustum;
    bln _is_glowing;
    bln _is_transparenting;
    SShaderData _o_additionalShaderData;

public:
    virtual ~CObjectBase();
    CObjectBase( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats );
	CObjectBase() = delete;
	CObjectBase &operator = ( const CObjectBase & ) = delete;
	CObjectBase( const CObjectBase & ) = delete;
    void PosSet( const vec3 &o_pos );
    const vec3 &PosGet() const;
    void SizeSet( const vec3 &o_size );
    const vec3 &SizeGet() const;
    void RotRadSet( const vec3 &o_rot );
    const vec3 &RotRadGet() const;
    void RotSetDeg( const vec3 &o_rot );
    vec3 RotGetDeg() const;
    void MaterialSet( ui8 index, SMaterial &&o_matIn );
    SMaterial &MaterialGet( ui8 index );
    void RemoveLights();
    void AttachLight( const CPointLight *cpo_light );
    bln IsLightableGet() const;
    void IsLightableSet( bln is_lightable );
    bln IsVisibleGet() const;
    void IsVisibleSet( bln is_visible );
    bln IsInFrustumGet() const;
    bln IsGlowingGet() const;
    bln IsTransparentingGet() const;
    void SetAdditionalShaderData( void *data, uiw size, SShaderData::targetShader_t targetShaders );  //  can be null to remove current
    virtual void FlushBuffers();
    virtual void Update() = 0;
    virtual void Draw( bln is_stepTwo ) = 0;
};

#endif __C_OBJECT_BASE_HPP__