#include "PreHeader.hpp"
#include "The Test.hpp"
#include "CObject.hpp"
#include "TextureLoader.hpp"
#include "CHalos.hpp"
#include "Camera.hpp"
#include "S3ASCLoader.hpp"
#include "Heap.hpp"
#include "CMatrix.hpp"
#include "CParticleSystemGPU.hpp"
#include "CommandsManager.hpp"
#include "RendererGlobals.hpp"
#include <Files.hpp>
#include "Globals.hpp"

namespace
{
    CObject *Grass;
    CVec < CObject * > FloorPlites;
    CObject *Road;
    CVec < CObject * > Walls;
    CVec < CPointLight * > Room1Ligts;
    CVec < CPointLight * > Room2Ligts;
    CVec < CPointLight * > Room3Ligts;
    CVec < CPointLight * > BuildingLigts;
    CVec < CPointLight * > Building2Ligts;
    CVec < CPointLight * > Building3Ligts;
    CVec < CPointLight * > MatrixLigts;
    CVec < CPointLight * > FromInsideLigts;
    CVec < CObject * > Bulbs;
    CHalos *Halos;
    ui32 WalkPlatesStart, WalkPlatesEnd;
    CVec < CObject * > Furniture;
    CVec < CObject * > Buildings;
    CObject *Fence;
    CVec < CMatrix * > Matrices;
    CVec < CParticleSystemGPU * > ParticleSystems;
	CVec < CObjectBase * > ParticleTests;
}

#define CF( a ) ((a) * 0.25)
#define CT( a ) ((a) / 0.25)

static void CheckParticleTests();

