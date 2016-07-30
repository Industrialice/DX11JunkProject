#include "PreHeader.hpp"
#include "Camera.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"
#include <math.h>

CCamera::CCamera( f32 nearPlane, f32 farPlane, f32 fov )
{
    _o_position.x = _o_position.y = _o_position.z = 0.f;

    _o_xvec.x = 1.f;
    _o_xvec.y = 0.f;
    _o_xvec.z = 0.f;

    _o_yvec.x = 0.f;
    _o_yvec.y = 1.f;
    _o_yvec.z = 0.f;

    _o_zvec.x = 0.f;
    _o_zvec.y = 0.f;
    _o_zvec.z = 1.f;

    _camSpeed = 10.f;
    _is_upRestrict = true;
    _upRestrict = 0.95f;
    _is_downRestrict = true;
    _downRestrict = -0.95f;

    _nearPlane = nearPlane;
    _farPlane = farPlane;
    FOVSet( fov, true );
}

f32 CCamera::FOVGet() const
{
    return _fov;
}

void CCamera::FOVSet( f32 fov, bln is_remember )
{
    LiceMath::Projection( &_o_proj, fov, _nearPlane, _farPlane, (f32)RendererGlobals::RenderingWidth / (f32)RendererGlobals::RenderingHeight );
    if( is_remember )
    {
        _fov = fov;
    }
    LiceMath::M4x4Transpose( &_o_projTransp, &_o_proj );
}

const vec3 *CCamera::XVec() const
{
    return &_o_xvec;
}

const vec3 *CCamera::YVec() const
{
    return &_o_yvec;
}

const vec3 *CCamera::ZVec() const
{
    return &_o_zvec;
}

f32 CCamera::UpRestrictionGet() const
{
    return _upRestrict;
}

void CCamera::UpRestrictionSet( f32 val )
{
    _upRestrict = val;
    _is_upRestrict = val <= 1.f && val >= -1.f;
}

f32 CCamera::DownRestrictionGet() const
{
    return _downRestrict;
}

void CCamera::DownRestrictionSet( f32 val )
{
    _downRestrict = val;
    _is_downRestrict = val <= 1.f && val >= -1.f;
}

vec3 CCamera::PositionGet() const
{
    return _o_position;
}

void CCamera::PositionSet( const vec3 &o_posMeters )
{
    _o_position = o_posMeters;
}

void CCamera::LookAtSet( const vec3 &o_look )  //  TODO: fix this
{
    /*LiceMath::Vec3Subtract( &_o_zvec, &o_look, &_o_position );
    if( LiceMath::Vec3Equals... )  //  if look at the same position
    {
        _o_zvec.z = 1.f;
    }
    else
    {
        LiceMath::Vec3NormalizeInplace( &_o_zvec );
    }

    _o_yvec.x = 0.f;
    _o_yvec.y = 1.f - _o_zvec.z;
    _o_yvec.z = 0.f;

    _o_xvec.x = _o_yvec.y * _o_zvec.z;
    _o_xvec.y = 0.f;
    _o_xvec.z = _o_yvec.y * _o_zvec.x;
    LiceMath::Vec3NormalizeInplace( &_o_xvec );*/
}

const vec3 *CCamera::LookAtGet() const
{
    return &_o_zvec;
}

void CCamera::MoveX( f32 move )
{
    vec3 o_adder;
    LiceMath::Vec3Scale( &o_adder, &_o_xvec, move * _camSpeed );
    LiceMath::Vec3AdditionInplace( &_o_position, &o_adder );
}

void CCamera::MoveY( f32 move )
{
    vec3 o_adder;
    LiceMath::Vec3Scale( &o_adder, &_o_yvec, move * _camSpeed );
    LiceMath::Vec3AdditionInplace( &_o_position, &o_adder );
}

void CCamera::MoveZ( f32 move )
{
    vec3 o_adder;
    LiceMath::Vec3Scale( &o_adder, &_o_zvec, move * _camSpeed );
    LiceMath::Vec3AdditionInplace( &_o_position, &o_adder );
}

