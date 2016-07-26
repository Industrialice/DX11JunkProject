#include "PreHeader.hpp"
#include "CMatrix.hpp"
#include "Globals.hpp"
#include "Camera.hpp"
#include "Bloom.hpp"
#include "ShadersManager.hpp"
#include "LayoutsManager.hpp"
#include "RendererGlobals.hpp"

SGeometry CMatrix::sao_geos[ 64 ];

namespace
{
    LayoutsManager::BufferDesc_t DescloDraw = LayoutsManager::BufferDesc_t();
    LayoutsManager::BufferDesc_t DescloStream = LayoutsManager::BufferDesc_t();
}

struct SCubeVertex
{
    vec3 o_pos;
    vec2 o_ampSpeedRange;
};

void CMatrix::CreateGeos()
{
    struct SVer
    {
        vec3 o_pos;
        vec2 o_tex;
        vec3 o_nor;
        vec3 o_tan;
    };

    static const ui16 sca_baseIndices[ 6 ] = { 0, 1, 2, 3, 0, 2 };

    static const SVer front[ 4 ] = 
    {
        { vec3( -1.f, -1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, -1.f, -1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) }
    };
    static const SVer right[ 4 ] =
    {
        { vec3( 1.f, -1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) }
    };
    static const SVer back[ 4 ] =
    {
        { vec3( 1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( -1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) }
    };
    static const SVer left[ 4 ] =
    {
        { vec3( -1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, 1.f, -1.f ), vec2( 1.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, -1.f, -1.f ), vec2( 1.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) }
    };
    static const SVer up[ 4 ] =
    {
        { vec3( -1.f, 1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) }
    };
    static const SVer down[ 4 ] =
    {
        { vec3( -1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, -1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, -1.f, -1.f ), vec2( 1.f, 0.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) }
    };

    static const SVer *all[ 6 ] = { front, right, back, left, up, down };

    ui16 indicesBuf[ 36 ];
    SVer verticesBuf[ 24 ];

    for( ui32 var = 1; var < 64; ++var )
    {
        ui32 Found = 0;
        for( ui32 test = 1, plane = 0; plane < 6; test <<= 1, ++plane )
        {
            if( var & test )
            {
                _MemCpy( indicesBuf + Found * 6, sca_baseIndices, sizeof(sca_baseIndices) );
                for( ui32 index = 0; index < 6; ++index )
                {
                    (indicesBuf + Found * 6)[ index ] += Found * 4;
                }
                _MemCpy( verticesBuf + Found * 4, all[ plane ], sizeof(SVer[ 4 ]) );
                ++Found;
            }
        }

        const D3D11_BUFFER_DESC co_bufDesc =
        {
            sizeof(SVer[ 4 ]) * Found,
            D3D11_USAGE_IMMUTABLE,
            D3D11_BIND_VERTEX_BUFFER,
            0,
            0,
            0
        };
        D3D11_SUBRESOURCE_DATA o_bufData;
        o_bufData.pSysMem = verticesBuf;
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &co_bufDesc, &o_bufData, &sao_geos[ var ].i_vbufs[ 0 ] ) );

        const D3D11_BUFFER_DESC co_ibufDesc =
        {
            sizeof(ui16[ 6 ]) * Found,
            D3D11_USAGE_IMMUTABLE,
            D3D11_BIND_INDEX_BUFFER,
            0,
            0,
            0
        };
        D3D11_SUBRESOURCE_DATA o_ibufData;
        o_ibufData.pSysMem = indicesBuf;
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &co_ibufDesc, &o_ibufData, &sao_geos[ var ].i_ibuf ) );

        static const VertexBufferFieldDesc a_Pos3DTexNorTanDesc[] =
        {
            { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
            { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 },
            { "NORMAL0", DXGI_FORMAT_R32G32B32_FLOAT, 20, 0 },
            { "TANGENT0", DXGI_FORMAT_R32G32B32_FLOAT, 32, 0 }
        };
    
        sao_geos[ var ].description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos3DTexNorTanDesc ) );
        sao_geos[ var ].verticesCount = Found * 4;
        sao_geos[ var ].topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        sao_geos[ var ].is_32bitIndices = false;
        sao_geos[ var ].ComputeXAABB = 0;
        sao_geos[ var ].strides[ 0 ] = sizeof(SVer);
        sao_geos[ var ].strides[ 1 ] = sizeof(SCubeVertex);
        sao_geos[ var ].offsets[ 0 ] = 0;
        sao_geos[ var ].offsets[ 1 ] = 0;
        sao_geos[ var ].vbufsCount = 2;
        sao_geos[ var ].instancesCount = 1;
        sao_geos[ var ].indicesCount = Found * 6;
    }
}

