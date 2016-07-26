#ifndef __STATES_MANAGERS_HPP__
#define __STATES_MANAGERS_HPP__

namespace SamplersManager
{
    DX11_EXPORT ID3D11SamplerState *GetState( const D3D11_SAMPLER_DESC *cpo_desc );
    DX11_EXPORT D3D11_SAMPLER_DESC GetDesc( ID3D11SamplerState *i_sampler );
    DX11_EXPORT void FreeState( ID3D11SamplerState *i_sampler );
}

namespace BlendStatesManager
{
    DX11_EXPORT ID3D11BlendState *GetState( const D3D11_BLEND_DESC *cpo_desc );
    DX11_EXPORT D3D11_BLEND_DESC GetDesc( ID3D11BlendState *i_state );
    DX11_EXPORT void FreeState( ID3D11BlendState *i_state );
}

namespace RasterizerStatesManager
{
    DX11_EXPORT ID3D11RasterizerState *GetState( const D3D11_RASTERIZER_DESC *cpo_desc );
    DX11_EXPORT D3D11_RASTERIZER_DESC GetDesc( ID3D11RasterizerState *i_state );
    DX11_EXPORT void FreeState( ID3D11RasterizerState *i_state );
}

#endif __STATES_MANAGERS_HPP__