#ifndef __C_POINTS_LIGHTS_MANAGER_HPP__
#define __C_POINTS_LIGHTS_MANAGER_HPP__

#include "CPointLight.hpp"
#include "CObjectBase.hpp"
#include "ShadersManager.hpp"
#include "Geometry.hpp"
#include "CHalos.hpp"

class CPointsLightsManager
{
    CVec < CPointLight * > _o_lights;
    CVec < ID3D11Query * > _o_occlusionQuerys;
    CHalos *_po_halos;
    f32 _brightScale;
    bln _is_configChanged;

public:
    CPointsLightsManager();
    void AddLight( CPointLight *po_light );
    void RemoveLight( CPointLight *po_light );
    void RemoveByIndex( uiw index );
    uiw Size() const;
    CPointLight *LightGetByIndex( uiw index );
    void Attach( CObjectBase *po_object ) const;
    void Visualize();

private:
    void PushHalos();
};

#endif __C_POINTS_LIGHTS_MANAGER_HPP__