void CMatrix::ChoseGeo( bln is_frontVsbl, bln is_rightVsbl, bln is_backVsbl, bln is_leftVsbl, bln is_upVsbl, bln is_downVsbl )  //  private
{
    ui32 index = is_frontVsbl << 0;
    index |= is_rightVsbl << 1;
    index |= is_backVsbl << 2;
    index |= is_leftVsbl << 3;
    index |= is_upVsbl << 4;
    index |= is_downVsbl << 5;
    ID3D11Buffer *bufSaved = _o_mats[ 0 ].po_geo->i_vbufs[ 1 ];
    *_o_mats[ 0 ].po_geo = sao_geos[ index ];
    _o_mats[ 0 ].po_geo->i_vbufs[ 1 ] = bufSaved;
}

CMatrix::~CMatrix()
{
}

CMatrix::CMatrix( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats, ui32 width, ui32 height, ui32 depth, f32 amp, f32 maxSpeed, f32 minSpeed, bln is_visibleOptimization ) : CObject( o_pos, o_rot, o_size, std::move( o_mats ) )
{
    if( !sao_geos[ 1 ].vbufsCount )
    {
        CreateGeos();
    }

    if( _o_mats.Size() != 2 )
    {
        SOFTBREAK;
        for( ui32 mat = 0; mat < _o_mats.Size(); ++mat )
        {
            _o_mats[ mat ].is_inFrustum = false;
        }
        return;
    }

    _o_mats[ 0 ].is_inFrustum = true;
    _o_mats[ 0 ].o_geoSlice = SGeometrySlice( 0, 0, 0, 0 );
    *_o_mats[ 0 ].po_geo = sao_geos[ 1 ];
    _o_mats[ 1 ].is_inFrustum = false;
    _o_mats[ 1 ].is_enabled = false;

    _is_inFrustum = true;
    _width = width;
    _height = height;
    _depth = depth;
    _maxAmp = amp;
    _maxSpeed = maxSpeed;
    _minSpeed = minSpeed;
    _is_visibilityOptimization = is_visibleOptimization;
    GenMatrix();

    static const VertexBufferFieldDesc sca_matrixDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 },
        { "NORMAL0", DXGI_FORMAT_R32G32B32_FLOAT, 20, 0 },
        { "TANGENT0", DXGI_FORMAT_R32G32B32_FLOAT, 32, 0 },
        { "LOCATION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 1 },
        { "AMPPARAMS0", DXGI_FORMAT_R32G32_FLOAT, 12, 1 }
    };

    static const VertexBufferFieldDesc sca_matrixStreamDesc[] =
    {
        { "LOCATION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "AMPPARAMS0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 }
    };

    if( DescloDraw.IsNull() )
    {
        LayoutsManager::BufferDesc_t drawBuf = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( sca_matrixDesc, 4 ) );
        LayoutsManager::BufferDesc_t instanceBuf = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( sca_matrixDesc + 4, 2 ) );
        DescloDraw = RendererGlobals::DefLayoutsManager.UniteCompiledBufferDescs( CVec < LayoutsManager::BufferDesc_t >( { drawBuf, instanceBuf } ) );
    }

    if( DescloStream.IsNull() )
    {
        DescloStream = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( sca_matrixStreamDesc ) );
    }
    
    _o_mats[ 0 ].is_geoShaderDefined = ShadersManager::TryToBlend( DescloDraw, _o_mats[ 0 ].shader, &_o_mats[ 0 ].i_lo );

    _o_mats[ 1 ].is_geoShaderDefined = ShadersManager::TryToBlend( DescloStream, _o_mats[ 1 ].shader, &_o_mats[ 1 ].i_lo );
}

