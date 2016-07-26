#ifndef __RENDERER_GLOBALS_HPP__
#define __RENDERER_GLOBALS_HPP__

#include "Camera.hpp"
#include "Bloom.hpp"
#include "LayoutsManager.hpp"

namespace RendererGlobals
{
    DX11_EXPORT extern CCamera *CurrentCamera, MainCamera;

    DX11_EXPORT extern CBloom *CurrentBloom, MainBloom;

    DX11_EXPORT ScreenRect PixelsRectToScreenRect( const PixelsRect &rect );

    DX11_EXPORT extern ID3D11Device *i_Device;
	DX11_EXPORT extern ID3D11DeviceContext *i_ImContext;
	DX11_EXPORT extern IDXGISwapChain *i_SwapChain;
	DX11_EXPORT extern ID3D11Texture2D *i_DepthStencilBuffer;
	DX11_EXPORT extern ID3D11RenderTargetView *i_MainRenderTargetView;
    DX11_EXPORT extern ID3D11ShaderResourceView *i_MainSRV;
	DX11_EXPORT extern ID3D11DepthStencilView *i_DepthStencilView;
	DX11_EXPORT extern D3D11_VIEWPORT o_ScreenViewport;
    DX11_EXPORT extern ID3D11RasterizerState *i_RS;
    DX11_EXPORT extern ID3D11BlendState *i_NoBlend;
    DX11_EXPORT extern ID3D11DepthStencilState *i_DepthStencilDefault;

    DX11_EXPORT extern LayoutsManager DefLayoutsManager;

    DX11_EXPORT extern ID3D11ShaderResourceView *i_WhiteTex;
    DX11_EXPORT extern ID3D11ShaderResourceView *i_BlackTex;

    DX11_EXPORT extern ID3D11Buffer *ai_VSShaderRegisters[ 14 ];
    DX11_EXPORT extern ID3D11Buffer *ai_GSShaderRegisters[ 14 ];
    DX11_EXPORT extern ID3D11Buffer *ai_PSShaderRegisters[ 14 ];

    DX11_EXPORT void SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY topo );
    DX11_EXPORT void SetPrimitiveTopologyOr( D3D11_PRIMITIVE_TOPOLOGY first, D3D11_PRIMITIVE_TOPOLOGY second );  //  priority to first
    DX11_EXPORT void UnbindVBuffers( ui32 start, ui32 count );
    DX11_EXPORT void UnbindSOTargets( ui32 count );
    DX11_EXPORT void SetViewports( UINT count, const D3D11_VIEWPORT *vp );
}

#endif __RENDERER_GLOBALS_HPP__