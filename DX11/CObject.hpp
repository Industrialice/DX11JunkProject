#ifndef __OBJECT_HPP__
#define __OBJECT_HPP__

#include "CObjectBase.hpp"
#include "Geometry.hpp"

class DX11_EXPORT CObject : public CObjectBase
{
public:
    virtual ~CObject() override;
    CObject( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats );
    virtual void Update() override;
    virtual void Draw( bln is_stepTwo ) override;
};

#endif __OBJECT_HPP__