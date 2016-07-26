#ifndef __PARTICLE_SYSTEM_GPU_HPP__
#define __PARTICLE_SYSTEM_GPU_HPP__

#include "CObject.hpp"

//  must have 2 materials: first for drawing, second for stream output
class CParticleSystemGPU : public CObject
{
public:
    virtual ~CParticleSystemGPU() override;
    CParticleSystemGPU( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats );
    virtual void Update() override;
    virtual void Draw( bln is_stepTwo ) override;
};

#endif __PARTICLE_SYSTEM_HPP__