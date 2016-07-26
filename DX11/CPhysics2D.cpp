#include "PreHeader.hpp"
#include "CPhysics2D.hpp"
#include "Globals.hpp"

namespace
{
    const ui32 ShapeSizes[ 3 ] = { sizeof(CPhysics2D::SRect), sizeof(CPhysics2D::SCircle), sizeof(CPhysics2D::STriangle) };
}

CPhysics2D::CPhysics2D() : CObjectBase( vec3(), vec3(), vec3(), CVec < SMaterial, void >() )
{
    _rawBuffer = Heap::Alloc<byte>( 0 );
    _rawBufferAllocated = 0;
    _rawBufferSize = 0;
    _MemZero( _shapeCount, sizeof(_shapeCount) );
}

void CPhysics2D::Add( const SShape *shape )
{
    ui32 size = ShapeSizes[ shape->realType ];
    ++_shapeCount[ shape->realType ];
    if( _rawBufferSize + size + (DATA_ALIGNMENT - 1) > _rawBufferAllocated )
    {
        _rawBufferAllocated = _rawBufferSize + size + (DATA_ALIGNMENT - 1);
        _rawBuffer = Heap::Realloc( _rawBuffer, _rawBufferAllocated );
    }
    byte *mem = _rawBuffer + _rawBufferSize;
    byte *alignedMem = (byte *)MEMALIGN( mem, DATA_ALIGNMENT );
    _MemCpy( alignedMem, shape, size );
    _rawBufferSize += size + (alignedMem - mem);
}

ui32 CPhysics2D::ShapeCount() const
{
    return _shapeCount[ ShapeRect ] + _shapeCount[ ShapeCircle ] + _shapeCount[ ShapeTriangle ];
}

void CPhysics2D::Update()  //  virtual
{
    SShape *shape;
    for( ui32 offset = 0; shape = (SShape *)FindNextShape( &offset ); )
    {
        shape->rotation += shape->angleSpeed * Globals::DT;
        vec2 dtSpeed;
        LiceMath::Vec2Scale( &dtSpeed, &shape->velocity, Globals::DT );
        LiceMath::Vec2AdditionInplace( &shape->pos, &dtSpeed );
        if( shape->pos.x < -Globals::Width )
        {
            shape->velocity.x = -shape->velocity.x;
            shape->pos.x = -Globals::Width + 0.00001;
        }
        else if( shape->pos.x > Globals::Width )
        {
            shape->velocity.x = -shape->velocity.x;
            shape->pos.x = Globals::Width - 0.00001;
        }
        if( shape->pos.y < -Globals::Height )
        {
            shape->velocity.y = -shape->velocity.y;
            shape->pos.y = -Globals::Height + 0.00001;
        }
        else if( shape->pos.y > Globals::Height )
        {
            shape->velocity.y = -shape->velocity.y;
            shape->pos.y = Globals::Height - 0.00001;
        }
    }
}

CPhysics2D::SShape *CPhysics2D::FindNextShape( ui32 *p_offset )  //  protected
{
    for( ; ; )
    {
        if( *p_offset < _rawBufferSize )
        {
            byte *mem = _rawBuffer + *p_offset;
            byte *alignedMem = (byte *)MEMALIGN( mem, DATA_ALIGNMENT );
            SShape *shape = (SShape *)alignedMem;
            *p_offset += ShapeSizes[ shape->realType ] + (alignedMem - mem);
             return shape;
        }
        else
        {
            return 0;
        }
    }
    UNREACHABLE;
}

CPhysics2D::SShape *CPhysics2D::FindNextShapeOfType( ShapeTypes type, ui32 *p_offset )  //  protected
{
    for( ; ; )
    {
        SShape *shape = FindNextShape( p_offset );
        if( !shape )
        {
            return 0;
        }
        if( shape->realType == type )
        {
            return shape;
        }
    }
    UNREACHABLE;
}