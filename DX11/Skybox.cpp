#include "PreHeader.hpp"
#include "Skybox.hpp"
#include "TextureLoader.hpp"
#include "ShadersManager.hpp"
#include "Geometry.hpp"
#include "CObject.hpp"
#include "Camera.hpp"
#include "RendererGlobals.hpp"

namespace
{
	byte Drawer[ sizeof(CObject) ];
}

void Skybox::Update()
{
	CObject *drawer = (CObject *)Drawer;
    drawer->PosSet( RendererGlobals::MainCamera.PositionGet() );
    drawer->Update();
}

void Skybox::Draw()
{
	CObject *drawer = (CObject *)Drawer;
    drawer->Draw( false );
}

void Skybox::TextureSet( const char *cp_pnn )
{
}

void Skybox::Create( const char *cp_pnn )
{
    static SGeometry o_geo;
    Geometry::Box( &o_geo );
    o_geo.ComputeXAABB = 0;
    SGeometrySlice o_geoSlice( 0, 0, o_geo.verticesCount, o_geo.indicesCount );

    D3D11_SAMPLER_DESC o_sampMirLo;
    o_sampMirLo.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_sampMirLo.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_sampMirLo.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_sampMirLo.BorderColor[ 0 ] = 1.f;
    o_sampMirLo.BorderColor[ 1 ] = 1.f;
    o_sampMirLo.BorderColor[ 2 ] = 1.f;
    o_sampMirLo.BorderColor[ 3 ] = 1.f;
    o_sampMirLo.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_sampMirLo.Filter = D3D11_FILTER_ANISOTROPIC;
    o_sampMirLo.MaxAnisotropy = 16;
    o_sampMirLo.MaxLOD = FLT_MAX;
    o_sampMirLo.MinLOD = -FLT_MAX;
    o_sampMirLo.MipLODBias = 0.f;

    D3D11_BLEND_DESC o_blend = {};
    o_blend.AlphaToCoverageEnable = false;
    o_blend.IndependentBlendEnable = false;
    o_blend.RenderTarget[ 0 ].BlendEnable = false;
    o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    o_blend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;

    D3D11_RASTERIZER_DESC o_rsDesc;
    o_rsDesc.AntialiasedLineEnable = false;
    o_rsDesc.CullMode = D3D11_CULL_NONE;
    o_rsDesc.DepthBias = 0;
    o_rsDesc.DepthBiasClamp = 0.f;
    o_rsDesc.DepthClipEnable = true;
    o_rsDesc.FillMode = D3D11_FILL_SOLID;
    o_rsDesc.FrontCounterClockwise = false;
    o_rsDesc.MultisampleEnable = false;
    o_rsDesc.ScissorEnable = false;
    o_rsDesc.SlopeScaledDepthBias = 0.f;

    CVec < SMaterial, void > o_mat( 1 );
    CVec < STex, void > o_tex( 1 );

    o_tex.EmplaceBack( TextureLoader::Load( cp_pnn ), &o_sampMirLo, vec2( 0, 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.EmplaceBack( &o_geo, o_geoSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "skybox" ), &o_blend, &o_rsDesc, Colora::White, Colora::Black, Color::Black, 0.1, RStates::target );
    new (Drawer) CObject( RendererGlobals::MainCamera.PositionGet(), vec3( 0, 0, 0 ), vec3( 5000, 5000, 5000 ), std::move( o_mat ) );
}