#include "PreHeader.hpp"
#include "CPointLight.hpp"

CPointLight::CPointLight()
{
    _o_pos = vec3();
    _o_color = f96color();
    _power = 0.f;
    _is_on = false;
}

CPointLight::CPointLight( const vec3 &o_pos, const f96color o_color, f32 power, bln is_on, bln is_visualize, bln is_occlusion )
{
    _o_pos = o_pos;
    _o_color = o_color;
    _power = power;
    _is_on = is_on;
    _is_visualize = is_visualize;
    _is_occlusion = is_occlusion;
    ComputeActivity();
}

void CPointLight::TurnOn()
{
    _is_on = true;
    ComputeActivity();
}

void CPointLight::TurnOff()
{
    _is_on = false;
    _is_active = false;
}

void CPointLight::Toggle()
{
    _is_on = !_is_on;
    ComputeActivity();
}

bln CPointLight::IsOnGet() const
{
    return _is_on;
}

void CPointLight::IsOnSet( bln is_on )
{
    _is_on = is_on;
    ComputeActivity();
}

const vec3 &CPointLight::PositionGet() const
{
    return _o_pos;
}

void CPointLight::PositionSet( const vec3 &o_pos )
{
    _o_pos = o_pos;
}

const f96color &CPointLight::ColorGet() const
{
    return _o_color;
}

void CPointLight::ColorSet( const f96color &o_color )
{
    _o_color = o_color;
    ComputeActivity();
}

f32 CPointLight::PowerGet() const
{
    return _power;
}

void CPointLight::PowerSet( f32 power )
{
    _power = power;
    ComputeActivity();
}

bln CPointLight::IsLightActive() const
{
    return _is_active;
}

bln CPointLight::IsVisualizeGet() const
{
    return _is_visualize;
}

void CPointLight::IsVisualizeSet( bln is_visualize )
{
    _is_visualize = true;
}

bln CPointLight::IsOcclusionGet() const
{
    return _is_occlusion;
}

void CPointLight::IsOcclusionSet( bln is_occlusion )
{
    _is_occlusion = is_occlusion;
}

void CPointLight::ComputeActivity()  //  private
{
    _is_active = _is_on && _power > 0.0001f && (_o_color.r + _o_color.g + _o_color.b > 0.0001f);
}