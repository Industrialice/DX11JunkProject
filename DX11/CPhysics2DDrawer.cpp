#include "PreHeader.hpp"
#include "CPhysics2DDrawer.hpp"
#include "Bloom.hpp"
#include "ShadersManager.hpp"
#include "TextureLoader.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"

namespace
{
    ID3D11ShaderResourceView *i_RectTex;
    ID3D11ShaderResourceView *i_CircleTex;
    ID3D11Buffer *i_ShapeVB;
    ID3D11Buffer *i_ShapeDynamicVB;
    ui32 ShapeDynamicVBSize;
    sdrhdl RectShader, CircleShader, TriangleShader;
    ID3D11SamplerState *i_Sampler;
    ID3D11BlendState *i_Blend;
}

static void InitializeStaticData();
static NOINLINE void AllocDynamicVB( ui32 size );

CPhysics2DDrawer::CPhysics2DDrawer() : CPhysics2D()
{
    InitializeStaticData();
}

void CPhysics2DDrawer::FlushBuffers()  //  virtual
{
    CPhysics2D::FlushBuffers();

    if( i_ShapeDynamicVB )
    {
        i_ShapeDynamicVB->Release();
    }
    i_ShapeDynamicVB = 0;
    ShapeDynamicVBSize = 0;
}

void CPhysics2DDrawer::Draw( bln is_stepTwo )  //  virtual
{
    if( !is_stepTwo )
    {
        return;
    }

    RendererGlobals::MainBloom.RenderingStatesSet( RStates::target );
    RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &i_Sampler );
    if( _shapeCount[ ShapeRect ] )
    {
        if( _shapeCount[ ShapeRect ] * (sizeof(vec4) + sizeof(vec2)) > ShapeDynamicVBSize )
        {
            AllocDynamicVB( _shapeCount[ ShapeRect ] * (sizeof(vec4) + sizeof(vec2)) );
        }
        ShadersManager::ApplyShader( RectShader, false );
        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( i_ShapeDynamicVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
        ui32 srOffset = 0;
        const SRect *rect;
        for( ui32 findOffset = 0; rect = (SRect *)FindNextShapeOfType( ShapeRect, &findOffset ); )
        {
            m2x2 matrix;
            LiceMath::M2x2ScaleRotate( &matrix, rect->size.x, rect->size.y, rect->rotation );
            _MemCpy( (byte *)o_sr.pData + srOffset, &matrix, sizeof(m2x2) );
			srOffset += sizeof(m2x2);
            _MemCpy( (byte *)o_sr.pData + srOffset, &rect->pos, sizeof(vec2) );
			srOffset += sizeof(vec2);
        }
        RendererGlobals::i_ImContext->Unmap( i_ShapeDynamicVB, 0 );
        static const UINT strides[ 2 ] = { sizeof(vec2) + sizeof(vec2), sizeof(vec4) + sizeof(vec2) };
        static const UINT offsets[ 2 ] = { 0, 0 };
        ID3D11Buffer *i_buffers[ 2 ] = { i_ShapeVB, i_ShapeDynamicVB };
        RendererGlobals::i_ImContext->IASetVertexBuffers( 0, 2, i_buffers, strides, offsets );
        RendererGlobals::SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_RectTex );
        RendererGlobals::i_ImContext->OMSetBlendState( RendererGlobals::i_NoBlend, 0, 0xFFffFFff );
        RendererGlobals::i_ImContext->RSSetState( RendererGlobals::i_RS );
        RendererGlobals::i_ImContext->DrawInstanced( 4, _shapeCount[ ShapeRect ], 0, 0 );
    }
    if( _shapeCount[ ShapeCircle ] )
    {
        if( _shapeCount[ ShapeCircle ] * (sizeof(vec4) + sizeof(vec2)) > ShapeDynamicVBSize )
        {
            AllocDynamicVB( _shapeCount[ ShapeCircle ] * (sizeof(vec4) + sizeof(vec2)) );
        }
        ShadersManager::ApplyShader( CircleShader, false );
        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( i_ShapeDynamicVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
        ui32 srOffset = 0;
        const SCircle *circle;
        for( ui32 findOffset = 0; circle = (SCircle *)FindNextShapeOfType( ShapeCircle, &findOffset ); )
        {
            m2x2 matrix;
            LiceMath::M2x2ScaleRotate( &matrix, circle->radius, circle->radius, circle->rotation );
            _MemCpy( (byte *)o_sr.pData + srOffset, &matrix, sizeof(m2x2) );
			srOffset += sizeof(m2x2);
            _MemCpy( (byte *)o_sr.pData + srOffset, &circle->pos, sizeof(vec2) );
			srOffset += sizeof(vec2);
        }
        RendererGlobals::i_ImContext->Unmap( i_ShapeDynamicVB, 0 );
        static const UINT strides[ 2 ] = { sizeof(vec2) + sizeof(vec2), sizeof(vec4) + sizeof(vec2) };
        static const UINT offsets[ 2 ] = { 0, 0 };
        ID3D11Buffer *i_buffers[ 2 ] = { i_ShapeVB, i_ShapeDynamicVB };
        RendererGlobals::i_ImContext->IASetVertexBuffers( 0, 2, i_buffers, strides, offsets );
        RendererGlobals::SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_CircleTex );
        RendererGlobals::i_ImContext->OMSetBlendState( i_Blend, 0, 0xFFffFFff );
        RendererGlobals::i_ImContext->RSSetState( RendererGlobals::i_RS );
        RendererGlobals::i_ImContext->DrawInstanced( 4, _shapeCount[ ShapeCircle ], 0, 0 );
    }
    if( _shapeCount[ ShapeTriangle ] )
    {
        //ShadersManager::ApplyShader( TriangleShader, false );
    }
}

