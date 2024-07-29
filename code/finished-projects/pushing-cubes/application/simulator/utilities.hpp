#pragma once

#include <iostream>
#include <random>
#include <vector>

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace physx;

namespace util
{

glm::vec3 PxVec3ToGlmVec3(const PxVec3& vector);

glm::mat4 PxTransformToGlmMat4(const PxTransform& transform);

PxConvexMesh* CreateRandomConvexMesh(PxPhysics* physics, const PxVec3& size, const PxU32& vertexCount);

PxConvexMesh* CreateConvexMesh(PxPhysics* physics, PxU32 numVerts, const PxVec3* verts);

std::vector<glm::vec3> GetConvexMeshTriangles(const PxConvexMesh* mesh);

float RandomFloat(float min, float max);

} // namespace util
