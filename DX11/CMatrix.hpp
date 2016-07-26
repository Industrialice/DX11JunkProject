#ifndef __CMATRIX_HPP__
#define __CMATRIX_HPP__

#include "CObject.hpp"

//  must have 2 materials: first for drawing, second for stream output
class CMatrix : public CObject
{
    ui32 _width, _height, _depth;
    f32 _maxAmp;
    f32 _maxSpeed, _minSpeed;
    bln _is_visibilityOptimization;

    static SGeometry sao_geos[ 64 ];

public:
    virtual ~CMatrix() override;
    CMatrix( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats, ui32 width, ui32 height, ui32 depth, f32 amp, f32 maxSpeed, f32 minSpeed, bln is_visibleOptimization );
    void PropsSet( ui32 width, ui32 height, ui32 depth, f32 amp );
    void PropsGet( ui32 *p_width, ui32 *p_height, ui32 *p_depth, f32 *p_amp );
    virtual void Update() override;
    virtual void Draw( bln is_stepTwo ) override;

private:
    template < const bln cis_lightable > void UpdateCubes();
    void GenMatrix();
    void ChoseGeo( bln is_frontVsbl, bln is_rightVsbl, bln is_backVsbl, bln is_leftVsbl, bln is_upVsbl, bln is_downVsbl );
    static void CreateGeos();
};

#endif __CMATRIX_HPP__