void InitializeStaticData()
{
    if( i_RectTex )
    {
        return;
    }
    i_RectTex = TextureLoader::Load( "Textures/box.dds" );
    i_CircleTex = TextureLoader::Load( "Textures/brw-chrome.dds" );

    const f32 scale = 0.5f;
    struct SShapeVertex
    {
        vec2 pos;
        vec2 texcoord;
    } vertices[ 4 ] =
    {
        { vec2( -scale, -scale ), vec2( 0.f, 1.f ) },
        { vec2( -scale, scale ), vec2( 0.f, 0.f ) },
        { vec2( scale, -scale ), vec2( 1.f, 1.f ) },
        { vec2( scale, scale ), vec2( 1.f, 0.f ) }
    };
    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = vertices;

    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = sizeof(vertices);
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &desc, &data, &i_ShapeVB ) );

    RectShader = ShadersManager::AcquireByName( "rect_shader" );
    CircleShader = ShadersManager::AcquireByName( "rect_shader" );

    D3D11_BLEND_DESC blend = {};
    blend.AlphaToCoverageEnable = false;
    blend.IndependentBlendEnable = false;
    blend.RenderTarget[ 0 ].BlendEnable = true;
    blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    blend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    i_Blend = BlendStatesManager::GetState( &blend );

    D3D11_SAMPLER_DESC o_samp;
    o_samp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    o_samp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    o_samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    o_samp.BorderColor[ 0 ] = 1.f;
    o_samp.BorderColor[ 1 ] = 1.f;
    o_samp.BorderColor[ 2 ] = 1.f;
    o_samp.BorderColor[ 3 ] = 1.f;
    o_samp.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_samp.Filter = D3D11_FILTER_ANISOTROPIC;
    o_samp.MaxAnisotropy = 16;
    o_samp.MaxLOD = FLT_MAX;
    o_samp.MinLOD = -FLT_MAX;
    o_samp.MipLODBias = 0.f;
    i_Sampler = SamplersManager::GetState( &o_samp );
}

void AllocDynamicVB( ui32 size )
{
    if( i_ShapeDynamicVB )
    {
        i_ShapeDynamicVB->Release();
    }
    ShapeDynamicVBSize = size + 1024;
    D3D11_BUFFER_DESC desc;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = ShapeDynamicVBSize;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &desc, 0, &i_ShapeDynamicVB ) );
}