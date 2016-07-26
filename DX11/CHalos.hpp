#ifndef __C_HALOS__
#define __C_HALOS__

#include "CObject.hpp"

class CHalos : public CObject
{
    struct SHalo : CharPOD
    {
        vec3 o_pos;
        f32 intensity;
        f128color o_color;
        CObjectBase *testObject;
    };
    struct SOccludeQuery
    {
        COMUniquePtr< ID3D11Query > i_query;
        ui32 index;
    };
    CVec < SHalo, void > _o_halos;
    CVec < SOccludeQuery, void > _o_oqueries;
    COMUniquePtr< ID3D11Buffer > _i_vbDraw;
    ui32 _visibleHalos;
    ui32 _visibleReserved;
    bln _is_occlude;
    COMUniquePtr< ID3D11BlendState > _i_blend;

public:
    virtual ~CHalos() override;
    CHalos( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats, bln is_occlude );
    bln IsOccludeGet() const;
    void IsOccludeSet( bln is_occlude );
    void Clear();
    void Append( const vec3 &o_pos, f32 intensity, const f128color &o_color, CObjectBase *testObject );
    void RemoveAssociated( CObjectBase *testObject );
    virtual void Update() override;
    virtual void Draw( bln is_stepTwo ) override;

private:
    void NonOccludeUpdate();
};

#endif __C_HALOS__