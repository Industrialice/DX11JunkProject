#ifndef __OBJECTS_MANAGER_HPP__
#define __OBJECTS_MANAGER_HPP__

//#include "CPointsLightsManager.hpp"
#include "CObjectBase.hpp"

namespace ObjectsManager
{
    void Add( CObjectBase *po_obj );
    bln Remove( CObjectBase *po_obj );
    ui32 Count();
    CObjectBase *AcquireByIndex( ui32 index );
    ui32 Index( CObjectBase *po_obj );

    namespace Private
    {
        //void UpdateAll( const CPointsLightsManager *cpo_plm );
        //void DrawAll();
        void RemoveAll();
    }
}

#endif __OBJECTS_MANAGER_HPP__