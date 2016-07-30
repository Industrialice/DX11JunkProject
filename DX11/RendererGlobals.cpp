#include "PreHeader.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"

namespace
{
    D3D11_PRIMITIVE_TOPOLOGY CurrentPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    D3D11_VIEWPORT CurrentViewport;
}

namespace RendererGlobals
{
    CCamera *CurrentCamera, MainCamera;

    CBloom *CurrentBloom, MainBloom;

    ScreenRect PixelsRectToScreenRect( const PixelsRect &rect )
    {
        ScreenRect sr;
        sr.x0 = (rect.x0 * 2) / (f32)RendererGlobals::RenderingWidth - 1.f;
        sr.x1 = (rect.x1 * 2) / (f32)RendererGlobals::RenderingWidth - 1.f;
        sr.y0 = (rect.y0 * -2) / (f32)RendererGlobals::RenderingHeight + 1.f;
        sr.y1 = (rect.y1 * -2) / (f32)RendererGlobals::RenderingHeight + 1.f;
        return sr;
    }

    ID3D11Device *i_Device;
	ID3D11DeviceContext *i_ImContext;
	IDXGISwapChain *i_SwapChain;
	ID3D11Texture2D *i_DepthStencilBuffer;
	ID3D11RenderTargetView *i_SuperSampleRTV;
	ID3D11ShaderResourceView *i_SuperSampleSRV;
	ID3D11RenderTargetView *i_MainRenderTargetView;
    ID3D11ShaderResourceView *i_MainSRV;
	ID3D11DepthStencilView *i_DepthStencilView;
	D3D11_VIEWPORT o_ScreenViewport;
    ID3D11RasterizerState *i_RS;
    ID3D11BlendState *i_NoBlend;
    ID3D11DepthStencilState *i_DepthStencilDefault;
	bln is_UseSuperSampling = false;
	ui32 RenderingWidth, RenderingHeight;

    LayoutsManager DefLayoutsManager;

    ID3D11ShaderResourceView *i_WhiteTex;
    ID3D11ShaderResourceView *i_BlackTex;

    ID3D11Buffer *ai_VSShaderRegisters[ 14 ];
    ID3D11Buffer *ai_GSShaderRegisters[ 14 ];
    ID3D11Buffer *ai_PSShaderRegisters[ 14 ];

    void SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY topo )
    {
        if( CurrentPrimitiveTopology != topo )
        {
            i_ImContext->IASetPrimitiveTopology( topo );
            CurrentPrimitiveTopology = topo;
        }
    }
    
    void SetPrimitiveTopologyOr( D3D11_PRIMITIVE_TOPOLOGY first, D3D11_PRIMITIVE_TOPOLOGY second )
    {
        if( CurrentPrimitiveTopology != first && CurrentPrimitiveTopology != second )
        {
            i_ImContext->IASetPrimitiveTopology( first );
            CurrentPrimitiveTopology = first;
        }
    }

    void UnbindVBuffers( ui32 start, ui32 count )
    {
        ASSUME( start + count <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        static ID3D11Buffer *const bufs[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
        static const UINT offsetsAndStrides[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
        i_ImContext->IASetVertexBuffers( start, count, bufs, offsetsAndStrides, offsetsAndStrides );
    }

    void UnbindSOTargets( ui32 count )
    {
        ASSUME( count < D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        static ID3D11Buffer *const i_null[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
        static const UINT null[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
        RendererGlobals::i_ImContext->SOSetTargets( count, i_null, null );
    }

    void SetViewports( UINT count, const D3D11_VIEWPORT *vp )
    {
        if( count == 0 )
        {
			Funcs::ClearPod( &CurrentViewport );
            RendererGlobals::i_ImContext->RSSetViewports( 0, 0 );
        }
        else
        {
            ASSUME( vp );
            if( count > 1 || !_MemEquals( &CurrentViewport, vp, sizeof(D3D11_VIEWPORT) ) )
            {
                _MemCpy( &CurrentViewport, vp, sizeof(D3D11_VIEWPORT) );
                RendererGlobals::i_ImContext->RSSetViewports( count, vp );
            }
        }
    }
}