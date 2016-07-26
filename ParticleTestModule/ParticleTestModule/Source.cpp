﻿#include "PreHeader.hpp"
#include "Header.hpp"

struct alignas(64) ParticleFrame
{
	m4x4 mat;
};

enum { ParticlesReserveSize = 1000000 };

CObject *CreateBox();

ParticleTest::~ParticleTest()
{
	TextureLoader::Free( _textureView );
}

ParticleTest::ParticleTest( const vec3 &o_pos ) : CObject( o_pos, vec3(), vec3(), CVec < SMaterial, void >() )
{
	D3D11_RASTERIZER_DESC rsDesc;
    rsDesc.AntialiasedLineEnable = false;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthBias = 0;
    rsDesc.DepthBiasClamp = 0.f;
    rsDesc.DepthClipEnable = true;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.MultisampleEnable = false;
    rsDesc.ScissorEnable = false;
    rsDesc.SlopeScaledDepthBias = 0.f;

	_rs = RasterizerStatesManager::GetState( &rsDesc );

	D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[ 0 ].BlendEnable = true;
    blendDesc.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	_bs = BlendStatesManager::GetState( &blendDesc );

	D3D11_SAMPLER_DESC sampDesc;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.BorderColor[ 0 ] = 1.f;
    sampDesc.BorderColor[ 1 ] = 1.f;
    sampDesc.BorderColor[ 2 ] = 1.f;
    sampDesc.BorderColor[ 3 ] = 1.f;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.MaxAnisotropy = 1;
    sampDesc.MaxLOD = FLT_MAX;
    sampDesc.MinLOD = -FLT_MAX;
    sampDesc.MipLODBias = 0.f;

	_ss = SamplersManager::GetState( &sampDesc );

	_drawShader = ShadersManager::AcquireByName( "particleTest" );

	ASSUME( sizeof(SParticle) == 32 );

	static const D3D11_BUFFER_DESC vbufDesc =
	{
		ParticlesReserveSize * sizeof(SParticle),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
		0,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		sizeof(SParticle)
	};

	DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &vbufDesc, 0, _particlesVB.AddrModifiable() ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateUnorderedAccessView( _particlesVB, 0, _uav.AddrModifiableRelease() ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateShaderResourceView( _particlesVB, 0, _srv.AddrModifiableRelease() ) );

	static const D3D11_BUFFER_DESC vbufFrameDataDesc =
	{
		ParticlesReserveSize * sizeof(ParticleFrame),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
		0,
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
		sizeof(ParticleFrame)
	};

	DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &vbufFrameDataDesc, 0, _particlesFrameDataVB.AddrModifiable() ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateUnorderedAccessView( _particlesFrameDataVB, 0, _frameDataUAV.AddrModifiableRelease() ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateShaderResourceView( _particlesFrameDataVB, 0, _frameDataSRV.AddrModifiableRelease() ) );

	_textureView = TextureLoader::Load( "Textures/ball.dds" );
	
	if( CompileShader( L"Shaders/eraseFrameData_cs.hlsl", &_eraseFrameDataCS ) )
	{
		SENDLOG( CLogger::Tag::important, "compiled erase frame data cs\n" );
	}

	D3D11_BUFFER_DESC uniBufDesc;
    uniBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    uniBufDesc.ByteWidth = 256;
    uniBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    uniBufDesc.MiscFlags = 0;
    uniBufDesc.StructureByteStride = 0;
    uniBufDesc.Usage = D3D11_USAGE_DYNAMIC;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &uniBufDesc, 0, _uniBuffer.AddrModifiable() ) );

	LoadPSystems();
}

void ParticleTest::Update()
{
	for( PSystem &ps : _psystems )
	{
		for( PSystem::CS &cs : ps.csStack )
		{
			cs.uniforms->UpdateDebugInfo();
		}
	}
}