void CMatrix::PropsSet( ui32 width, ui32 height, ui32 depth, f32 amp )
{
    _width = width;
    _height = height;
    _depth = depth;
    _maxAmp = amp;
    GenMatrix();
}

void CMatrix::PropsGet( ui32 *p_width, ui32 *p_height, ui32 *p_depth, f32 *p_amp )
{
    DSA( p_width, _width );
    DSA( p_height, _height );
    DSA( p_depth, _depth );
    DSA( p_amp, _maxAmp );
}

template < const bln cis_lightable > NOINLINE void CMatrix::UpdateCubes() { }

void CMatrix::Update()  //  virtual
{
    CObject::Update();

    if( !_is_inFrustum )
    {
        return;
    }

    bln is_downVsbl = RendererGlobals::CurrentCamera->PositionGet().y - _o_pos.y < _maxAmp;
    bln is_upVsbl = RendererGlobals::CurrentCamera->PositionGet().y - _o_pos.y > -_maxAmp;

    vec3 o_camLook;
    LiceMath::Vec3Normalize( &o_camLook, &vec3( RendererGlobals::CurrentCamera->LookAtGet()->x, 0.f, RendererGlobals::CurrentCamera->LookAtGet()->z ) );

    f32 horFov = DEGREETORADIAN( RendererGlobals::CurrentCamera->FOVGet() / 2 );

    m3x3 mtrx;
    LiceMath::M3x3RotateY( &mtrx, horFov );
    vec3 o_camRight;
    LiceMath::Vec3TransformByM3x3( &o_camRight, &o_camLook, &mtrx );

    LiceMath::M3x3RotateY( &mtrx, -horFov );
    vec3 o_camLeft;
    LiceMath::Vec3TransformByM3x3( &o_camLeft, &o_camLook, &mtrx );

    vec3 o_frontNor = vec3( 0, 0, -1 );
    LiceMath::Vec3TransformNormalByM4x3Inplace( &o_frontNor, &_o_w );
    bln is_frontVsbl = LiceMath::Vec3Dot( &o_frontNor, &o_camRight ) < 0.f;
    is_frontVsbl |= LiceMath::Vec3Dot( &o_frontNor, &o_camLeft ) < 0.f;

    vec3 o_backNor = vec3( 0, 0, 1 );
    LiceMath::Vec3TransformNormalByM4x3Inplace( &o_backNor, &_o_w );
    bln is_backVsbl = LiceMath::Vec3Dot( &o_backNor, &o_camRight ) < 0.f;
    is_backVsbl |= LiceMath::Vec3Dot( &o_backNor, &o_camLeft ) < 0.f;

    vec3 o_leftNor = vec3( -1, 0, 0 );
    LiceMath::Vec3TransformNormalByM4x3Inplace( &o_leftNor, &_o_w );
    bln is_leftVsbl = LiceMath::Vec3Dot( &o_leftNor, &o_camRight ) < 0.f;
    is_leftVsbl |= LiceMath::Vec3Dot( &o_leftNor, &o_camLeft ) < 0.f;

    vec3 o_rightNor = vec3( 1, 0, 0 );
    LiceMath::Vec3TransformNormalByM4x3Inplace( &o_rightNor, &_o_w );
    bln is_rightVsbl = LiceMath::Vec3Dot( &o_rightNor, &o_camRight ) < 0.f;
    is_rightVsbl |= LiceMath::Vec3Dot( &o_rightNor, &o_camLeft ) < 0.f;

    Funcs::Swap( &_o_mats[ 0 ].po_geo->i_vbufs[ 1 ], &_o_mats[ 1 ].po_geo->i_vbufs[ 0 ] );

    if( _is_visibilityOptimization )
    {
        ChoseGeo( is_frontVsbl, is_rightVsbl, is_backVsbl, is_leftVsbl, is_upVsbl, is_downVsbl );
    }
    else
    {
        ChoseGeo( true, true, true, true, true, true );
    }

    _o_mats[ 0 ].o_geoSlice.indicesCount = _o_mats[ 0 ].po_geo->indicesCount;
    _o_mats[ 0 ].o_geoSlice.verticesCount = _o_mats[ 0 ].po_geo->verticesCount;
    _o_mats[ 0 ].instanceCount = _width * _height * _depth;

    //SENDLOG( CLogger::Tag::important, "visibility %b %b %b %b %b %b\n",, is_frontVsbl, is_rightVsbl, is_backVsbl, is_leftVsbl, is_upVsbl, is_downVsbl );

    if( _o_mats[ 1 ].is_geoShaderDefined )
    {
        RendererGlobals::i_ImContext->IASetInputLayout( _o_mats[ 1 ].i_lo );

        RendererGlobals::SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

        ShadersManager::ApplyShader( _o_mats[ 1 ].shader, true );

        const UINT c_offset = 0;
        const UINT c_stride = sizeof(SCubeVertex);
        RendererGlobals::i_ImContext->IASetVertexBuffers( 0, 1, &_o_mats[ 0 ].po_geo->i_vbufs[ 1 ], &c_stride, &c_offset );
        RendererGlobals::i_ImContext->SOSetTargets( 1, &_o_mats[ 1 ].po_geo->i_vbufs[ 0 ], &c_offset );

        RendererGlobals::i_ImContext->Draw( _width * _height * _depth, 0 );

        RendererGlobals::UnbindSOTargets( 1 );
        RendererGlobals::UnbindVBuffers( 0, 1 );
    }
}

