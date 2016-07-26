#include "PreHeader.hpp"
#include "Map.hpp"
#include "ObjectsManager.hpp"
#include "Geometry.hpp"
#include "TextureLoader.hpp"
#include "Globals.hpp"
#include "Camera.hpp"
//#include "CPointsLightsManager.hpp"
#include "LayoutManager.hpp"
#include "CMatrix.hpp"
#include "CHalos.hpp"
#include "CPhysics2DDrawer.hpp"

#define OBJECTS_COUNT 250

namespace 
{
    CObject *po_Water;
    CObject *apo_Box[ OBJECTS_COUNT ];
    //CPointsLightsManager *po_PLM;
    SGeometry o_ParticlesStreamed, o_Particles;
    CObject *po_ParticleSystemStreamed, *po_ParticleSystem;
    ID3D11Buffer *ai_ParticleVBuf[ 2 ];
    ID3D11Buffer *ai_ParticleDrawVBuf[ 2 ];
}

#pragma optimize( "s", on )

void Map::Create()
{
    //po_PLM = new CPointsLightsManager();

    //SGeometry o_box;
    //Geometry::BoxTNIndexed( &o_box );
    //SGeometry o_flat;
    //Geometry::FlatTN( &o_flat );

    //D3D11_SAMPLER_DESC o_samp;
    //o_samp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    //o_samp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    //o_samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    //o_samp.BorderColor[ 0 ] = 1.f;
    //o_samp.BorderColor[ 1 ] = 1.f;
    //o_samp.BorderColor[ 2 ] = 1.f;
    //o_samp.BorderColor[ 3 ] = 1.f;
    //o_samp.ComparisonFunc = D3D11_COMPARISON_NEVER;
    //o_samp.Filter = D3D11_FILTER_ANISOTROPIC;
    //o_samp.MaxAnisotropy = 16;
    //o_samp.MaxLOD = FLT_MAX;
    //o_samp.MinLOD = -FLT_MAX;
    //o_samp.MipLODBias = 0.f;
    //
    //D3D11_BLEND_DESC o_blend = {};
    //o_blend.AlphaToCoverageEnable = false;
    //o_blend.IndependentBlendEnable = false;
    //o_blend.RenderTarget[ 0 ].BlendEnable = true;
    //o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    //o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    //o_blend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    //o_blend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    //o_blend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    //o_blend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    //o_blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;

    //CObjectVector < SMaterial, void, Allocator::Simple < SMaterial >, ui8 > o_mat( 1 );
    //CVector < STex, void, Allocator::Simple < STex >, ui8, true, false > o_tex( 3 );
    //o_samp.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    //o_samp.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

    //o_blend.RenderTarget[ 0 ].BlendEnable = false;

    //for( ui32 box = 0; box < OBJECTS_COUNT; ++box )
    //{
    //    vec3 o_pos;
    //    bln is_collisions;
    //    ui32 limit = 0;
    //    do
    //    {
    //        if( limit > OBJECTS_COUNT * OBJECTS_COUNT )
    //        {
    //            break;
    //        }
    //        is_collisions = false;
    //        o_pos = vec3( Funcs::RandomRangeF32( -12, 12 ), Funcs::RandomRangeF32( 7, 22 ), Funcs::RandomRangeF32( -1, -10 ) );
    //        for( ui32 index = 0; index < box; ++index )
    //        {
    //            f32 dist = LiceMath::Vec3Distance( &o_pos, &apo_Box[ index ]->PosGet() );
    //            if( dist < 2.f )
    //            {
    //                is_collisions = true;
    //                break;
    //            }
    //        }
    //        ++limit;
    //    } while( is_collisions );

    //    o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/white.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //    o_tex.PushBack( STex( TextureLoader::Load( "Textures/am_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //    o_tex.PushBack( STex( TextureLoader::Load( "Textures/white.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //    o_mat.Clear().PushBack( SMaterial( 0, o_box.indicesCount, 0, o_box.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_l2" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //    apo_Box[ box ] = new CObject( o_pos, vec3( Funcs::RandomRangeF32( 0, f32_pi * 2 ), Funcs::RandomRangeF32( 0, f32_pi * 2 ), Funcs::RandomRangeF32( 0, f32_pi * 2 ) ), vec3( 0.5, 0.5, 0.5 ), o_mat, &o_box );
    //    ObjectsManager::Add( apo_Box[ box ] );
    //}
    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/not_a_jade.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/not_a_jade_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/not_a_jade_smap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/not_a_jade_gmap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 0, 5, 3 ), vec3( 0, 0, 0 ), vec3( 5, 5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/code.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/code_gmap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 6, 5, 3 ), vec3( 0, 0, 0 ), vec3( 5, 5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/desktop.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/desktop_gmap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( -6, 5, 3 ), vec3( 0, 0, 0 ), vec3( 5, 5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/pad_desktop.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/pad_desktop.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( -12, 5, 3 ), vec3( 0, 0, 0 ), vec3( 5, 2.5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/umbrella_eye.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/umbrella_eye.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( -12.f, 8.5, 3 ), vec3( 0, 0, 0 ), vec3( 5, 2.5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/floor3_cmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_smap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_l2" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 0, 0, 3 ), vec3( 0, 0, 0 ), vec3( 5, 2.55, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/floor3_cmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_smap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_l2" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 5, 0, 0 ), vec3( 0, -f32_pi, 0 ), vec3( 5, 2.55, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/floor3_cmap.dds" ), &o_samp, vec2( 0 ), vec2( 3 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 3 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_smap.dds" ), &o_samp, vec2( 0 ), vec2( 3 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_l2" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 0, 0, 0 ), vec3( 0, -f32_pi / 2, 0 ), vec3( 3, 2.55, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/floor3_cmap.dds" ), &o_samp, vec2( 0 ), vec2( 3 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 3 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor3_smap.dds" ), &o_samp, vec2( 0 ), vec2( 3 / 2.55 * 5, 5 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_l2" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 5, 0, 3 ), vec3( 0, f32_pi / 2, 0 ), vec3( 3, 2.55, 1 ), o_mat, &o_flat ) );
    //ObjectsManager::AcquireByIndex( 2 )->IsLightableSet( false );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/floor_cmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 3.f * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 3.f * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor_smap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 3.f * 5, 5 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_l2" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 0, 0, 0 ), vec3( f32_pi / 2, 0, 0 ), vec3( 5, 3, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/wall_cmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 3.f * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/wall_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 3.f * 5, 5 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/wall_smap.dds" ), &o_samp, vec2( 0 ), vec2( 5 / 3.f * 5, 5 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_l2" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 0, 2.55, 3 ), vec3( -f32_pi / 2, 0, 0 ), vec3( 5, 3, 1 ), o_mat, &o_flat ) );

    //o_blend.RenderTarget[ 0 ].BlendEnable = true;
    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/bloom_test.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/bloom_test.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( -2.5, 12, 5 ), vec3( 0, 0, 0 ), vec3( 10, 1, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/bloom_test2.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/bloom_test2.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( -1.5, 14, 5 ), vec3( 0, 0, 0 ), vec3( 2, 1, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/bloom_test3.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/bloom_test3.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 2.f, 10, 5 ), vec3( 0, 0, 0 ), vec3( 2, 1, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/pony0.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/pony0.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 0, 11, 3 ), vec3( 0, 0, 0 ), vec3( 5, 5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/pony1.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/pony1.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( -2, 11, 0 ), vec3( 0, 0, 0 ), vec3( 5, 5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/pony2.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/pony2.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 2, 15, -1 ), vec3( 0, 0, 0 ), vec3( 5, 2.5, 1 ), o_mat, &o_flat ) );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/bloom_test4.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/bloom_test4.dds" ), &o_samp, vec2( 0 ), vec2( 1, 1 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "glowing" ), &o_blend, Color::White, Color::White, true, true, true, true ) );
    //ObjectsManager::Add( new CObject( vec3( 3.f, 12, 0 ), vec3( 0, 0, 0 ), vec3( 2, 1, 1 ), o_mat, &o_flat ) );

    //o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
    //o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/oilrush_ocean_waves_normal1.dds" ), &o_samp, vec2( 0, 0 ), vec2( 15 / 3.f * 5, 15 ), vec2( 0 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/oilrush_ocean_waves_normal2.dds" ), &o_samp, vec2( 0.1, 0.1 ), vec2( 15 / 3.f * 5, 15 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_flat.indicesCount, 0, o_flat.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "water" ), &o_blend, f128color( 0.5f, 0.5f, 1.f, 0.85f ), f128color( 1.f, 1.f, 1.f, 1.f ), false, true, true, true ) );
    //po_Water = new CObject( vec3( 0, 0.2, 0 ), vec3( f32_pi / 2, 0, 0 ), vec3( 5, 3, 1 ), o_mat, &o_flat );
    //po_Water->IsVisibleSet( false );
    //ObjectsManager::Add( po_Water );

    //const ui32 c_count = 3000000;

    //static struct SVer
    //{
    //    vec3 o_pos;
    //    vec3 o_speed;
    //    vec2 o_size;
    //} sao_vertices[ c_count ];
    //for( ui32 index = 0; index < c_count; ++index )
    //{
    //    sao_vertices[ index ].o_pos = vec3( Funcs::RandomRangeF32( -100, 100 ), Funcs::RandomRangeF32( -100, 100 ), Funcs::RandomRangeF32( -100, 100 ) );
    //    sao_vertices[ index ].o_speed = vec3( Funcs::RandomRangeF32( -500, 500 ), Funcs::RandomRangeF32( 0, 500 ), Funcs::RandomRangeF32( -500, 500 ) );
    //    sao_vertices[ index ].o_size = vec2( 1.f, 1.172f );
    //}

    //static const D3D11_BUFFER_DESC sco_vbufDesc =
    //{
    //    sizeof(SVer) * c_count,
    //    D3D11_USAGE_DEFAULT,
    //    D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT,
    //    0,
    //    0,
    //    0
    //};
    //D3D11_SUBRESOURCE_DATA o_bufData;
    //o_bufData.pSysMem = sao_vertices;
    //DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &ai_ParticleVBuf[ 1 ] ) );
    //DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &ai_ParticleVBuf[ 0 ] ) );

    //static const struct SVer2
    //{
    //    vec2 o_pos;
    //    vec2 o_texcoord;
    //} scao_vertices2[ 4 ] =
    //{
    //    { vec2( -0.5, -0.5 ), vec2( 0, 1 ) },
    //    { vec2( -0.5, 0.5 ), vec2( 0, 0 ) },
    //    { vec2( 0.5, -0.5 ), vec2( 1, 1 ) },
    //    { vec2( 0.5, 0.5 ), vec2( 1, 0 ) }
    //};
    //static const D3D11_BUFFER_DESC sco_vbufDesc2 =
    //{
    //    sizeof(scao_vertices2),
    //    D3D11_USAGE_IMMUTABLE,
    //    D3D11_BIND_VERTEX_BUFFER,
    //    0,
    //    0,
    //    0
    //};
    //D3D11_SUBRESOURCE_DATA o_bufData2;
    //o_bufData2.pSysMem = scao_vertices2;
    //DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc2, &o_bufData2, &ai_ParticleDrawVBuf[ 0 ] ) );
    //ai_ParticleDrawVBuf[ 1 ] = ai_ParticleVBuf[ 0 ];

    //static const D3D11_INPUT_ELEMENT_DESC sca_particlesDesc[] =
    //{
    //    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //    { "PPOSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    //    { "PSPEED", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    //    { "PSIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    //};

    //static const ui32 sca_strides[ 2 ] = { sizeof(SVer2), sizeof(SVer) };
    //static const ui32 sca_offsets[ 2 ] = { 0, 0 };
    //LayoutManager::ShaderInputDescCompile( sca_particlesDesc, COUNTOF( sca_particlesDesc ), &o_Particles.po_desc );
    //o_Particles.descLen = COUNTOF( sca_particlesDesc );
    //o_Particles.cp_strides = sca_strides;
    //o_Particles.topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    //o_Particles.verticesCount = 4;
    //o_Particles.is_streamed = false;
    //o_Particles.indicesCount = 0;
    //o_Particles.is_32bitIndices = false;
    //o_Particles.i_ibuf = 0;
    //o_Particles.ai_vbufs = ai_ParticleDrawVBuf;
    //o_Particles.pi_vsview = 0;
    //o_Particles.vsviewsCount = 0;
    //o_Particles.vbufsCount = 2;
    //o_Particles.ComputeXAABB = 0;
    //o_Particles.instancesCount = c_count;
    //o_Particles.cp_offsets = sca_offsets;
    //o_Particles.is_unbind = true;

    //static const D3D11_INPUT_ELEMENT_DESC sca_Pos3DSize3DSize2DDesc[] =
    //{
    //    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //    { "SPEED", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    //    { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    //};

    //static const ui32 sc_stride = sizeof(SVer);
    //LayoutManager::ShaderInputDescCompile( sca_Pos3DSize3DSize2DDesc, COUNTOF( sca_Pos3DSize3DSize2DDesc ), &o_ParticlesStreamed.po_desc );
    //o_ParticlesStreamed.descLen = COUNTOF( sca_Pos3DSize3DSize2DDesc );
    //o_ParticlesStreamed.cp_strides = &sc_stride;
    //o_ParticlesStreamed.verticesCount = c_count;
    //o_ParticlesStreamed.topo = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    //o_ParticlesStreamed.indicesCount = 0;
    //o_ParticlesStreamed.is_32bitIndices = false;
    //o_ParticlesStreamed.is_streamed = true;
    //o_ParticlesStreamed.i_ibuf = 0;
    //o_ParticlesStreamed.ai_vbufs = ai_ParticleVBuf;
    //o_ParticlesStreamed.pi_vsview = 0;
    //o_ParticlesStreamed.ComputeXAABB = 0;
    //o_ParticlesStreamed.vbufsCount = 2;
    //o_ParticlesStreamed.vsviewsCount = 0;
    //o_ParticlesStreamed.instancesCount = 1;
    //o_ParticlesStreamed.cp_offsets = sca_offsets;
    //o_ParticlesStreamed.is_unbind = true;

    //o_blend.RenderTarget[ 0 ].BlendEnable = false;
    //o_tex.Clear();
    //o_mat.Clear().PushBack( SMaterial( 0, o_ParticlesStreamed.indicesCount, 0, o_ParticlesStreamed.verticesCount, o_ParticlesStreamed.instancesCount, 0, o_tex, ShadersManager::AcquireByName( "particle_stream" ), &o_blend, f128color( 1.f, 1.f, 1.f, 0.85f ), f128color( 1.f, 1.f, 1.f, 1.f ), false, false, false, true ) );
    //po_ParticleSystemStreamed = new CObject( vec3( 0 ), vec3( 0 ), vec3( 0 ), o_mat, &o_ParticlesStreamed );
    //po_ParticleSystemStreamed->IsLightableSet( false );
    //po_ParticleSystemStreamed->IsVisibleSet( true );
    //ObjectsManager::Add( po_ParticleSystemStreamed );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/not_a_jade.dds" ), &o_samp, vec2( 0 ), vec2( 1 ), vec2( 0 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_Particles.indicesCount, 0, o_Particles.verticesCount, o_Particles.instancesCount, 0, o_tex, ShadersManager::AcquireByName( "particle_instanced" ), &o_blend, f128color( 1.f, 1.f, 1.f, 0.85f ), f128color( 1.f, 1.f, 1.f, 1.f ), false, true, true, true ) );
    //po_ParticleSystem = new CObject( vec3( 0 ), vec3( 0 ), vec3( 0 ), o_mat, &o_Particles );
    //po_ParticleSystem->IsLightableSet( false );
    //po_ParticleSystem->IsVisibleSet( true );
    //ObjectsManager::Add( po_ParticleSystem );

    //for( i32 w = 0; w < 4; ++w )
    //{
    //    for( i32 h = 0; h < 4; ++h )
    //    {
    //        i32 a_choice[ 3 ];
    //        do
    //        {
    //            a_choice[ 0 ] = Funcs::RandomRangeUI32( 0, 1 );
    //            a_choice[ 1 ] = Funcs::RandomRangeUI32( 0, 1 );
    //            a_choice[ 2 ] = Funcs::RandomRangeUI32( 0, 1 );
    //        } while( a_choice[ 0 ] + a_choice[ 1 ] + a_choice[ 2 ] != 1 );

    //        po_PLM->AddLight( new CPointLight( vec3( 0.1f + w * (4.8f / 3.f), 0.2, 0.1f + h * (2.8f / 3.f) ), f96color( a_choice[ 0 ], a_choice[ 1 ], a_choice[ 2 ] ), 0.5, true, true, true ) );
    //    }
    //}

    /*CPhysics2DDrawer *phys = new CPhysics2DDrawer();
    for( ui32 random = 0; random < 100; ++random )
    {
        f32 posx = Funcs::RandomRangeF32( -Globals::Width / 4, Globals::Width / 4 );
        f32 posy = Funcs::RandomRangeF32( -Globals::Height / 4, Globals::Height / 4 );
        f32 speedx = Funcs::RandomRangeF32( -200, 200 );
        f32 speedy = Funcs::RandomRangeF32( -200, 200 );
        f32 rot = Funcs::RandomRangeF32( 0, f32_pi * 2 );
        f32 angleSpeed = Funcs::RandomRangeF32( -50, 50 );
        ui32 sizex = Funcs::RandomRangeUI32( 10, 75 );
        if( rand() % 2 )
        {
            ui32 sizey = Funcs::RandomRangeUI32( 10, 75 );
            phys->Add( &CPhysics2D::SRect( vec2( posx, posy ), rot, vec2( speedx, speedy ), angleSpeed, vec2( sizex, sizey ) ) );
        }
        else
        {
            phys->Add( &CPhysics2D::SCircle( vec2( posx, posy ), rot, vec2( speedx, speedy ), angleSpeed, sizex ) );
        }
    }
    ObjectsManager::Add( phys );*/

    //SGeometry o_flatHalo;
    //Geometry::FlatHalo( &o_flatHalo, 0.5 );
    //o_tex.Clear();
    //o_mat.Clear().PushBack( SMaterial( 0, o_flatHalo.indicesCount, 0, o_flatHalo.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "particle_new_halo" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //CHalos *po_halo = new CHalos( vec3( 0, 0, 0 ), vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), o_mat, &o_flatHalo, true );
    //po_halo->IsLightableSet( false );
    //ObjectsManager::Add( po_halo );
    //
    //for( ui32 index = 0; index < 2000; ++index )
    //{
    //    const f32 coordRange = 200;

    //    f32 x, y, z;
    //    do
    //    {
    //        x = Funcs::RandomRangeF32( -coordRange, coordRange );
    //        y = Funcs::RandomRangeF32( -coordRange, coordRange );
    //        z = Funcs::RandomRangeF32( -coordRange, coordRange );
    //    } while( LiceMath::Vec3Length( &vec3( x, y, z ) ) > coordRange );
    //    //f32 z = 0;
    //    f32 r = Funcs::RandomRangeF32( 0.1, 2 );
    //    f32 rc, gc, bc;
    //    do
    //    {
    //        rc = Funcs::RandomF32();
    //        gc = Funcs::RandomF32();
    //        bc = Funcs::RandomF32();
    //    } while( rc < 0.8 && gc < 0.8 && bc < 0.8 );
    //    po_halo->PushBack( vec3( x, y, z ), r, f128color( rc, gc, bc, Funcs::RandomF32() ) );
    //}

    //po_PLM->AddLight( new CPointLight( vec3( -3, 8.48, -5.9 ), f96color( 1, 0, 0 ), 25, true, true, true ) );
    //po_PLM->AddLight( new CPointLight( vec3( 3, 8.48, -4.9 ), f96color( 0, 0, 1 ), 25, true, true, true ) );

    //SGeometry o_box2;
    //Geometry::BoxTNIndexedBottomless( &o_box2 );

    //o_tex.Clear().PushBack( STex( TextureLoader::Load( "Textures/floor_cmap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 2 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor_nmap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 2 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_tex.PushBack( STex( TextureLoader::Load( "Textures/floor_smap.dds" ), &o_samp, vec2( 0 ), vec2( 1, 2 ), vec2( 0.5, 0.5 ), 0 ) );
    //o_mat.Clear().PushBack( SMaterial( 0, o_box2.indicesCount, 0, o_box2.verticesCount, 0, 0, o_tex, ShadersManager::AcquireByName( "light_matrix_renderer" ), &o_blend, Color::White, Color::White, false, true, true, true ) );
    //const i32 width = 3162, height = 317;
    //CMatrix *po_matrix = new CMatrix( vec3( -0.1 * width, -5, -height ), vec3( 0, 0, 0 ), vec3( 0.1, 2, 1 ), o_mat, &o_box2, width, height, 2.f );
    //ObjectsManager::Add( po_matrix );

    //po_PLM->AddLight( new CPointLight( vec3( 0, 35, 0 ), f96color( 1, 1, 1 ), 35, true, true, true ) );
    ////po_PLM->AddLight( new CPointLight( vec3( 0, 0, 0 ), f96color( 1, 1, 1 ), 5, true, true, true ) );
}