void GeoFromFile( const char *cp_pnn, SGeometry *geo )
{
    ui16 vertexCount;
    f32 *vbuf;
    ui32 vbufSize;
    ui32 indexCount;
    ui16 *ibuf;
    VertexFmt::vertexFmt_t *fmts;
    ui32 attribsCount;
    bln result = S3ASCLoader::LoadFromFile( cp_pnn, &fmts, &attribsCount, &vertexCount, &vbuf, &vbufSize, &indexCount, &ibuf );
    if( !result )
    {
        ::MessageBoxA( 0, cp_pnn, "geo loading failed", 0 );
        ::FatalAppExitW( 1, 0 );
    }
    Geometry::Make( geo, fmts, attribsCount, vbuf, vbufSize, vertexCount, ibuf, indexCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    Heap::Free( vbuf );
    Heap::Free( ibuf );
    Heap::Free( fmts );
}

void GeoFromFilesToMaterials( const char *const *cp_pnns, SMaterial *mats, ui32 count, SGeometry *geos )
{
    ASSUME( cp_pnns && mats && count && geos );

    struct SStorage : CharPOD
    {
        VertexFmt::vertexFmt_t *fmt;
        ui32 attribsCount;
        ui32 vertexUnitedCount;
        ui32 vertexUnitedSize;
        ui32 indexUnitedCount;
        f32 *vbufUnited;
        ui16 *ibufUnited;
    };
    CVec < SStorage > storages( count, count );
    _MemZero( &storages[ 0 ], sizeof(SStorage) * count );
    ui32 usedStorages = 0;

    for( ui32 index = 0; index < count; ++index )
    {
        ui16 vertexCount;
        f32 *vbuf;
        ui32 vbufSize;
        ui32 indexCount;
        ui16 *ibuf;
        VertexFmt::vertexFmt_t *fmt;
        ui32 attribsCount;
        bln result = S3ASCLoader::LoadFromFile( cp_pnns[ index ], &fmt, &attribsCount, &vertexCount, &vbuf, &vbufSize, &indexCount, &ibuf );
        if( !result )
        {
			SOFTBREAK;
            ::MessageBoxA( 0, cp_pnns[ index ], "geo loading failed", 0 );
            ::FatalAppExitW( 1, 0 );
        }
        ui32 selectedStorage = 0;
        for( ; selectedStorage < usedStorages; ++selectedStorage )
        {
            if( attribsCount == storages[ selectedStorage ].attribsCount )
            {
                if( _MemEquals( fmt, storages[ selectedStorage ].fmt, sizeof(VertexFmt::vertexFmt_t) * attribsCount ) )
                {
                    break;
                }
            }
        }

        mats[ index ].po_geo = &geos[ selectedStorage ];
        mats[ index ].o_geoSlice.indicesCount = indexCount;
        mats[ index ].o_geoSlice.startIndex = storages[ selectedStorage ].indexUnitedCount;
        mats[ index ].o_geoSlice.startVertex = storages[ selectedStorage ].vertexUnitedCount;
        mats[ index ].o_geoSlice.verticesCount = vertexCount;

        if( selectedStorage == usedStorages )
        {
            ASSUME( usedStorages < count );
            storages[ selectedStorage ].fmt = fmt;
            storages[ selectedStorage ].vbufUnited = vbuf;
            storages[ selectedStorage ].ibufUnited = ibuf;
            storages[ selectedStorage ].attribsCount = attribsCount;
            storages[ selectedStorage ].indexUnitedCount = indexCount;
            storages[ selectedStorage ].vertexUnitedCount = vertexCount;
            storages[ selectedStorage ].vertexUnitedSize = vbufSize;
            ++usedStorages;
        }
        else
        {
            storages[ selectedStorage ].vbufUnited = (f32 *)Heap::Realloc( storages[ selectedStorage ].vbufUnited, storages[ selectedStorage ].vertexUnitedSize + vbufSize );
            _MemCpy( (byte *)storages[ selectedStorage ].vbufUnited + storages[ selectedStorage ].vertexUnitedSize, vbuf, vbufSize );
            storages[ selectedStorage ].ibufUnited = (ui16 *)Heap::Realloc( storages[ selectedStorage ].ibufUnited, (storages[ selectedStorage ].indexUnitedCount + indexCount) * sizeof(ui16) );
            _MemCpy( storages[ selectedStorage ].ibufUnited + storages[ selectedStorage ].indexUnitedCount, ibuf, indexCount * sizeof(ui16) );
            storages[ selectedStorage ].indexUnitedCount += indexCount;
            storages[ selectedStorage ].vertexUnitedCount += vertexCount;
            storages[ selectedStorage ].vertexUnitedSize += vbufSize;
            Heap::Free( vbuf );
            Heap::Free( ibuf );
            Heap::Free( fmt );
        }
    }

    for( ui32 storage = 0; storage < usedStorages; ++storage )
    {
        Geometry::Make( &geos[ storage ], storages[ storage ].fmt, storages[ storage ].attribsCount, storages[ storage ].vbufUnited, storages[ storage ].vertexUnitedSize, storages[ storage ].vertexUnitedCount, storages[ storage ].ibufUnited, storages[ storage ].indexUnitedCount, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        Heap::Free( storages[ storage ].vbufUnited );
        Heap::Free( storages[ storage ].ibufUnited );
        Heap::Free( storages[ storage ].fmt );
    }
}

class CDrawBlocker
{
    struct SObjData : CharPOD
    {
        vec3 pos;
        vec3 normal;
        CObjectBase *ob;
    };
    CVec < SObjData, void > _objs;

public:
    void Add( const vec3 &pos, const vec3 &normal, CObjectBase *object )
    {
        SObjData data;
		data.pos = pos;
		data.normal = normal;
		data.ob = object;
        _objs.Append( data );
    }

    void Update()
    {
        for( ui32 index = 0; index < _objs.Size(); ++index )
        {
            vec3 toCam;
            LiceMath::Vec3Subtract( &toCam, &RendererGlobals::MainCamera.PositionGet(), &_objs[ index ].pos );
            f32 dot = LiceMath::Vec3Dot( &toCam, &_objs[ index ].normal );
            _objs[ index ].ob->IsVisibleSet( dot < 0 );
        }
    }
} DrawBlocker;

void TheTest::Update()
{
	CheckParticleTests();

    if( Grass )
    {
        Grass->Update();
    }
    if( Road )
    {
        Road->Update();
    }
    for( ui32 index = 0; index < FloorPlites.Size(); ++index )
    {
        FloorPlites[ index ]->Update();
    }
    for( ui32 index = 0; index < Walls.Size(); ++index )
    {
        Walls[ index ]->Update();
    }
    for( ui32 index = 0; index < Furniture.Size(); ++index )
    {
        Furniture[ index ]->Update();
    }
    for( ui32 index = 0; index < Buildings.Size(); ++index )
    {
        Buildings[ index ]->Update();
    }
    if( Fence )
    {
        Fence->Update();
    }
    for( ui32 index = 0; index < Matrices.Size(); ++index )
    {
        Matrices[ index ]->Update();
    }
    for( ui32 index = 0; index < ParticleSystems.Size(); ++index )
    {
        ParticleSystems[ index ]->Update();
    }
    for( ui32 index = 0; index < Bulbs.Size(); ++index )
    {
        Bulbs[ index ]->Update();
    }
    if( Halos )
    {
        Halos->Update();
    }
	for( auto &value : ParticleTests )
	{
		value->Update();
	}

    DrawBlocker.Update();
}

void TheTest::Draw()
{
    for( ui32 step = 0; step < 2; ++step )
    {
        bln is_stepTwo = step > 0;

        if( Grass && Grass->IsInFrustumGet() && Grass->IsVisibleGet() )
        {
            if( is_stepTwo == false || Grass->IsGlowingGet() )
            {
                Grass->Draw( is_stepTwo );
            }
        }
        if( Road && Road->IsInFrustumGet() && Road->IsVisibleGet() )
        {
            if( is_stepTwo == false || Road->IsGlowingGet() )
            {
                Road->Draw( is_stepTwo );
            }
        }
        for( ui32 index = 0; index < FloorPlites.Size(); ++index )
        {
            if( FloorPlites[ index ]->IsInFrustumGet() && FloorPlites[ index ]->IsVisibleGet() )
            {
                if( is_stepTwo == false || FloorPlites[ index ]->IsGlowingGet() )
                {
                    FloorPlites[ index ]->Draw( is_stepTwo );
                }
            }
        }
        for( ui32 index = 0; index < Walls.Size(); ++index )
        {
            if( Walls[ index ]->IsInFrustumGet() && Walls[ index ]->IsVisibleGet() )
            {
                if( is_stepTwo == false || Walls[ index ]->IsGlowingGet() )
                {
                    Walls[ index ]->Draw( is_stepTwo );
                }
            }
        }
        for( ui32 index = 0; index < Furniture.Size(); ++index )
        {
            if( Furniture[ index ]->IsInFrustumGet() && Furniture[ index ]->IsVisibleGet() )
            {
                if( is_stepTwo == false || Furniture[ index ]->IsGlowingGet() )
                {
                    Furniture[ index ]->Draw( is_stepTwo );
                }
            }
        }
        for( ui32 index = 0; index < Buildings.Size(); ++index )
        {
            if( Buildings[ index ]->IsInFrustumGet() && Buildings[ index ]->IsVisibleGet() )
            {
                if( is_stepTwo == false || Buildings[ index ]->IsGlowingGet() )
                {
                    Buildings[ index ]->Draw( is_stepTwo );
                }
            }
        }
        if( Fence && Fence->IsInFrustumGet() && Fence->IsVisibleGet() )
        {
            if( is_stepTwo == false || Fence->IsGlowingGet() )
            {
                Fence->Draw( is_stepTwo );
            }
        }
        for( ui32 index = 0; index < Matrices.Size(); ++index )
        {
            if( Matrices[ index ]->IsInFrustumGet() && Matrices[ index ]->IsVisibleGet() )
            {
                if( is_stepTwo == false || Matrices[ index ]->IsGlowingGet() )
                {
                    Matrices[ index ]->Draw( is_stepTwo );
                }
            }
        }
        for( ui32 index = 0; index < ParticleSystems.Size(); ++index )
        {
            if( ParticleSystems[ index ]->IsInFrustumGet() && ParticleSystems[ index ]->IsVisibleGet() )
            {
                if( is_stepTwo == false || ParticleSystems[ index ]->IsGlowingGet() )
                {
                    ParticleSystems[ index ]->Draw( is_stepTwo );
                }
            }
        }
        if( Halos && Halos->IsVisibleGet() )
        {
            if( is_stepTwo == false || Halos->IsGlowingGet() )
            {
                Halos->Draw( is_stepTwo );
            }
        }
		for( auto &value : ParticleTests )
		{
			value->Draw( is_stepTwo );
		}
    }
}

#pragma optimize( "s", on )

namespace
{
	CObjectBase *(*CreateParticleTest)( const vec3 &o_pos );
	void (*DestroyParticleTest)( CObjectBase *test );
	HMODULE ParticleTestLib;
}

void TheTest::ReloadParticleTests()
{
	for( CObjectBase *pt : ParticleTests )
	{
		DestroyParticleTest( pt );
	}
	ParticleTests.Clear();

	CreateParticleTest = nullptr;
	DestroyParticleTest = nullptr;
	FreeLibrary( ParticleTestLib );

	CError error;

	wchar_t moduleFN[ MAX_PATH ];

	FilePath sourceModulePath, targetModulePath;

	GetModuleFileNameW( GetModuleHandleW( nullptr ), moduleFN, sizeof(moduleFN) );

	sourceModulePath = moduleFN;
	sourceModulePath.PopLevel();
	targetModulePath = sourceModulePath;

	sourceModulePath += L"ParticleTestModule.dll";
	targetModulePath += L"ParticleTestModuleCopy.dll";

	if( !Files::CopyFileTo( sourceModulePath, targetModulePath, true, &error ) )
	{
		SENDLOG( CLogger::Tag::error, "failed to create ParticleTestModuleCopy.dll file due to error %s\n", error.Description() );
		return;
	}

	ParticleTestLib = LoadLibraryA( "ParticleTestModuleCopy.dll" );
	if( !ParticleTestLib )
	{
		SENDLOG( CLogger::Tag::error, "failed to load library ParticleTestModuleCopy.dll\n" );
		return;
	}

	*(FARPROC *)&CreateParticleTest = GetProcAddress( ParticleTestLib, "CreateParticleTest" );
	if( !CreateParticleTest )
	{
		SENDLOG( CLogger::Tag::error, "failed to resolve symbol CreateParticleTest\n" );
		return;
	}

	*(FARPROC *)&DestroyParticleTest = GetProcAddress( ParticleTestLib, "DestroyParticleTest" );
	if( !DestroyParticleTest )
	{
		SENDLOG( CLogger::Tag::error, "failed to resolve symbol DestroyParticleTest\n" );
		return;
	}

	ParticleTests.EmplaceBack( CreateParticleTest( vec3( 2.5, 4.5, -8 ) ) );
}

void TheTest::Create()
{
    //return;

	ReloadParticleTests();

	//return;

    static SGeometry o_flattn;
    Geometry::FlatTN( &o_flattn );
    SGeometrySlice o_flattnSlice( 0, 0, o_flattn.verticesCount, o_flattn.indicesCount );
    static SGeometry o_flat;
    Geometry::Flat( &o_flat );
    SGeometrySlice o_flatSlice( 0, 0, o_flat.verticesCount, o_flat.indicesCount );
    static SGeometry o_flat2;
    Geometry::Flat2( &o_flat2 );
    SGeometrySlice o_flat2Slice( 0, 0, o_flat2.verticesCount, o_flat2.indicesCount );

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

    D3D11_SAMPLER_DESC o_sampDefTri;
    o_sampDefTri.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDefTri.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDefTri.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDefTri.BorderColor[ 0 ] = 1.f;
    o_sampDefTri.BorderColor[ 1 ] = 1.f;
    o_sampDefTri.BorderColor[ 2 ] = 1.f;
    o_sampDefTri.BorderColor[ 3 ] = 1.f;
    o_sampDefTri.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_sampDefTri.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    o_sampDefTri.MaxAnisotropy = 1;
    o_sampDefTri.MaxLOD = FLT_MAX;
    o_sampDefTri.MinLOD = -FLT_MAX;
    o_sampDefTri.MipLODBias = 0.f;

    D3D11_SAMPLER_DESC o_sampDefLo;
    o_sampDefLo.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDefLo.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDefLo.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    o_sampDefLo.BorderColor[ 0 ] = 1.f;
    o_sampDefLo.BorderColor[ 1 ] = 1.f;
    o_sampDefLo.BorderColor[ 2 ] = 1.f;
    o_sampDefLo.BorderColor[ 3 ] = 1.f;
    o_sampDefLo.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_sampDefLo.Filter = D3D11_FILTER_ANISOTROPIC;
    o_sampDefLo.MaxAnisotropy = 16;
    o_sampDefLo.MaxLOD = FLT_MAX;
    o_sampDefLo.MinLOD = -FLT_MAX;
    o_sampDefLo.MipLODBias = -1.f;

    D3D11_SAMPLER_DESC o_sampMir;
    o_sampMir.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_sampMir.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_sampMir.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_sampMir.BorderColor[ 0 ] = 1.f;
    o_sampMir.BorderColor[ 1 ] = 1.f;
    o_sampMir.BorderColor[ 2 ] = 1.f;
    o_sampMir.BorderColor[ 3 ] = 1.f;
    o_sampMir.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_sampMir.Filter = D3D11_FILTER_ANISOTROPIC;
    o_sampMir.MaxAnisotropy = 16;
    o_sampMir.MaxLOD = FLT_MAX;
    o_sampMir.MinLOD = -FLT_MAX;
    o_sampMir.MipLODBias = 0.f;

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
    o_sampMirLo.MipLODBias = -1.f;

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
    o_blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC o_blend2 = {};
    o_blend2.AlphaToCoverageEnable = false;
    o_blend2.IndependentBlendEnable = false;
    o_blend2.RenderTarget[ 0 ].BlendEnable = true;
    o_blend2.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    o_blend2.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    o_blend2.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_blend2.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_blend2.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_blend2.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_blend2.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC o_blend3 = {};
    o_blend3.AlphaToCoverageEnable = false;
    o_blend3.IndependentBlendEnable = false;
    o_blend3.RenderTarget[ 0 ].BlendEnable = true;
    o_blend3.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
    o_blend3.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
    o_blend3.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_blend3.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_blend3.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_blend3.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_blend3.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC o_blendGlow = {};
    o_blendGlow.AlphaToCoverageEnable = false;
    o_blendGlow.IndependentBlendEnable = false;
    o_blendGlow.RenderTarget[ 0 ].BlendEnable = true;
    o_blendGlow.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    o_blendGlow.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    o_blendGlow.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_blendGlow.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_blendGlow.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_blendGlow.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_blendGlow.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

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

    D3D11_RASTERIZER_DESC o_rsDesc2;
    o_rsDesc2.AntialiasedLineEnable = false;
    o_rsDesc2.CullMode = D3D11_CULL_FRONT;
    o_rsDesc2.DepthBias = 0;
    o_rsDesc2.DepthBiasClamp = 0.f;
    o_rsDesc2.DepthClipEnable = true;
    o_rsDesc2.FillMode = D3D11_FILL_SOLID;
    o_rsDesc2.FrontCounterClockwise = false;
    o_rsDesc2.MultisampleEnable = false;
    o_rsDesc2.ScissorEnable = false;
    o_rsDesc2.SlopeScaledDepthBias = 0.f;

    D3D11_RASTERIZER_DESC o_rsDesc3;
    o_rsDesc3.AntialiasedLineEnable = false;
    o_rsDesc3.CullMode = D3D11_CULL_NONE;
    o_rsDesc3.DepthBias = 0;
    o_rsDesc3.DepthBiasClamp = 0.f;
    o_rsDesc3.DepthClipEnable = true;
    o_rsDesc3.FillMode = D3D11_FILL_SOLID;
    o_rsDesc3.FrontCounterClockwise = false;
    o_rsDesc3.MultisampleEnable = false;
    o_rsDesc3.ScissorEnable = false;
    o_rsDesc3.SlopeScaledDepthBias = 0.f;

    CVec < SMaterial, void > o_mat( 1 );
    CVec < STex, void > o_tex( 3 );

    CVec < STex, void > o_bulbTex( 2 );
    o_bulbTex.EmplaceBack( TextureLoader::Load( "Textures/bulb.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_bulbTex.EmplaceBack( TextureLoader::Load( "Textures/bulb.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    CVec < SMaterial, void > o_bulbMat( 1 );
    o_bulbMat.EmplaceBack( &o_flat, o_flatSlice, 1, 0, std::move( o_bulbTex ), ShadersManager::AcquireByName( "billboard_glowing" ), &o_blend2, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::glowmap | RStates::depthTest | RStates::depthWrite );

    static SGeometry o_flattnHalo;
    Geometry::FlatHalo( &o_flattnHalo );
    SGeometrySlice o_flattnHaloSlice( 0, 0, o_flattnHalo.verticesCount, o_flattnHalo.indicesCount );

    o_tex.Clear();
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattnHalo, o_flattnHaloSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "particle_new_halo_intense" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target );
    //o_mat.EmplaceBack( &o_flattnHalo, o_flattnHaloSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "particle_new_halo" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::glowmap );
    Halos = new CHalos( vec3( 0, 0, 0 ), vec3( 0, 0, 0 ), vec3( 0, 0, 0 ), std::move( o_mat ), true );

    /*  grass  */

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/grass2.dds" ), &o_sampDef, vec2( 0 ), vec2( 250, 250 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/grass2_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 250, 250 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthWrite );
    Grass = new CObject( vec3( CF( -2000 ), 0, CF( -2000 ) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 4000 ), CF( 4000 ), 1 ), std::move( o_mat ) );

    /*  walk roads  */

    WalkPlatesStart = FloorPlites.Size();

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor2.dds" ), &o_sampDef, vec2( 0 ), vec2( 4, 13 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor2_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 4, 13 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor2_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( 4, 13 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target );
    FloorPlites.Append( new CObject( vec3( -500 + (2005 * 0.25), 0, 500 - (2032 * 0.25) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 4 ), CF( 13 ), 1 ), std::move( o_mat ) ) );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor2.dds" ), &o_sampDef, vec2( 0 ), vec2( 4000, 4 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor2_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 4, 13 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor2_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( 4, 13 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target );
    FloorPlites.Append( new CObject( vec3( -500 + (0 * 0.25), 0, 500 - (2036 * 0.25) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 4000 ), CF( 4 ), 1 ), std::move( o_mat ) ) );

    WalkPlatesEnd = FloorPlites.Size();

    /*  vehicle road  */

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/road.dds" ), &o_sampDef, vec2( 0 ), vec2( 1000 / 4.25, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "lightless" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target );
    Road = new CObject( vec3( -500, 0, -10.5 - 4.25 ), vec3( f32_pi / 2, 0, 0 ), vec3( 1000, 4.25, 1 ), std::move( o_mat ) );

    for( ui32 light_l2 = 0; light_l2 < FromInsideLigts.Size(); ++light_l2 )
    {
        Grass->AttachLight( FromInsideLigts[ light_l2 ] );
        for( ui32 plate = WalkPlatesStart; plate < WalkPlatesEnd; ++plate )
        {
            FloorPlites[ plate ]->AttachLight( FromInsideLigts[ light_l2 ] );
        }
    }

    const f32 base = 0.4;
    const f32 wallThickness = 0.15;
    const f32 wallInnerThickness = 0.1;

    /*  room 1  */

    Room1Ligts.Append( new CPointLight( vec3( 1.5, 2.52 + base, -1 ), f96color( 1, 1, 1 ), 5, true, true, true ) );
    Room1Ligts.Append( new CPointLight( vec3( 3.5, 2.5 + base, -3 ), f96color( 1, 1, 1 ), 5, true, true, true ) );
    Bulbs.Append( new CObject( Room1Ligts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Room1Ligts.Back()->PositionGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );

    FromInsideLigts.Append( new CPointLight( vec3( 0, 1.5 + base, -2.5 ), f96color( 1, 1, 1 ), 2, true, true, true ) );
    Bulbs.Append( new CObject( FromInsideLigts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( FromInsideLigts.Back()->PositionGet(), 150, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );
    FromInsideLigts.Append( new CPointLight( vec3( 2.5, 1.5 + base, -5 ), f96color( 1, 1, 1 ), 2, true, true, true ) );
    Bulbs.Append( new CObject( FromInsideLigts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( FromInsideLigts.Back()->PositionGet(), 150, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );

    static SGeometry lamp0Geo;
    GeoFromFile( "Meshes\\lamp2_group00.s3ascg", &lamp0Geo );
    SGeometrySlice lamp0GeoSlice( 0, 0, lamp0Geo.verticesCount, lamp0Geo.indicesCount );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &lamp0Geo, lamp0GeoSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "lightless_texturless" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::White, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Furniture.Append( new CObject( vec3( 1.55f, 2.6f + base, -0.9f ), vec3( 0, 0, 0 ), vec3( 0.35, 0.35, 0.35 ), std::move( o_mat ) ) );
    Bulbs.Append( new CObject( vec3( 1.24, 2.52 + base, -0.98f ), vec3(), vec3( 0.5947265625 * 0.04, 0.04, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Bulbs.Back()->PosGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );
    Bulbs.Append( new CObject( vec3( 1.63, 2.52 + base, -1.21f ), vec3(), vec3( 0.5947265625 * 0.04, 0.04, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Bulbs.Back()->PosGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );
    Bulbs.Append( new CObject( vec3( 1.633, 2.52 + base, -0.755f ), vec3(), vec3( 0.5947265625 * 0.04, 0.04, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Bulbs.Back()->PosGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );

    static SGeometry chairLoungeModernGeo;
    GeoFromFile( "Meshes\\chairLoungeModern.s3ascg", &chairLoungeModernGeo );
    SGeometrySlice chairLoungeModernGeoSlice( 0, 0, chairLoungeModernGeo.verticesCount, chairLoungeModernGeo.indicesCount );
    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &chairLoungeModernGeo, chairLoungeModernGeoSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_simple_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
	*Furniture.AppendNum() = new CObject( vec3( 2, 0 + base, -2 ), vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Furniture.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }

    /*o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_gmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 2, 8 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_gmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 2, 8 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "glowing" ), &o_blendGlow, &o_rsDesc, Colora::White, Colora::White, Color::Black, 1.f, RStates::target | RStates::depthTest | RStates::depthWrite | RStates::glowmap );
    FloorPlites.Append( new CObject( vec3( -500 + (2021 * 0.25), 0 + base, 500 - (2019 * 0.25) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 2 ), CF( 8 ), 1 ), std::move( o_mat ) ) );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_gmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 9, 2 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_gmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 9, 2 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "glowing" ), &o_blendGlow, &o_rsDesc, Colora::White, Colora::White, Color::Black, 1.f, RStates::target | RStates::depthTest | RStates::depthWrite | RStates::glowmap );
    FloorPlites.Append( new CObject( vec3( -500 + (2021 * 0.25), 0 + base, 500 - (2011 * 0.25) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 9 ), CF( 2 ), 1 ), std::move( o_mat ) ) );
   */
    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0.dds" ), &o_sampDef, vec2( 0 ), vec2( 30, 20 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_nmap.dds" ), &o_sampDefLo, vec2( 0 ), vec2( 30, 20 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_smap.dds" ), &o_sampDefLo, vec2( 0 ), vec2( 30, 20 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    FloorPlites.Append( new CObject( vec3( -500 + (2000 * 0.25), 0 + base, 500 - (2019 * 0.25) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 30 ), CF( 20 ), 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        FloorPlites.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall2.dds" ), &o_sampMir, vec2( 0 ), vec2( 1.7, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall2_nmap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 1.7, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall2_smap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 1.7, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 0, 0 + base, 0.25 ), vec3( 0, 0, 0 ), vec3( 7.5, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 0, base, 0.25 ), vec3( 0, 0, 1 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc2, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 0, 2.75 + base, 0.25 + wallThickness ), vec3( -f32_pi / 2, 0, 0 ), vec3( 11.5, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 0, base, 0.25 ), vec3( 0, 0, 1 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc2, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 0, 0 + base, 0.25 + wallThickness ), vec3( -f32_pi / 2, 0, 0 ), vec3( 11.5, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall2.dds" ), &o_sampMir, vec2( 0 ), vec2( 1.136, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall2_nmap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 1.7, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall2_smap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 1.7, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 0, 0 + base, -4.75 ), vec3( 0, f32_pi + f32_pi / 2, 0 ), vec3( 5, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 0, base, -4.75 ), vec3( -1, 0, 0 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc2, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( -wallThickness, 2.75 + base, -4.75 ), vec3( -f32_pi / 2, f32_pi + f32_pi / 2, 0 ), vec3( 5 + wallThickness, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 0, base, -4.75 ), vec3( -1, 0, 0 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc2, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 0, 0 + base, -4.75 ), vec3( 0, f32_pi, 0 ), vec3( wallThickness, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 0, base, -4.75 ), vec3( -1, 0, 0 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( -wallThickness, 0 + base, -4.75 - wallThickness ), vec3( f32_pi / 2, 0, 0 ), vec3( 3.75 + wallThickness, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    for( ui32 light_l2 = 0; light_l2 < Room2Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room2Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc2, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( -wallThickness, 0 + base, -4.75 ), vec3( -f32_pi / 2, f32_pi + f32_pi / 2, 0 ), vec3( 5 + wallThickness, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    for( ui32 light_l2 = 0; light_l2 < Room2Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room2Ligts[ light_l2 ] );
    }

    /*  room 2  */

    Room2Ligts.Append( new CPointLight( vec3( 5.625, 2.5 + base, -6.24 ), f96color( 1, 1, 1 ), 4, true, true, true ) );
    Bulbs.Append( new CObject( Room2Ligts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Room2Ligts.Back()->PositionGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );
    Room2Ligts.Append( new CPointLight( vec3( 5.625, 2.5 + base, -6.26 ), f96color( 1, 1, 1 ), 4, true, true, true ) );
    Bulbs.Append( new CObject( Room2Ligts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Room2Ligts.Back()->PositionGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor1.dds" ), &o_sampDef, vec2( 0 ), vec2( 15, 10 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor1_nmap.dds" ), &o_sampDefLo, vec2( 0 ), vec2( 15, 10 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor1_smap.dds" ), &o_sampDefLo, vec2( 0 ), vec2( 15, 10 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    FloorPlites.Append( new CObject( vec3( -500 + (2015 * 0.25), 0 + base, 500 - (2029 * 0.25) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 15 ), CF( 10 ) - wallInnerThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room2Ligts.Size(); ++light_l2 )
    {
        FloorPlites.Back()->AttachLight( Room2Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 3.75, 0 + base, -4.75 - wallInnerThickness ), vec3( f32_pi / 2, 0, 0 ), vec3( 3.75, wallInnerThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    for( ui32 light_l2 = 0; light_l2 < Room2Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room2Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 3.75 - wallThickness, 0 + base, -4.75 - wallInnerThickness ), vec3( f32_pi / 2, f32_pi / 2, 0 ), vec3( 2.5, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    for( ui32 light_l2 = 0; light_l2 < Room2Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room2Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 3.75 - wallThickness, 0 + base, -7.25 - wallThickness ), vec3( f32_pi / 2, 0, 0 ), vec3( 7.75 + wallThickness * 2, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }
    for( ui32 light_l2 = 0; light_l2 < Room2Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room2Ligts[ light_l2 ] );
    }

    const char *const toiletExpensiveFiles[] = 
    {
        "Meshes\\toiletExpensive_group00.s3ascg"
    };
    static SGeometry toiletExpensiveGeo[ COUNTOF( toiletExpensiveFiles ) ];
    SMaterial toiletExpensiveMats[ COUNTOF( toiletExpensiveFiles ) ];
    GeoFromFilesToMaterials( toiletExpensiveFiles, toiletExpensiveMats, COUNTOF( toiletExpensiveFiles ), toiletExpensiveGeo );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    for( ui32 mat = 0; mat < COUNTOF( toiletExpensiveFiles ); ++mat )
    {
        o_mat.EmplaceBack( toiletExpensiveMats[ mat ].po_geo, toiletExpensiveMats[ mat ].o_geoSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_simple_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
        o_mat[ mat ].is_geoShaderDefined = ShadersManager::TryToBlend( o_mat[ mat ].po_geo->description, o_mat[ mat ].shader, &o_mat[ mat ].i_lo );
    }
    Furniture.Append( new CObject( vec3( 7, 0 + base, -6 ), vec3( 0, -f32_pi / 2, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room2Ligts.Size(); ++light_l2 )
    {
        Furniture.Back()->AttachLight( Room2Ligts[ light_l2 ] );
    }

    /*  room 3  */

    Room3Ligts.Append( new CPointLight( vec3( 9, 2.5 + base, -2 ), f96color( 1, 1, 1 ), 5, true, true, true ) );
    Bulbs.Append( new CObject( Room3Ligts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Room3Ligts.Back()->PositionGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );
    Room3Ligts.Append( new CPointLight( vec3( 9, 2.5 + base, -5 ), f96color( 1, 1, 1 ), 5, true, true, true ) );
    Bulbs.Append( new CObject( Room3Ligts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Room3Ligts.Back()->PositionGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor3.dds" ), &o_sampDef, vec2( 0 ), vec2( 16, 30 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor3_nmap.dds" ), &o_sampDefLo, vec2( 0 ), vec2( 16, 30 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor3_smap.dds" ), &o_sampDefLo, vec2( 0 ), vec2( 16, 30 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    FloorPlites.Append( new CObject( vec3( -500 + (2030 * 0.25) + wallInnerThickness, 0 + base, 500 - (2029 * 0.25) ), vec3( f32_pi / 2, 0, 0 ), vec3( CF( 16 ) - wallInnerThickness, CF( 30 ), 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        FloorPlites.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0.dds" ), &o_sampMir, vec2( 0 ), vec2( 0.87, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0_nmap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 0.87, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0_smap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 0.87, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 7.5 + wallInnerThickness, 0 + base, 0.25 ), vec3( 0, 0, 0 ), vec3( 4 - wallInnerThickness, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 7.5, base, 0.25 ), vec3( 0, 0, 1 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0.dds" ), &o_sampMir, vec2( 0 ), vec2( 1.6363, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0_nmap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 1.6363, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0_smap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 1.6363, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 11.5, 0 + base, 0.25 ), vec3( 0, f32_pi / 2, 0 ), vec3( 7.5, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 11.5, base, 0.25 ), vec3( 1, 0, 0 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0.dds" ), &o_sampMir, vec2( 0 ), vec2( 0.87, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0_nmap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 0.87, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/wall0_smap.dds" ), &o_sampMirLo, vec2( 0 ), vec2( 0.87, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 11.5, 0 + base, -7.25 ), vec3( 0, f32_pi, 0 ), vec3( 4 - wallInnerThickness, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 11.5, 2.75 + base, 0.25 + wallThickness ), vec3( f32_pi / 2, f32_pi / 2, 0 ), vec3( 7.5 + wallThickness, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 11.5, base, 0.25 ), vec3( 1, 0, 0 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc2, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 11.5 + wallThickness, 0 + base, -7.25 ), vec3( 0, -f32_pi, 0 ), vec3( wallThickness, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }
    DrawBlocker.Add( vec3( 11.5, base, -7.25 ), vec3( 1, 0, 0 ), Walls.Back() );

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
	o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc2, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 7.5 + wallInnerThickness, 0 + base, 0.25 ), vec3( 0, -f32_pi, 0 ), vec3( wallInnerThickness, 2.75, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }

    o_tex.Clear();
	o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 7.5, 0 + base, 0.25 ), vec3( f32_pi / 2, f32_pi / 2, 0 ), vec3( 7.5, wallInnerThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/dummy_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 11.5, 0 + base, 0.25 + wallThickness ), vec3( f32_pi / 2, f32_pi / 2, 0 ), vec3( 7.5 + wallThickness, wallThickness, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Room3Ligts.Size(); ++light_l2 )
    {
        Walls.Back()->AttachLight( Room3Ligts[ light_l2 ] );
    }

    /*  base  */

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_cmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 3.75 / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 3.75 / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( 3.75 / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 0 - wallThickness, 0, -4.75 - wallThickness ), vec3( 0, 0, 0 ), vec3( 3.75, base, 1 ), std::move( o_mat ) ) );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_cmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( (5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 0 - wallThickness, 0, 0.25 + wallThickness ), vec3( 0, f32_pi / 2, 0 ), vec3( 5 + wallThickness * 2, base, 1 ), std::move( o_mat ) ) );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_cmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 2.5 / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 2.5 / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( 2.5 / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 3.75 - wallThickness, 0, -4.75 - wallThickness ), vec3( 0, f32_pi / 2, 0 ), vec3( 2.5, base, 1 ), std::move( o_mat ) ) );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_cmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (7.75 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (7.75 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( (7.75 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 3.75 - wallThickness, 0, -7.25 - wallThickness ), vec3( 0, 0, 0 ), vec3( 7.75 + wallThickness * 2, base, 1 ), std::move( o_mat ) ) );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_cmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (11.5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (11.5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( (11.5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 11.5 + wallThickness, 0, 0.25 + wallThickness ), vec3( 0, f32_pi, 0 ), vec3( 11.5 + wallThickness * 2, base, 1 ), std::move( o_mat ) ) );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_cmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (7.5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( (7.5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/diamond_wall_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( (7.5 + wallThickness * 2) / base, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
    Walls.Append( new CObject( vec3( 11.5 + wallThickness, 0, -7.25 - wallThickness ), vec3( 0, -f32_pi / 2, 0 ), vec3( 7.5 + wallThickness * 2, base, 1 ), std::move( o_mat ) ) );

    /*  building  */

    BuildingLigts.Append( new CPointLight( vec3( -35, 10.5, -12 ), f96color( 1, 1, 1 ), 100, true, true, true ) );
    Bulbs.Append( new CObject( BuildingLigts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( BuildingLigts.Back()->PositionGet(), BuildingLigts.Back()->PowerGet(), f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );
    /*BuildingLigts.Append( new CPointLight( vec3( -55, 12.5, -5 ), f96color( 1, 1, 1 ), 300, true, true, true ) );
    Bulbs.Append( new CObject( BuildingLigts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( BuildingLigts.Back()->PositionGet(), 250, f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );*/

    const char *const files[] = 
    {
        "Meshes\\building_group00.s3ascg",
        "Meshes\\building_group01.s3ascg",
        "Meshes\\building_group02.s3ascg",
        "Meshes\\building_group03.s3ascg",
        "Meshes\\building_group04.s3ascg",
        "Meshes\\building_group05.s3ascg",
        "Meshes\\building_group06.s3ascg",
        "Meshes\\building_group07.s3ascg",
        "Meshes\\building_group08.s3ascg",
        "Meshes\\building_group09.s3ascg",
        "Meshes\\building_group10.s3ascg",
        "Meshes\\building_group11.s3ascg",
        "Meshes\\building_group12.s3ascg",
        "Meshes\\building_group13.s3ascg",
        "Meshes\\building_group14.s3ascg",
        "Meshes\\building_group15.s3ascg",
        "Meshes\\building_group16.s3ascg",
        "Meshes\\building_group17.s3ascg",
        "Meshes\\building_group18.s3ascg",
        "Meshes\\building_group19.s3ascg",
        "Meshes\\building_group20.s3ascg",
        "Meshes\\building_group21.s3ascg",
        "Meshes\\building_group22.s3ascg",
        //"Meshes\\building_group23.s3ascg",
        "Meshes\\building_group24.s3ascg",
        "Meshes\\building_group25.s3ascg",
        "Meshes\\building_group26.s3ascg",
        "Meshes\\building_group27.s3ascg"
    };
    static SGeometry building[ COUNTOF( files ) ];
    SMaterial dummyMats1[ COUNTOF( files ) ];
    GeoFromFilesToMaterials( files, dummyMats1, COUNTOF( files ), building );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    for( ui32 mat = 0; mat < COUNTOF( files ); ++mat )
    {
        o_mat.EmplaceBack( dummyMats1[ mat ].po_geo, dummyMats1[ mat ].o_geoSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_simple_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
        o_mat[ mat ].is_geoShaderDefined = ShadersManager::TryToBlend( o_mat[ mat ].po_geo->description, o_mat[ mat ].shader, &o_mat[ mat ].i_lo );
    }
    Buildings.Append( new CObject( vec3( -50, 0.5, 0 ), vec3( 0, f32_pi, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < BuildingLigts.Size(); ++light_l2 )
    {
        Buildings.Back()->AttachLight( BuildingLigts[ light_l2 ] );
    }

    ///*  building 2  */

    Building2Ligts.Append( new CPointLight( vec3( 35, 10.5, -12 ), f96color( 1, 1, 1 ), 100, true, true, true ) );
    Bulbs.Append( new CObject( Building2Ligts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Building2Ligts.Back()->PositionGet(), Building2Ligts.Back()->PowerGet(), f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );

    const char *const files2[] = 
    {
        "Meshes\\building_2 (1).s3ascg",
        "Meshes\\building_2 (2).s3ascg",
        "Meshes\\building_2 (3).s3ascg",
        "Meshes\\building_2 (4).s3ascg",
        "Meshes\\building_2 (5).s3ascg",
        "Meshes\\building_2 (6).s3ascg",
        "Meshes\\building_2 (7).s3ascg",
        "Meshes\\building_2 (8).s3ascg",
        "Meshes\\building_2 (9).s3ascg",
        "Meshes\\building_2 (10).s3ascg",
        "Meshes\\building_2 (11).s3ascg",
        "Meshes\\building_2 (12).s3ascg",
        "Meshes\\building_2 (13).s3ascg",
        "Meshes\\building_2 (14).s3ascg",
        "Meshes\\building_2 (15).s3ascg",
        "Meshes\\building_2 (16).s3ascg",
        "Meshes\\building_2 (17).s3ascg",
        "Meshes\\building_2 (18).s3ascg",
        "Meshes\\building_2 (19).s3ascg",
        "Meshes\\building_2 (20).s3ascg",
        "Meshes\\building_2 (21).s3ascg",
        "Meshes\\building_2 (22).s3ascg",
        "Meshes\\building_2 (23).s3ascg",
        "Meshes\\building_2 (24).s3ascg",
        "Meshes\\building_2 (25).s3ascg",
        "Meshes\\building_2 (26).s3ascg",
        "Meshes\\building_2 (27).s3ascg",
        "Meshes\\building_2 (28).s3ascg",
        "Meshes\\building_2 (29).s3ascg",
        "Meshes\\building_2 (30).s3ascg"
    };
    static SGeometry building2[ COUNTOF( files2 ) ];
    SMaterial dummyMats2[ COUNTOF( files2 ) ];
    GeoFromFilesToMaterials( files2, dummyMats2, COUNTOF( files2 ), building2 );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    for( ui32 mat = 0; mat < COUNTOF( files2 ); ++mat )
    {
        o_mat.EmplaceBack( dummyMats2[ mat ].po_geo, dummyMats2[ mat ].o_geoSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_simple_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
        o_mat[ mat ].is_geoShaderDefined = ShadersManager::TryToBlend( o_mat[ mat ].po_geo->description, o_mat[ mat ].shader, &o_mat[ mat ].i_lo );
    }

    Buildings.Append( new CObject( vec3( 50, 1.25, 10 ), vec3( 0, f32_pi, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Building2Ligts.Size(); ++light_l2 )
    {
        Buildings.Back()->AttachLight( Building2Ligts[ light_l2 ] );
    }

    /*  building 3  */

    Building3Ligts.Append( new CPointLight( vec3( 135, 10.5, -12 ), f96color( 1, 1, 1 ), 100, true, true, true ) );
    Bulbs.Append( new CObject( Building3Ligts.Back()->PositionGet(), vec3(), vec3( 0.5947265625 * 0.1, 0.1, 1 ), std::move( o_bulbMat ) ) );
    Halos->Append( Building3Ligts.Back()->PositionGet(), Building3Ligts.Back()->PowerGet(), f128color( 1, 1, 0.3, 1 ), Bulbs.Back() );

    const char *const files3[] = 
    {
        "Meshes\\building_3 (1).s3ascg",
        "Meshes\\building_3 (2).s3ascg",
    };
    static SGeometry building3[ COUNTOF( files3 ) ];
    SMaterial dummyMats3[ COUNTOF( files3 ) ];
    GeoFromFilesToMaterials( files3, dummyMats3, COUNTOF( files3 ), building3 );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/white.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    for( ui32 mat = 0; mat < COUNTOF( files3 ); ++mat )
    {
        o_mat.EmplaceBack( dummyMats3[ mat ].po_geo, dummyMats3[ mat ].o_geoSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_simple_l1" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1f, RStates::target | RStates::depthTest | RStates::depthWrite );
        o_mat[ mat ].is_geoShaderDefined = ShadersManager::TryToBlend( o_mat[ mat ].po_geo->description, o_mat[ mat ].shader, &o_mat[ mat ].i_lo );
    }
    Buildings.Append( new CObject( vec3( 135, 8, 10 ), vec3( 0, f32_pi, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) ) );
    for( ui32 light_l2 = 0; light_l2 < Building3Ligts.Size(); ++light_l2 )
    {
        Buildings.Back()->AttachLight( Building3Ligts[ light_l2 ] );
    }

    /*  fence  */

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures\\fence2.dds" ), &o_sampDef, vec2( 0, 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "lightless_discard" ), &o_blend2, &o_rsDesc3, Colora::White, Colora::White, Color::Black, 0, RStates::target | RStates::depthTest | RStates::depthWrite );
    Fence = new CObject( vec3( 2, 2, -4 ), vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) );

    /*  matrix  */

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_nmap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/floor0_smap.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    static SGeometry matrixGeos[ 2 ];
    o_mat.Clear();
    o_mat.EmplaceBack( &matrixGeos[ 0 ], SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_matrix_renderer" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0, RStates::target | RStates::depthTest | RStates::depthWrite );
    o_mat.EmplaceBack( &matrixGeos[ 1 ], SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_matrix_so" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0, RStates::nothing );
    const i32 width = 25, height = 50, depth = 1;
    vec3 o_matrixSize( 0.01, 0.01, 0.01 );
    Matrices.Append( new CMatrix( vec3( -o_matrixSize.x * width + 2, 0.85 + base, -o_matrixSize.z * height - 1.9 ), vec3( 0, 0, 0 ), o_matrixSize, std::move( o_mat ), width, height, depth, 0.5f, 0.1, 0.5, true ) );
    for( ui32 light_l2 = 0; light_l2 < Room1Ligts.Size(); ++light_l2 )
    {
        Matrices.Back()->AttachLight( Room1Ligts[ light_l2 ] );
    }

    /*  matrix 2  */

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/vinyl_glowmap3.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    static SGeometry matrix2Geos[ 2 ];
    o_mat.Clear();
    o_mat.EmplaceBack( &matrix2Geos[ 0 ], SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_matrix_renderer2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0, RStates::target | RStates::depthTest | RStates::depthWrite | RStates::glowmap );
    o_mat.EmplaceBack( &matrix2Geos[ 1 ], SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_matrix_so2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1, RStates::nothing );
    const i32 width2 = 3, height2 = 2, depth2 = 50;
    vec3 o_matrixSize2( 0.2, 0.5, 0.2 );
    Matrices.Append( new CMatrix( vec3( -o_matrixSize2.x * width2, 0.2, -o_matrixSize2.z * height2 - 11.5 ), vec3( 0, 0, f32_pi / 2 ), o_matrixSize2, std::move( o_mat ), width2, height2, depth2, 500, 1, 5, false ) );

    /*  matrix 3  */

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/black.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/vinyl_glowmap3.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    static SGeometry matrix3Geos[ 2 ];
    o_mat.Clear();
    o_mat.EmplaceBack( &matrix3Geos[ 0 ], SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_matrix_renderer2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0, RStates::target | RStates::depthTest | RStates::depthWrite | RStates::glowmap );
    o_mat.EmplaceBack( &matrix3Geos[ 1 ], SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "light_matrix_so2" ), &o_blend, &o_rsDesc, Colora::White, Colora::White, Color::Black, 0.1, RStates::nothing );
    const i32 width3 = 3, height3 = 2, depth3 = 50;
    vec3 o_matrixSize3( 0.2, 0.5, 0.2 );
    Matrices.Append( new CMatrix( vec3( -o_matrixSize3.x * width3, 0.2, -o_matrixSize3.z * height3 - 13 ), vec3( f32_pi, 0, f32_pi / 2 ), o_matrixSize3, std::move( o_mat ), width3, height3, depth3, 500, 1, 5, false ) );

    ///*  particle test  */
    //{
    //const ui32 c_count = 2000000;

    //struct SVer : CharPOD
    //{
    //    vec3 o_pos;
    //    vec3 o_speed;
    //    f32 size;
    //};
    //CVec < SVer > vertices( c_count, c_count );
    //for( ui32 index = 0; index < c_count; ++index )
    //{
    //    const f32 range = 2000.f;
    //    const f32 speedScale = 0.5f;
    //    vertices[ index ].o_pos = vec3( Funcs::RandomRangeF32( -range, range ), Funcs::RandomRangeF32( 0, range ), Funcs::RandomRangeF32( -range, range ) );
    //    vertices[ index ].o_speed = vec3( Funcs::RandomRangeF32( -50 * speedScale, 50 * speedScale ), Funcs::RandomRangeF32( -50 * speedScale, -10 * speedScale ), Funcs::RandomRangeF32( -50 * speedScale, 50 * speedScale ) );
    //    const f32 width = 0.5f;
    //    const f32 height = 0.5f;
    //    *(ui32 *)&vertices[ index ].size = (*(ui32 *)&width << 1) & 0xFFFF0000;
    //    *(ui32 *)&vertices[ index ].size |= (*(ui32 *)&height >> 15) & 0xFFFF;
    //}

    //static SGeometry o_particlesDraw, o_particlesStreamOutput;
    //Geometry::TriCircle( &o_particlesDraw, 0.5f );

    //static const D3D11_INPUT_ELEMENT_DESC sca_particlesDrawDesc[] =
    //{
    //    { "PPOSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    //    { "PSPEEDSIZE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    //};

    //o_particlesDraw.description = LayoutManager::ShaderInputDescAdd( o_particlesDraw.description, sca_particlesDrawDesc, COUNTOF( sca_particlesDrawDesc ) );
    //o_particlesDraw.ComputeXAABB = 0;

    //static const D3D11_INPUT_ELEMENT_DESC sca_Pos3DSize3DSize2DDesc[] =
    //{
    //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //    { "SPEEDSIZE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    //};

    //Geometry::StreamData( &o_particlesStreamOutput, sca_Pos3DSize3DSize2DDesc, COUNTOF( sca_Pos3DSize3DSize2DDesc ), &vertices[ 0 ], c_count, sizeof(SVer) );

    //o_tex.Clear();
    //o_tex.EmplaceBack( TextureLoader::Load( "Textures/vinyl_glowmap_circle.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    //o_tex.EmplaceBack( TextureLoader::Load( "Textures/vinyl_glowmap_circle.dds" ), &o_sampDef, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    //o_mat.Clear();
    //o_mat.EmplaceBack( &o_particlesDraw, SGeometrySlice( 0, 0, 3, 0 ), c_count, 0, std::move( o_tex ), ShadersManager::AcquireByName( "particle_instanced_glowing_discard" ), &o_blend2, &o_rsDesc3, Colora::White, Colora::White, Color::Black, 0, RStates::target | RStates::depthTest | RStates::glowmap );
    //o_mat.EmplaceBack( &o_particlesStreamOutput, SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "particle_stream_snow" ), &o_blend2, &o_rsDesc3, Colora::White, Colora::White, Color::Black, 0, RStates::nothing );
    //ParticleSystems.Append( new CParticleSystemGPU( vec3( 0, 0, 0 ), vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) ) );
    //}
    /*  particle test 2  */
    {
    const ui32 c_count = 2000;

    struct SVer : CharPOD
    {
        vec3 pos;
		vec3 startPos;
        vec2 lifeTime;
        vec3 speed;
        ui32 size;
        vec4 color;
    };
    CVec < SVer, void > vertices( c_count, c_count );
    vec3 boxMin( 7.5 + wallInnerThickness, base, -7.25 );
    vec3 boxMax( 11.5, 2.75 + base, 0.25 );
    for( ui32 index = 0; index < c_count; ++index )
    {
        vertices[ index ].startPos = vertices[ index ].pos = vec3( Funcs::RandomRangeF32( boxMin.x, boxMax.x ), Funcs::RandomRangeF32( boxMin.y, boxMax.y ), Funcs::RandomRangeF32( boxMin.z, boxMax.z ) );
        vertices[ index ].speed = vec3( 0, Funcs::RandomRangeF32( (boxMin.y - boxMax.y) / 9, (boxMin.y - boxMax.y) / 20 ), 0 );
        vertices[ index ].lifeTime.x = 0;
        vertices[ index ].lifeTime.y = Funcs::RandomRangeF32( 3, 10 );
        const f32 width = 0.05f;
        const f32 height = 0.05f;
        vertices[ index ].size = (*(ui32 *)&width << 1) & 0xFFFF0000;
        vertices[ index ].size |= (*(ui32 *)&height >> 15) & 0xFFFF;
        do
        {
            vertices[ index ].color = vec4( Funcs::RandomF32(), Funcs::RandomF32(), Funcs::RandomF32(), 1 );
        } while( vertices[ index ].color.r + vertices[ index ].color.g + vertices[ index ].color.b < 1.5f );
    }

    static SGeometry o_particlesDraw, o_particlesStreamOutput;
    Geometry::TriCircle( &o_particlesDraw, 0.5f );

    static const VertexBufferFieldDesc sca_particlesInctanceDesc[] =
    {
        { "PPOSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 1 },
        //{ "STARTPOSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "LIFETIME0", DXGI_FORMAT_R32G32_FLOAT, 24, 1 },
        { "PSPEEDSIZE0", DXGI_FORMAT_R32G32B32A32_FLOAT, 32, 1 },
        { "PCOLOR0", DXGI_FORMAT_R32G32B32A32_FLOAT, 48, 1 }
    };

    LayoutsManager::BufferDesc_t particlesInstanceCompiledDesc = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( sca_particlesInctanceDesc ) );
    o_particlesDraw.description = RendererGlobals::DefLayoutsManager.UniteCompiledBufferDescs( CVec < LayoutsManager::BufferDesc_t >( { o_particlesDraw.description, particlesInstanceCompiledDesc } ) );
    o_particlesDraw.ComputeXAABB = 0;

    static const VertexBufferFieldDesc sca_Pos3DSize3DSize2DDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "STARTPOSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 12, 0 },
        { "LIFETIME0", DXGI_FORMAT_R32G32_FLOAT, 24, 0 },
        { "SPEEDSIZE0", DXGI_FORMAT_R32G32B32A32_FLOAT, 32, 0 },
        //{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    Geometry::StreamData( &o_particlesStreamOutput, MakeRefVec( sca_Pos3DSize3DSize2DDesc ), &vertices[ 0 ], c_count, sizeof(SVer) );

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/violet_circle.dds" ), &o_sampDefTri, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/violet_circle.dds" ), &o_sampDefTri, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_particlesDraw, SGeometrySlice( 0, 0, 3, 0 ), c_count, 0, std::move( o_tex ), ShadersManager::AcquireByName( "particle_instanced_glowing_discard_lifetime" ), &o_blend3, &o_rsDesc3, Colora::White, Colora::White, Color::Black, 0, RStates::target | RStates::depthTest | RStates::glowmap );
    o_mat.EmplaceBack( &o_particlesStreamOutput, SGeometrySlice(), 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "particle_stream_fall" ), &o_blend2, &o_rsDesc3, Colora::White, Colora::White, Color::Black, 0, RStates::nothing );
    ParticleSystems.Append( new CParticleSystemGPU( vec3( 0, 0, 0 ), vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) ) );

    struct SData
    {
        vec4 boxMin;
        vec4 boxMax;
    } data = { vec4( boxMin, 0 ), vec4( boxMax, 0 ) };
    ParticleSystems.Back()->SetAdditionalShaderData( &data, sizeof(data), SShaderData::vertex );
    }

    /*  bloom test 1 */

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/vinyl_bloom_test.dds" ), &o_sampMir, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/vinyl_bloom_test2.dds" ), &o_sampMir, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "lightless_glowing" ), &o_blend2, &o_rsDesc, Colora::White, Colora::White, Color::Black, 1.f, RStates::target | RStates::depthTest | RStates::depthWrite | RStates::glowmap );
    Walls.Append( new CObject( vec3( 0, 2, -5 ), vec3( 0, 0, 0 ), vec3( 2.5, 2.5, 1 ), std::move( o_mat ) ) );

    /*  bloom test 2 */

    o_tex.Clear();
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/twi.dds" ), &o_sampMir, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_tex.EmplaceBack( TextureLoader::Load( "Textures/twi2.dds" ), &o_sampMir, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 );
    o_mat.Clear();
    o_mat.EmplaceBack( &o_flattn, o_flattnSlice, 1, 0, std::move( o_tex ), ShadersManager::AcquireByName( "lightless_glowing" ), &o_blend2, &o_rsDesc, Colora::White, Colora::White, Color::Black, 1.f, RStates::target | RStates::depthTest | RStates::depthWrite | RStates::glowmap );
    Walls.Append( new CObject( vec3( 0, 4.5, -5 ), vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), std::move( o_mat ) ) );
}

void CheckParticleTests()
{	
	static FilePath filePath;
	static FILETIME creatingTime, modificationTime;

	if( filePath.IsEmpty() )
	{
		wchar_t moduleFN[ MAX_PATH ];
		GetModuleFileNameW( GetModuleHandleW( nullptr ), moduleFN, sizeof(moduleFN) );

		filePath = moduleFN;
		filePath.PopLevel();
		filePath += L"ParticleTestModule.dll";

		HANDLE file = CreateFileW( filePath.PlatformPath(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );
		if( file == INVALID_HANDLE_VALUE )
		{
			SENDLOG( CLogger::Tag::error, "failed to open ParticleTestModule.dll for watching\n" );
			filePath = FilePath();
			return;
		}

		if( TRUE != GetFileTime( file, &creatingTime, 0, &modificationTime ) )
		{
			SENDLOG( CLogger::Tag::error, "failed to get ParticleTestModule.dll's file times for watching\n" );
			filePath = FilePath();
			return;
		}

		CloseHandle( file );
	}

	HANDLE file = CreateFileW( filePath.PlatformPath(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );
	if( file == INVALID_HANDLE_VALUE )
	{
		SENDLOG( CLogger::Tag::error, "failed to open ParticleTestModule.dll for watching\n" );
		return;
	}

	FILETIME currentCreatingTime, currentModificationTime;

	if( TRUE != GetFileTime( file, &currentCreatingTime, 0, &currentModificationTime ) )
	{
		SENDLOG( CLogger::Tag::error, "failed to get ParticleTestModule.dll's file times for watching\n" );
		return;
	}
	
	CloseHandle( file );

	bool isCreatingTimeChanged = creatingTime.dwHighDateTime != currentCreatingTime.dwHighDateTime || creatingTime.dwLowDateTime != currentCreatingTime.dwLowDateTime;
	bool isModificationTimeChanged = modificationTime.dwHighDateTime != currentModificationTime.dwHighDateTime || modificationTime.dwLowDateTime != currentModificationTime.dwLowDateTime;

	if( isCreatingTimeChanged || isModificationTimeChanged )
	{
		TheTest::ReloadParticleTests();
		creatingTime = currentCreatingTime;
		modificationTime = currentModificationTime;
	}
}