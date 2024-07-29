#include "utilities.hpp"

using namespace physx;

namespace util
{

glm::vec3 PxVec3ToGlmVec3(const physx::PxVec3& vector)
{
    return glm::vec3(vector.x, vector.y, vector.z);
}

PxVec3 GlmVec3ToPxVec3(const glm::vec3& vector)
{
    return PxVec3(vector.x, vector.y, vector.z);
}

glm::mat4 PxTransformToGlmMat4(const physx::PxTransform& transform)
{
    physx::PxVec3 p = transform.p;
    physx::PxQuat q = transform.q;

    glm::vec3 position(p.x, p.y, p.z);
    glm::quat rotation(q.w, q.x, q.y, q.z);
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::toMat4(rotation);
    glm::mat4 modelMatrix = translationMatrix * rotationMatrix;

    return modelMatrix;
}

PxConvexMesh* CreateConvexMesh(PxPhysics* physics, PxU32 numVerts, const PxVec3* verts)
{
    PxTolerancesScale tolerances;
    PxCookingParams params(tolerances);
    params.gaussMapLimit = 256;
    params.convexMeshCookingType = PxConvexMeshCookingType::eQUICKHULL;

    PxConvexMeshDesc desc;
    desc.points.data = verts;
    desc.points.count = numVerts;
    desc.points.stride = sizeof(PxVec3);
    desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    auto mesh = PxCreateConvexMesh(params, desc, physics->getPhysicsInsertionCallback());
    PX_ASSERT(mesh);
    return mesh;
}

std::vector<glm::vec3> GetConvexMeshTriangles(const PxConvexMesh* mesh)
{
    PxU32 numVertices = mesh->getNbVertices();
    const PxVec3* vertices = mesh->getVertices();
    PxU32 numPolygons = mesh->getNbPolygons();
    std::vector<glm::vec3> triangleVertices;

    for (PxU32 i = 0; i < numPolygons; ++i)
    {
        PxHullPolygon polygon;
        mesh->getPolygonData(i, polygon);

        const PxU8* indexBuffer = mesh->getIndexBuffer() + polygon.mIndexBase;
        for (PxU16 j = 1; j < polygon.mNbVerts - 1; ++j)
        {
            triangleVertices.push_back(PxVec3ToGlmVec3(vertices[indexBuffer[0]]));
            triangleVertices.push_back(PxVec3ToGlmVec3(vertices[indexBuffer[j]]));
            triangleVertices.push_back(PxVec3ToGlmVec3(vertices[indexBuffer[j + 1]]));
        }
    }

    return triangleVertices;
}

float RandomFloat(float min, float max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

PxConvexMesh* CreateRandomConvexMesh(PxPhysics* physics, const PxVec3& size, const PxU32& vertexCount)
{
    std::vector<PxVec3> vertices(vertexCount);

    auto xmin = -size.x / 2;
    auto xmax = size.x / 2;
    auto ymin = -size.y / 2;
    auto ymax = size.y / 2;
    auto zmin = -size.z / 2;
    auto zmax = size.z / 2;

    for (auto& vertex : vertices)
    {
        vertex = PxVec3(RandomFloat(xmin, xmax), RandomFloat(ymin, ymax), RandomFloat(zmin, zmax));
    }
    auto convexMesh = CreateConvexMesh(physics, static_cast<PxU32>(vertices.size()), vertices.data());
    return convexMesh;
}

} // namespace util