void CCamera::RotationAroundX( f32 angleDX )
{
    if( angleDX < 0.f )
    {
        if( _is_upRestrict && _o_zvec.y >= _upRestrict )
        {
            return;
        }
    }
    else if( angleDX > 0.f )
    {
        if( _is_downRestrict && _o_zvec.y <= _downRestrict )
        {
            return;
        }
    }

    vec3 o_vec;
    LiceMath::Vec3Normalize( &o_vec, &_o_xvec );
    m4x4 o_rot;
    LiceMath::M4x4RotateAxis( &o_rot, &o_vec, angleDX );
    LiceMath::Vec3TransformCoordByM4x4LastIdenInplace( &_o_zvec, &o_rot );
}

void CCamera::RotationAroundY( f32 angleDX )
{
    float e00 = ::cosf( angleDX );
    float e20 = ::sinf( angleDX );
    float e02 = -e20;
    float e22 = e00;

    float x = _o_xvec.x * e00 + _o_xvec.z * e20;
    _o_xvec.z = _o_xvec.x * e02 + _o_xvec.z * e22;
    _o_xvec.x = x;

    x = _o_zvec.x * e00 + _o_zvec.z * e20;
    _o_zvec.z = _o_zvec.x * e02 + _o_zvec.z * e22;
    _o_zvec.x = x;
}

vec3 CCamera::RotationDXGet() const
{
    //  TODO: uncomment
    /*vec3 o_z( 0.f, 0.f, 1.f ), o_x( 1.f, 0.f, 0.f ), o_y( 0.f, 1.f, 0.f );
    f32 x = ::acosf( ::D3DXVec3Dot( &_o_zvec, &o_z ) );
    f32 y = ::acosf( ::D3DXVec3Dot( &_o_xvec, &o_x ) );
    f32 z = ::acosf( ::D3DXVec3Dot( &_o_yvec, &o_y ) );

    return vec3( x, y, z );*/
    return vec3( 0, 0, 0 );
}

void CCamera::RotationDXSet( const vec3 &o_rot )  //  TODO: complete it
{
}

vec3 CCamera::RotationDegreeGet() const
{
    vec3 o_vec = RotationDXGet();
    o_vec.x = RADIANTODEGREE( o_vec.x );
    o_vec.y = RADIANTODEGREE( o_vec.y );
    o_vec.z = RADIANTODEGREE( o_vec.z );
    return o_vec;
}

void CCamera::RotationDegreeSet( const vec3 &o_rot )  //  TODO: complete it
{
}

const m4x3 *CCamera::View() const
{
    return &_o_view;
}

const m3x4 *CCamera::ViewTransposed() const
{
    return &_o_viewTransp;
}

const m4x4 *CCamera::Projection() const
{
    return &_o_proj;
}

const m4x4 *CCamera::ProjectionTransposed() const
{
    return &_o_projTransp;
}

const m4x4 *CCamera::ViewProjection() const
{
    return &_o_viewProj;
}

const m4x4 *CCamera::ViewProjectionTransposed() const
{
    return &_o_viewProjTransp;
}

bln CCamera::IsVisible( const f32 aa_maxmin[ 3 ][ 2 ], f32 *p_dist ) const
{
    i32 *frus = (i32 *)_a_frustumLook;
    f32 dist;

    for( i32 i = 0; i < 6; ++i )
    {
        vec3 o_vec;
        o_vec.x = aa_maxmin[ 0 ][ *frus++ ];
        o_vec.y = aa_maxmin[ 1 ][ *frus++ ];
        o_vec.z = aa_maxmin[ 2 ][ *frus++ ];

        dist = LiceMath::PlaneDotCoord( &_ao_frustum[ i ], &o_vec );
        if( dist < 0.f )
        {
            return false;
        }
    }

    *p_dist = dist;
    return true;
}

