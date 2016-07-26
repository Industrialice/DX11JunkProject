#ifndef __C_POINT_LIGHT_HPP__
#define __C_POINT_LIGHT_HPP__

class CPointLight
{
    vec3 _o_pos;
    f32 _power;
    f96color _o_color;
    bln _is_on;
    bln _is_active;
    bln _is_visualize;
    bln _is_occlusion;

public:
    CPointLight();
    CPointLight( const vec3 &o_pos, const f96color o_color, f32 power, bln is_on, bln is_visualize, bln is_occlusion );
    void TurnOn();
    void TurnOff();
    void Toggle();
    bln IsOnGet() const;
    void IsOnSet( bln is_on );
    const vec3 &PositionGet() const;
    void PositionSet( const vec3 &o_pos );
    const f96color &ColorGet() const;
    void ColorSet( const f96color &o_color );
    f32 PowerGet() const;
    void PowerSet( f32 power );
    bln IsLightActive() const;
    bln IsVisualizeGet() const;
    void IsVisualizeSet( bln is_visualize );
    bln IsOcclusionGet() const;
    void IsOcclusionSet( bln is_occlusion );

private:
    void ComputeActivity();
};

#endif __C_POINT_LIGHT_HPP__