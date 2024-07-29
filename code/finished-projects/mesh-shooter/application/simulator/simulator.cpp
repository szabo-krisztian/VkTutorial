#include "simulator.hpp"

#include "utilities.hpp"

Simulator::Simulator() : _mainMesh{nullptr}, _mainActor{nullptr}, _bulletMesh{nullptr}
{
	InitPhysX();
}

Simulator::~Simulator()
{
	CleanPhysX();
}



void Simulator::InitPhysX()
{
	
}

void Simulator::CleanPhysX()
{
	if (_bulletMesh)
	{
		_bulletMesh->release();
	}
	if (_mainMesh)
	{
		_mainMesh->release();	
	}
	if (_mainActor)
	{
		_mainActor->release();
	}
	for (auto& bulletActor : _bulletActors)
	{
		if (bulletActor)
		{
			bulletActor->release();
		}
	}
}

void Simulator::CreateMainMesh(const PxVec3& worldPosition, const PxVec3& dimensions, const PxU32& vertexCount)
{
	_mainMesh = util::CreateRandomConvexMesh(gPhysics, dimensions, vertexCount);
	_mainActor = CreateDynamic(PxTransform(worldPosition), PxConvexMeshGeometry(_mainMesh), 10.0f);
}

std::vector<glm::vec3> Simulator::GetMainMeshTriangles()
{
	return util::GetConvexMeshTriangles(_mainMesh);
}

glm::mat4 Simulator::GetMainMeshTransform()
{	
	PxTransform transform = _mainActor->getGlobalPose();
	return util::PxTransformToGlmMat4(transform);	
}

void Simulator::CreateBulletMesh(const PxVec3& dimensions, const PxU32& vertexCount)
{
	_bulletMesh = util::CreateRandomConvexMesh(gPhysics, dimensions, vertexCount);	
}

std::vector<glm::vec3> Simulator::GetBulletMeshTriangles()
{
	return util::GetConvexMeshTriangles(_bulletMesh);
}

size_t Simulator::GetBulletCount()
{
	return _bulletActors.size();
}

void Simulator::ShootBullet(const PxVec3& velocity, const PxVec3& position)
{
	if (_bulletActors.size() < _MAX_BULLETS)
	{
		_bulletActors.push_back(CreateDynamic(PxTransform(position), PxConvexMeshGeometry(_bulletMesh), 500.0f));
	}

	auto& currentBullet = _bulletActors[_currentBullet];
	currentBullet->setGlobalPose(PxTransform(position));
	currentBullet->setLinearVelocity(velocity);
	_currentBullet = (_currentBullet + 1) % _MAX_BULLETS;
}

std::vector<glm::mat4> Simulator::GetBulletTransforms()
{
	std::vector<glm::mat4> transforms;
	for (const auto& actor : _bulletActors)
	{
		if (!actor) { break; }
		transforms.push_back(util::PxTransformToGlmMat4(actor->getGlobalPose()));
	}
	return transforms;
}