#include "PreHeader.hpp"
#include "ObjectsManager.hpp"
#include <CVector.hpp>

namespace
{
    CVec < CObjectBase * > o_Objs;
}

void ObjectsManager::Add( CObjectBase *po_obj )
{
    o_Objs.Append( po_obj );
}

bln ObjectsManager::Remove( CObjectBase *po_obj )
{
    for( ui32 index = 0; index < o_Objs.Size(); ++index )
    {
        if( o_Objs[ index ] == po_obj )
        {
            //Object::Private::Release( po_obj );
            o_Objs[ index ] = o_Objs.Back();
            o_Objs.PopBack();
            return true;
        }
    }
    return false;
}

ui32 ObjectsManager::Count()
{
    return o_Objs.Size();
}

CObjectBase *ObjectsManager::AcquireByIndex( ui32 index )
{
    return o_Objs[ index ];
}

ui32 ObjectsManager::Index( CObjectBase *po_obj )
{
    for( ui32 index = 0; index < o_Objs.Size(); ++index )
    {
        if( o_Objs[ index ] == po_obj )
        {
            return index;
        }
    }
    return ui32_max;
}

/*void ObjectsManager::Private::UpdateAll( const CPointsLightsManager *cpo_plm )
{
    for( ui32 index = 0; index < o_Objs.Size(); ++index )
    {
        if( o_Objs[ index ]->IsLightableGet() )
        {
            cpo_plm->Attach( o_Objs[ index ] );
        }
        o_Objs[ index ]->Update();
    }
}

void ObjectsManager::Private::DrawAll()
{
    for( ui32 index = 0; index < o_Objs.Size(); ++index )
    {
        if( o_Objs[ index ]->IsInFrustumGet() && o_Objs[ index ]->IsVisibleGet() )
        {
            o_Objs[ index ]->Draw( false );
        }
    }
}*/

void ObjectsManager::Private::RemoveAll()
{
    o_Objs.Clear();
}