void CMatrix::Draw( bln is_stepTwo )  //  virtual
{
    CObject::Draw( is_stepTwo );
}

NOINLINE void CMatrix::GenMatrix()  //  private
{
    ui32 cubes = _width * _height * _depth;
    f32 xOffset = 0.f, zOffset = 0.f;

    UniquePtr< SCubeVertex, Heap::DefaultDeleter > po_cubes = Heap::Alloc<SCubeVertex>( cubes );
    _MemZero( po_cubes.Get(), cubes * sizeof(SCubeVertex) );

    for( ui32 h = 0; h < _height; ++h )
    {
        for( ui32 w = 0; w < _width; ++w )
        {
            for( ui32 d = 0, cube = h * _width * _depth + w * _depth; d < _depth; ++d, ++cube )
            {
                //f32 ampRange = Funcs::RandomRangeF32( 0.f, _maxAmp );
                f32 ampRange = _maxAmp;
                f32 yOffset = Funcs::RandomRangeF32( -ampRange, ampRange );
                f32 ampSpeed = Funcs::RandomRangeF32( _minSpeed, _maxSpeed );

                po_cubes[ cube ].o_pos = vec3( xOffset, yOffset, zOffset );
                po_cubes[ cube ].o_ampSpeedRange = vec2( ampSpeed, ampRange );
            }

            xOffset += _o_size.x * 2;
        }
        xOffset = 0.f;
        zOffset += _o_size.z * 2;
    }

    if( _o_mats[ 0 ].po_geo->i_vbufs[ 1 ] )
    {
        _o_mats[ 0 ].po_geo->i_vbufs[ 1 ]->Release();
    }
    if( _o_mats[ 1 ].po_geo->i_vbufs[ 0 ] )
    {
        _o_mats[ 1 ].po_geo->i_vbufs[ 0 ]->Release();
    }

    const D3D11_BUFFER_DESC co_vbufDesc =
    {
        sizeof(SCubeVertex) * cubes,
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_data;
    o_data.pSysMem = po_cubes.Get();
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &co_vbufDesc, &o_data, &_o_mats[ 0 ].po_geo->i_vbufs[ 1 ] ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &co_vbufDesc, &o_data, &_o_mats[ 1 ].po_geo->i_vbufs[ 0 ] ) );
}
