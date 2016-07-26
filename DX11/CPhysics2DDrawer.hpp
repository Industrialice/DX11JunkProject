#ifndef __CPHYSICS2D_DRAWER_HPP__
#define __CPHYSICS2D_DRAWER_HPP__

#include "CPhysics2D.hpp"

class CPhysics2DDrawer : public CPhysics2D
{
public:
    CPhysics2DDrawer();
    virtual void FlushBuffers() override;
    virtual void Draw( bln is_stepTwo ) override;
};

#endif __CPHYSICS2D_DRAWER_HPP__