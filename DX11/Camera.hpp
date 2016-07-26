#ifndef __CAMERA__
#define __CAMERA__

class CCamera
{
    f32 _fov;
    f32 _upRestrict;
    f32 _downRestrict;
    bln _is_upRestrict;
    bln _is_downRestrict;
    f32 _camSpeed;
    vec3 _o_position;
    vec3 _o_xvec;
    vec3 _o_yvec;
    vec3 _o_zvec;
    i32 _a_frustumLook[ 6 ][ 3 ];
    plane _ao_frustum[ 6 ];
    m4x3 _o_view;
    m3x4 _o_viewTransp;
    m4x4 _o_proj, _o_projTransp;
    m4x4 _o_viewProj, _o_viewProjTransp;
    f32 _nearPlane;
    f32 _farPlane;

public:
    CCamera() {  /*  void  */  }
    CCamera( f32 nearPlane, f32 farPlane, f32 fov );
    f32 FOVGet() const;
    void FOVSet( f32 fov, bln is_remember );
    const vec3 *XVec() const;
    const vec3 *YVec() const;
    const vec3 *ZVec() const;
    f32 UpRestrictionGet() const;
    void UpRestrictionSet( f32 val );
    f32 DownRestrictionGet() const;
    void DownRestrictionSet( f32 val );
    vec3 PositionGet() const;
    void PositionSet( const vec3 &o_posMeters );
    void LookAtSet( const vec3 &o_look );
    const vec3 *LookAtGet() const;
    void MoveX( f32 move );
    void MoveY( f32 move );
    void MoveZ( f32 move );
    void RotationAroundX( f32 angleDX );
    void RotationAroundY( f32 angleDX );
    vec3 RotationDXGet() const;
    void RotationDXSet( const vec3 &o_rot );
    vec3 RotationDegreeGet() const;
    void RotationDegreeSet( const vec3 &o_rot );
    const m4x3 *View() const;
    const m3x4 *ViewTransposed() const;
    const m4x4 *Projection() const;
    const m4x4 *ProjectionTransposed() const;
    const m4x4 *ViewProjection() const;
    const m4x4 *ViewProjectionTransposed() const;
    bln IsVisible( const f32 aa_maxmin[ 3 ][ 2 ], f32 *p_dist ) const;
    f32 SpeedGet() const;
    void SpeedSet( f32 speedMeters );
    f32 NearPlaneGet() const;
    void NearPlaneSet( f32 val );
    f32 FarPlaneGet() const;
    void FarPlaneSet( f32 val );
    void Update();
};

#endif __CAMERA__  //  34