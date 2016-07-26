#ifndef __CPHYSICS2D_HPP__
#define __CPHYSICS2D_HPP__

#include "CObjectBase.hpp"

class CPhysics2D : public CObjectBase
{
protected:
    enum ShapeTypes { ShapeRect = 0, ShapeCircle, ShapeTriangle };
    enum ShapeBits { RectBits = BIT( 0 ), CircleBits = BIT( 1 ), TriangleBits = BIT( 2 ) };

    byte *_rawBuffer;
    ui32 _rawBufferSize, _rawBufferAllocated;
    ui32 _shapeCount[ 3 ];

public:
    struct SShape
    {
        vec2 pos;
        f32 rotation;
        vec2 velocity;
        f32 angleSpeed;
        f32 sphereRadius;
        const ui32 idBit;
        const ShapeTypes realType;

        SShape( const vec2 &pos, f32 rotation, const vec2 &velocity, f32 angleSpeed, ui32 idBit, ShapeTypes realType ) : pos( pos ), rotation( rotation ), velocity( velocity ), angleSpeed( angleSpeed ), idBit( idBit ), realType( realType )
        {
            /*  void  */
        }
    };

    struct SRect : SShape
    {
        vec2 size;

        SRect( const vec2 &pos, f32 rotation, const vec2 &velocity, f32 angleSpeed, const vec2 &size ) : SShape( pos, rotation, velocity, angleSpeed, RectBits, ShapeRect ), size( size )
        {
            sphereRadius = 0;
        }
    };

    struct SCircle : SShape
    {
        f32 radius;

        SCircle( const vec2 &pos, f32 rotation, const vec2 &velocity, f32 angleSpeed, f32 radius ) : SShape( pos, rotation, velocity, angleSpeed, CircleBits, ShapeCircle ), radius( radius )
        {
            sphereRadius = radius;
        }
    };

    struct STriangle : SShape
    {
        vec3 size;

        STriangle( const vec2 &pos, f32 rotation, const vec2 &velocity, f32 angleSpeed, const vec3 size ) : SShape( pos, rotation, velocity, angleSpeed, TriangleBits, ShapeTriangle ), size( size )
        {
            sphereRadius = 0;
        }
    };

    CPhysics2D();
    void Add( const SShape *shape );
    ui32 ShapeCount() const;
    virtual void Update() override;
    virtual void Draw( bln is_stepTwo ) override = 0;

protected:
    SShape *FindNextShape( ui32 *p_offset );
    SShape *FindNextShapeOfType( ShapeTypes type, ui32 *p_offset );
};

#endif __CPHYSICS2D_HPP__