#pragma optimize( "", on )

void Map::Update()
{
    //if( po_PLM->Size() > 1 )
    //{
    //    po_PLM->LightGetByIndex( 1 )->PositionSet( Camera::PositionGet() );
    //}

    //CObjectBase *obj = ObjectsManager::AcquireByIndex( ObjectsManager::Count() - 1 );
    //vec3 rot = obj->RotRadGet();
    //LiceMath::Vec3AdditionInplace( &rot, &vec3( Globals::DT ) );
    //obj->RotRadSet( rot );

    if( po_Water )
    {
        const f32 od = 5.f;
        const f32 sd = 8.f;
        SMaterial o_mat = std::move( po_Water->MaterialGet( 0 ) );
        o_mat.o_textures[ 0 ].o_texOffset = vec2( o_mat.o_textures[ 0 ].o_texOffset.x + Globals::DT / od, o_mat.o_textures[ 0 ].o_texOffset.y + Globals::DT / sd );
        o_mat.o_textures[ 1 ].o_texOffset = vec2( o_mat.o_textures[ 1 ].o_texOffset.x - Globals::DT / od / 5, o_mat.o_textures[ 1 ].o_texOffset.y - Globals::DT / sd / 5 );
        po_Water->MaterialSet( 0, std::move( o_mat ) );
    }

    for( ui32 box = 0; box < OBJECTS_COUNT; ++box )
    {
        if( !apo_Box[ box ] )
        {
            continue;
        }
        vec3 o_rot = apo_Box[ box ]->RotRadGet();
        o_rot.x = Funcs::F32NormalizeRadian( o_rot.x + Globals::DT * 0.3 );
        o_rot.y = Funcs::F32NormalizeRadian( o_rot.y + Globals::DT * 0.3 );
        o_rot.z = Funcs::F32NormalizeRadian( o_rot.z + Globals::DT * 0.3 );
        apo_Box[ box ]->RotRadSet( o_rot );
    }

    /*if( po_ParticleSystem && po_ParticleSystemStreamed )
    {
        Funcs::Swap( &ai_ParticleVBuf[ 0 ], &ai_ParticleVBuf[ 1 ] );
        SGeometry o_geo = *po_ParticleSystem->GeometryGet();
        o_geo.i_vbufs[ 1 ] = ai_ParticleVBuf[ 0 ];
        po_ParticleSystem->GeometrySet( &o_geo );
    }*/

    //ObjectsManager::Private::UpdateAll( po_PLM );
}

void Map::Draw()
{
    //ObjectsManager::Private::DrawAll();
    //po_PLM->Visualize();
}