void ParticleTest::Draw( bln is_stepTwo )
{
	for( PSystem &ps : _psystems )
	{
		for( PSystem::CS &cs : ps.csStack )
		{
			cs.uniforms->DrawDebugInfo( is_stepTwo );
		}
	}

	if( is_stepTwo == false )
	{
		return;
	}

	RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 0, 1, _frameDataUAV.Addr(), 0 );
	RendererGlobals::i_ImContext->CSSetShader( _eraseFrameDataCS.get(), 0, 0 );
	RendererGlobals::i_ImContext->Dispatch( (UINT)MEMALIGN( _particlesCount, 64 ) / 64, 1, 1 );

	ID3D11UnorderedAccessView *resetUAVs[ 3 ] = {};
	RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 0, 3, resetUAVs, 0 );

	RendererGlobals::i_ImContext->CSSetConstantBuffers( 0, 1, _uniBuffer.Addr() );
	RendererGlobals::i_ImContext->CSSetConstantBuffers( 13, 1, &RendererGlobals::ai_VSShaderRegisters[ 13 ] );
			
	RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 0, 1, _uav.Addr(), 0 );
	RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 1, 1, _frameDataUAV.Addr(), 0 );

	for( PSystem &ps : _psystems )
	{
		for( PSystem::CS &cs : ps.csStack )
		{
			RendererGlobals::i_ImContext->CSSetShader( cs.uniforms->GetComputeShader(), 0, 0 );
			cs.uniforms->SetUniformsAndUAVs( _uniBuffer, ps.start );
			RendererGlobals::i_ImContext->Dispatch( ps.count / 64, 1, 1 );
		}
	}

	RendererGlobals::i_ImContext->CSSetUnorderedAccessViews( 0, 3, resetUAVs, 0 );

	if( _particlesCount )
	{
		RendererGlobals::SetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	
		RendererGlobals::i_ImContext->OMSetBlendState( _bs, 0, 0xFFffFFff );
    
		RendererGlobals::i_ImContext->RSSetState( _rs );

		RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &_ss );

		RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &_textureView );

		RendererGlobals::i_ImContext->IASetInputLayout( nullptr );

		ID3D11Buffer *resetVB = 0;
		UINT resetValues = 0;
		RendererGlobals::i_ImContext->IASetVertexBuffers( 0, 1, &resetVB, &resetValues, &resetValues );

		ShadersManager::ApplyShader( _drawShader, false );

		RendererGlobals::i_ImContext->VSSetShaderResources( 0, 1, _srv.Addr() );
		RendererGlobals::i_ImContext->VSSetShaderResources( 1, 1, _frameDataSRV.Addr() );

		RendererGlobals::CurrentBloom->RenderingStatesSet( RStates::depthTest | RStates::glowmap | RStates::target, false );

		RendererGlobals::i_ImContext->Draw( 3 * _particlesCount, 0 );

		ID3D11ShaderResourceView *resetSRVs[ 2 ] = {};
		RendererGlobals::i_ImContext->VSSetShaderResources( 0, 2, resetSRVs );
	}
}

CObject *CreateBox()
{
	static Nullable < SGeometry > boxGeo;
	if( boxGeo.IsNull() )
	{
		Geometry::BoxTN( &boxGeo.Get(), true );
	}

	SGeometrySlice slice;
	slice.indicesCount = boxGeo->indicesCount;
	slice.is_defined = true;
	slice.startIndex = 0;
	slice.startVertex = 0;
	slice.verticesCount = boxGeo->verticesCount;

	D3D11_SAMPLER_DESC o_sampDef;
    o_sampDef.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDef.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDef.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDef.BorderColor[ 0 ] = 1.f;
    o_sampDef.BorderColor[ 1 ] = 1.f;
    o_sampDef.BorderColor[ 2 ] = 1.f;
    o_sampDef.BorderColor[ 3 ] = 1.f;
    o_sampDef.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_sampDef.Filter = D3D11_FILTER_ANISOTROPIC;
    o_sampDef.MaxAnisotropy = 16;
    o_sampDef.MaxLOD = FLT_MAX;
    o_sampDef.MinLOD = -FLT_MAX;
    o_sampDef.MipLODBias = 0.f;

	D3D11_BLEND_DESC o_blend = {};
    o_blend.AlphaToCoverageEnable = false;
    o_blend.IndependentBlendEnable = false;
    o_blend.RenderTarget[ 0 ].BlendEnable = true;
    o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    o_blend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_RASTERIZER_DESC o_rsDesc;
    o_rsDesc.AntialiasedLineEnable = false;
    o_rsDesc.CullMode = D3D11_CULL_BACK;
    o_rsDesc.DepthBias = 0;
    o_rsDesc.DepthBiasClamp = 0.f;
    o_rsDesc.DepthClipEnable = true;
    o_rsDesc.FillMode = D3D11_FILL_SOLID;
    o_rsDesc.FrontCounterClockwise = false;
    o_rsDesc.MultisampleEnable = false;
    o_rsDesc.ScissorEnable = false;
    o_rsDesc.SlopeScaledDepthBias = 0.f;

	CVec < STex, void > tex;
	tex.EmplaceBack( TextureLoader::Load( "Textures/Best-Desktop-Background-130.jpg" ), &o_sampDef, vec2( 0 ), vec2( 4000, 4 ), vec2( 0.75, 0.75 ), 0 );

	CVec < SMaterial, void > mat;
	mat.EmplaceBack( &boxGeo.Get(), slice, 1, 0, std::move( tex ), ShadersManager::AcquireByName( "lightless" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.5, RStates::target );
	mat[ 0 ].is_enabled = true;
	mat[ 0 ].is_inFrustum = true;

	return new CObject( vec3( 0 ), vec3( 0 ), vec3( 0 ), std::move( mat ) );
}

extern "C" __declspec(dllexport) CObjectBase *CreateParticleTest( const vec3 &o_pos )
{
	return new ParticleTest( o_pos );
}

extern "C" __declspec(dllexport) void DestroyParticleTest( CObjectBase *test )
{
	delete test;
}

BOOL WINAPI DllMain( HINSTANCE h_inst, DWORD reason, PVOID )
{
    switch( reason )
    {
    case DLL_PROCESS_ATTACH:
        {
            ::DisableThreadLibraryCalls( h_inst );
			StdAbstractionLib_Initialize();
			FilloutEffects();
        }
        break;
    }
    return TRUE;
}