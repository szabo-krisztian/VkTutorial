#pragma once

#include <vector>
#include <iostream>

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "simulator_base.hpp"

using namespace physx;

class Simulator : public SimulatorBase
{
public:
    Simulator();
    ~Simulator();

    void                   CreateMainMesh(const PxVec3& worldPosition, const PxVec3& dimensions, const PxU32& vertexCount);
    std::vector<glm::vec3> GetMainMeshTriangles();
    glm::mat4              GetMainMeshTransform();

    void                   CreateBulletMesh(const PxVec3& dimensions, const PxU32& vertexCount);
    std::vector<glm::vec3> GetBulletMeshTriangles();

    size_t                 GetBulletCount();
    void                   ShootBullet(const PxVec3& direction, const PxVec3& position);
    std::vector<glm::mat4> GetBulletTransforms();    

private:
    PxConvexMesh               *_mainMesh;
    PxRigidDynamic             *_mainActor;

    const int                    _MAX_BULLETS = 10;
    int                          _currentBullet = 0;
    PxConvexMesh                 *_bulletMesh;
    std::vector<PxRigidDynamic*> _bulletActors;

    void InitPhysX();
    void CleanPhysX();
};
