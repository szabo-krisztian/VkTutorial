#pragma once

#include <vector>
#include <iostream>

#include <PxPhysicsAPI.h>

using namespace physx;

class SimulatorBase
{
public:
    SimulatorBase();
    ~SimulatorBase();

    void Update(float fps);

protected:
    static PxDefaultAllocator     gAllocator;
    static PxDefaultErrorCallback gErrorCallback;
    static PxFoundation           *gFoundation;
    static PxPhysics              *gPhysics;
    static PxDefaultCpuDispatcher *gDispatcher;
    static PxScene                *gScene;
    static PxMaterial             *gMaterial;
    static PxPvd                  *gPvd;

    PxRigidDynamic* CreateDynamic(const PxTransform& transform, const PxGeometry& geometry);

private:
    PxRigidStatic *_groundPlane;

    void InitPhysX();
    void CleanPhysX();
};
