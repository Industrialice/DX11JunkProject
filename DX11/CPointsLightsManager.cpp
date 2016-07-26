#include "PreHeader.hpp"
#include "CPointsLightsManager.hpp"
#include "StatesManagers.hpp"
#include "Bloom.hpp"
#include "Camera.hpp"
#include "ObjectsManager.hpp"

CPointsLightsManager::CPointsLightsManager()
{
    _brightScale = 15.f;
    _is_configChanged = true;

    //  TODO: dam't rasterizer state does not working
    /*SGeometry o_flatHalo;
    Geometry::FlatHalo( &o_flatHalo, 0.5 );
    CObjectVector < SMaterial, void, Allocator::Simple < SMaterial >, ui8 > o_mat( 1 );
    o_mat.PushBack( SMaterial( 0, o_flatHalo.indicesCount, 0, o_flatHalo.verticesCount, 0, 0, CVector < STex, void, Allocator::Simple < STex >, ui8, true, false >(), ShadersManager::AcquireByName( "particle_new_halo" ), &D3D11_BLEND_DESC(), &D3D11_RASTERIZER_DESC(), Colora::White, Colora::White, Color::Black, 0.f, false, true, true, true ) );
    _po_halos = new CHalos( vec3( 0, 0, 0 ), vec3( 0, 0, 0 ), vec3( 1, 1, 1 ), o_mat, &o_flatHalo, false );
    _po_halos->IsLightableSet( false );
    ObjectsManager::Add( _po_halos );*/
}

void CPointsLightsManager::AddLight( CPointLight *po_light )
{
    _o_lights.Append( po_light );
    _is_configChanged = true;
}

void CPointsLightsManager::RemoveLight( CPointLight *po_light )
{
    for( ui32 index = 0; index < _o_lights.Size(); ++index )
    {
        if( _o_lights[ index ] == po_light )
        {
            _o_lights.Erase( index, 1 );
            _is_configChanged = true;
            return;
        }
    }

    HARDBREAK;
}

void CPointsLightsManager::RemoveByIndex( uiw index )
{
    ASSUME( index < _o_lights.Size() );
    _o_lights.Erase( index, 1 );
    _is_configChanged = true;
}

uiw CPointsLightsManager::Size() const
{
    return _o_lights.Size();
}

CPointLight *CPointsLightsManager::LightGetByIndex( uiw index )
{
    return _o_lights[ index ];
}

void CPointsLightsManager::Attach( CObjectBase *po_object ) const
{
    po_object->RemoveLights();
    for( ui32 index = 0; index < _o_lights.Size(); ++index )
    {
        if( _o_lights[ index ]->IsLightActive() )
        {
            po_object->AttachLight( _o_lights[ index ] );
        }
    }
}

void CPointsLightsManager::Visualize()
{
    if( !_o_lights.Size() )
    {
        return;
    }

    if( _is_configChanged )
    {
        PushHalos();
    }
}

void CPointsLightsManager::PushHalos()  //  private
{
    _po_halos->Clear();

}