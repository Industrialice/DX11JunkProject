#ifndef __BLOOM_HPP__
#define __BLOOM_HPP__

#include "ShadersManager.hpp"

class DX11_EXPORT CBloom
{
public:
    typedef ui32 bloomQuality_t;
    struct BloomQuality
    {
        static const bloomQuality_t disabled = 0;
        static const bloomQuality_t low = 1;
        static const bloomQuality_t medium = 2;
        static const bloomQuality_t high = 3;
    };    

private:
    struct SRT
    {
        COMUniquePtr< ID3D11RenderTargetView > i_rtv;
        COMUniquePtr< ID3D11ShaderResourceView > i_srv;
    };

    struct SBloom
    {
        SRT horizontalTex;
        SRT verticalTex;
        sdrhdl horizontalShader[ 3 ];
        sdrhdl verticalShader[ 3 ];
        bloomQuality_t quality;
        f32 amount;
    } _o_nearBloom, _o_mediumBloom, _o_wideBloom;

	D3D11_VIEWPORT _vps[ 3 ] {};
	SRT _glowmapTexs[ 3 ] {};
    RStates::rstate_t _rstatesCur = RStates::nothing;
    RStates::rstate_t _rstatesLast = RStates::nothing;
    bln _is_bloomEnabled = false;
    bln _is_depthEnabled = false;
    bln _is_targetEnabled = false;
    bln _is_mediumRequired = false, _is_lowRequired = false;
    bln _is_verticalEnabled = false;
    bln _is_bloomActive = false;  //  if enabled and at least one type is not disabled
    ui32 _width = 0, _height = 0;
    COMUniquePtr< ID3D11RenderTargetView > _i_rtv;
    COMUniquePtr< ID3D11ShaderResourceView > _i_srv;
    COMUniquePtr< ID3D11DepthStencilView > _i_dsv;

public:
    ~CBloom();
    CBloom() {  /*  void  */  }
    CBloom( bloomQuality_t nearBloomQuality, bloomQuality_t mediumBloomQuality, bloomQuality_t wideBloomQuality, ui32 width, ui32 height, ID3D11RenderTargetView *i_rtv, ID3D11ShaderResourceView *i_srv, ID3D11DepthStencilView *i_dsv );
    CBloom( CBloom && ) = default;
    CBloom( const CBloom & ) = default;
    CBloom &operator = ( const CBloom & ) = default;
    CBloom &operator = ( CBloom && ) = default;

    void RenderingStatesSet( RStates::rstate_t states, bln is_cache = true, bln is_makeCurrentAsLast = true );
    void RenderingStatesRestore();
    RStates::rstate_t RenderingStatesGet() const;

    void IsTargetEnabledSet( bln is_enabled );
    bln IsTargetEnabledGet() const;
    void IsEnabledSet( bln is_enabled );
    bln IsEnabledGet() const;
    void IsVerticalEnabledSet( bln is_enabled );
    bln IsVerticalEnabledGet() const;
    void NearQualitySet( bloomQuality_t quality );
    bloomQuality_t NearQualityGet() const;
    void NearAmountSet( f32 amount );
    f32 NearAmountGet() const;
    void MediumQualitySet( bloomQuality_t quality );
    bloomQuality_t MediumQualityGet() const;
    void MediumAmountSet( f32 amount );
    f32 MediumAmountGet() const;
    void WideQualitySet( bloomQuality_t quality );
    bloomQuality_t WideQualityGet() const;
    void WideAmountSet( f32 amount );
    f32 WideAmountGet() const;
    void FlushGlowMap();
	void FlushToRT( ID3D11RenderTargetView *rt );

    struct Private
    {
        static void Initialize();
    };

private:
    static ID3D11SamplerState *_si_samp;
    static ID3D11DepthStencilState *_si_depthTestOnly;
    static ID3D11DepthStencilState *_si_depthWriteOnly;
    static ID3D11BlendState *_si_blend;
    static sdrhdl _s_flatterBloomFinalShader;
    static sdrhdl _s_flatterBloomDoNothingShader;
	static sdrhdl _s_flatterOnlySampleShader;

    void CreateSources( SBloom *po_bloom );
    void CreateRT( SRT *rt, ui32 width, ui32 height );
    void ApplyBlur( SBloom *po_bloom );
    void RenderTo( SRT *sourceRt, SRT *destRt, const D3D11_VIEWPORT *vp, sdrhdl shader );
};

#endif __BLOOM_HPP__