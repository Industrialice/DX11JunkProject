#include "PreHeader.hpp"
#include "CObjectBase.hpp"

CObjectBase::~CObjectBase()
{
}

CObjectBase::CObjectBase( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats )
{
    _o_pos = o_pos;
    _o_rot = o_rot;
    _o_size = o_size;
    _o_mats = std::move( o_mats );
    _is_lightable = true;
    _is_visible = true;
    _o_additionalShaderData = SShaderData();
    //LiceMath::M4x3Scale3DRotateXYZTranslate3DVec( &_o_w, &o_size, &o_rot, &o_pos );
    LiceMath::M4x3Identity( &_o_wrot );
    _is_transparenting = false;
    _is_glowing = false;
    for( ui32 mat = 0; mat < _o_mats.Size(); ++mat )
    {
        if( _o_mats[ mat ].is_enabled )
        {
            _is_glowing |= (_o_mats[ mat ].rstates & RStates::glowmap) != 0;
        }
    }
}

PROPERTYG( CObjectBase::Pos, _o_pos, const vec3 & );

void CObjectBase::PosSet( const vec3 &source )
{
    _o_pos = source;
    //LiceMath::M4x3Scale3DRotateXYZTranslate3DVec( &_o_w, &_o_size, &_o_rot, &_o_pos );
}

PROPERTYG( CObjectBase::Size, _o_size, const vec3 & );

void CObjectBase::SizeSet( const vec3 &source )
{
    _o_size = source;
    //LiceMath::M4x3Scale3DRotateXYZTranslate3DVec( &_o_w, &_o_size, &_o_rot, &_o_pos );
}

PROPERTYG( CObjectBase::RotRad, _o_rot, const vec3 & );

void CObjectBase::RotRadSet( const vec3 &source )
{
    _o_rot = source;
    //LiceMath::M4x3Scale3DRotateXYZTranslate3DVec( &_o_w, &_o_size, &_o_rot, &_o_pos );
}

void CObjectBase::RotSetDeg( const vec3 &o_rot )
{
    _o_rot = vec3( DEGREETORADIAN( o_rot.x ), DEGREETORADIAN( o_rot.y ), DEGREETORADIAN( o_rot.z ) );
    //LiceMath::M4x3Scale3DRotateXYZTranslate3DVec( &_o_w, &_o_size, &_o_rot, &_o_pos );
}

vec3 CObjectBase::RotGetDeg() const
{
    return vec3( RADIANTODEGREE( _o_rot.x ), RADIANTODEGREE( _o_rot.y ), RADIANTODEGREE( _o_rot.z ) );
}

void CObjectBase::MaterialSet( ui8 index, SMaterial &&o_matIn )
{
    if( index >= _o_mats.Size() )
    {
        HARDBREAK;
        return;
    }
    _o_mats[ index ] = std::move( o_matIn );
}

SMaterial &CObjectBase::MaterialGet( ui8 index )
{
    if( index >= _o_mats.Size() )
    {
        HARDBREAK;
        index = 0;
    }
    return _o_mats[ index ];
}

void CObjectBase::RemoveLights()
{
    _o_lights.Clear();
}

void CObjectBase::AttachLight( const CPointLight *cpo_light )
{
    _o_lights.Append( cpo_light );
}

PROPERTYG( CObjectBase::IsLightable, _is_lightable, bln );

void CObjectBase::IsLightableSet( bln is_lightable )
{
    _is_lightable = is_lightable;
    if( !_is_lightable )
    {
        RemoveLights();
    }
}

PROPERTY( CObjectBase::IsVisible, _is_visible, bln );

PROPERTYG( CObjectBase::IsInFrustum, _is_inFrustum, bln );

PROPERTYG( CObjectBase::IsGlowing, _is_glowing, bln );

PROPERTYG( CObjectBase::IsTransparenting, _is_transparenting, bln );
    
void CObjectBase::SetAdditionalShaderData( void *data, uiw size, SShaderData::targetShader_t targetShaders )
{
    if( _o_additionalShaderData.data )
    {
        Heap::Free( _o_additionalShaderData.data );
    }
    if( data )
    {
        _o_additionalShaderData.data = Heap::Alloc( size );
        _MemCpy( _o_additionalShaderData.data, data, size );
        _o_additionalShaderData.dataSize = size;
        _o_additionalShaderData.targetShaders = targetShaders;
    }
    else
    {
        _o_additionalShaderData.data = 0;
        _o_additionalShaderData.targetShaders = SShaderData::none;
        _o_additionalShaderData.dataSize = 0;
    }
}

void CObjectBase::FlushBuffers()  //  virtual
{
}