f32 CCamera::SpeedGet() const
{
    return _camSpeed;
}

void CCamera::SpeedSet( f32 speedMeters )
{
    _camSpeed = speedMeters;
}

f32 CCamera::NearPlaneGet() const
{
    return _nearPlane;
}

void CCamera::NearPlaneSet( f32 val )
{
    _nearPlane = val;
    FOVSet( FOVGet(), false );
}

f32 CCamera::FarPlaneGet() const
{
    return _farPlane;
}

void CCamera::FarPlaneSet( f32 val )
{
    _farPlane = val;
    FOVSet( FOVGet(), false );
}

void CCamera::Update()
{
    LiceMath::Vec3NormalizeInplace( &_o_zvec );

    LiceMath::Vec3Cross( &_o_yvec, &_o_zvec, &_o_xvec );
    LiceMath::Vec3NormalizeInplace( &_o_yvec );

    LiceMath::Vec3Cross( &_o_xvec, &_o_yvec, &_o_zvec );
    LiceMath::Vec3NormalizeInplace( &_o_xvec );

    _o_view.e00 = _o_xvec.x;
    _o_view.e10 = _o_xvec.y;
    _o_view.e20 = _o_xvec.z;

    _o_view.e01 = _o_yvec.x;
    _o_view.e11 = _o_yvec.y;
    _o_view.e21 = _o_yvec.z;

    _o_view.e02 = _o_zvec.x;
    _o_view.e12 = _o_zvec.y;
    _o_view.e22 = _o_zvec.z;

    _o_view.e30 = -LiceMath::Vec3Dot( &_o_xvec, &_o_position );
    _o_view.e31 = -LiceMath::Vec3Dot( &_o_yvec, &_o_position );
    _o_view.e32 = -LiceMath::Vec3Dot( &_o_zvec, &_o_position );

    LiceMath::M4x3AsM4x4LastIdenMultM4x4( &_o_viewProj, &_o_view, &_o_proj );
    LiceMath::M4x4Transpose( &_o_viewProjTransp, &_o_viewProj );
    LiceMath::M4x3Transpose( &_o_viewTransp, &_o_view );

    vec4 o_col0( _o_viewProj.e00, _o_viewProj.e10, _o_viewProj.e20, _o_viewProj.e30 );
    vec4 o_col1( _o_viewProj.e01, _o_viewProj.e11, _o_viewProj.e21, _o_viewProj.e31 );
    vec4 o_col2( _o_viewProj.e02, _o_viewProj.e12, _o_viewProj.e22, _o_viewProj.e32 );
    vec4 o_col3( _o_viewProj.e03, _o_viewProj.e13, _o_viewProj.e23, _o_viewProj.e33 );

    LiceMath::Vec4Addition( (vec4 *)&_ao_frustum[ 0 ], &o_col3, &o_col0 );  //  left
    LiceMath::Vec4Subtract( (vec4 *)&_ao_frustum[ 1 ], &o_col3, &o_col0 );  //  right
    LiceMath::Vec4Subtract( (vec4 *)&_ao_frustum[ 2 ], &o_col3, &o_col1 );  //  top
    LiceMath::Vec4Addition( (vec4 *)&_ao_frustum[ 3 ], &o_col3, &o_col1 );  //  bottom
    LiceMath::Vec4Subtract( (vec4 *)&_ao_frustum[ 5 ], &o_col3, &o_col2 );  //  far
    _MemCpy( &_ao_frustum[ 4 ], &o_col2, sizeof(vec4) );                    //  near

    for( i32 i = 0; i < 6; ++i )
    {
        LiceMath::PlaneNormalizeInplace( &_ao_frustum[ i ] );

        _a_frustumLook[ i ][ 0 ] = _ao_frustum[ i ].a < 0.f;
        _a_frustumLook[ i ][ 1 ] = _ao_frustum[ i ].b < 0.f;
        _a_frustumLook[ i ][ 2 ] = _ao_frustum[ i ].c < 0.f;
    }
}