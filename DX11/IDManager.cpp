#include "PreHeader.hpp"
#include "IDManager.hpp"

CIDManagerSequential::~CIDManagerSequential()
{
}

CIDManagerSequential::CIDManagerSequential() : _ids( 1, 0 )
{
    _firstFreeId.id = 0;
    _lastKnownFirstFreeId.id = 0;
}

NOINLINE bln CIDManagerSequential::ReserveID( id_t id )
{
    if( id == id_t_null )
    {
        return false;
    }
    if( id.id >= _ids.CellsGet() )
    {
		_ids.PushBack( 1 );
        return true;
    }
    if( _ids.Get( id.id ) )
    {
        return false;
    }
    if( _firstFreeId == id )
    {
        _firstFreeId = id_t_null;
    }
    _ids.Set( id.id, 1 );
    return true;
}

bln CIDManagerSequential::IsIDReserved( id_t id ) const
{
    if( id == id_t_null || id.id >= _ids.CellsGet() )
    {
        return false;
    }
    return _ids.Get( id.id ) != 0;
}

NOINLINE void CIDManagerSequential::FreeID( id_t id )
{
    if( id.id >= _ids.CellsGet() )
    {
        ASSUME( id == id_t_null );
        return;
    }
    _ids.Set( id.id, 0 );
    if( id.id < _firstFreeId.id )
    {
        _firstFreeId = id;
        _lastKnownFirstFreeId = id;
    }
}

NOINLINE id_t CIDManagerSequential::FindFirstFreeID()
{
    if( _firstFreeId != id_t_null )
    {
        return _firstFreeId;
    }
    for( uiw index = _lastKnownFirstFreeId.id + 1; index < _ids.CellsGet(); ++index )
    {
        if( _ids.Get( index ) == 0 )
        {
            _firstFreeId.id = index;
            _lastKnownFirstFreeId.id = index;
            return { (ui32)index };
        }
    }
    if( _ids.CellsGet() == id_t_null.id - 1 )  //  out of space
    {
        return id_t_null;
    }
    _firstFreeId.id = _ids.CellsGet();
    _lastKnownFirstFreeId.id = _ids.CellsGet();
    _ids.PushBack( 0 );
    return _firstFreeId;
}