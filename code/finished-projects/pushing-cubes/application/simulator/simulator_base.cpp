#include "simulator_base.hpp"

PxDefaultAllocator 		SimulatorBase::gAllocator;
PxDefaultErrorCallback  SimulatorBase::gErrorCallback;
PxFoundation* 			SimulatorBase::gFoundation = nullptr;
PxPhysics* 				SimulatorBase::gPhysics = nullptr;
PxDefaultCpuDispatcher* SimulatorBase::gDispatcher = nullptr;
PxScene* 				SimulatorBase::gScene = nullptr;
PxMaterial* 			SimulatorBase::gMaterial = nullptr;
PxPvd* 					SimulatorBase::gPvd = nullptr;

SimulatorBase::SimulatorBase()
{
	InitPhysX();
}

SimulatorBase::~SimulatorBase()
{
	CleanPhysX();
}

void SimulatorBase::Update(float fps)
{
	gScene->simulate(fps);
	gScene->fetchResults(true);
}

PxRigidDynamic* SimulatorBase::CreateDynamic(const PxTransform& transform, const PxGeometry& geometry)
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, transform, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.1f);
	dynamic->setLinearDamping(0.1f);
	gScene->addActor(*dynamic);
	return dynamic;
}

void SimulatorBase::InitPhysX()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher	= gDispatcher;
	sceneDesc.filterShader	= PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if(pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	_groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, -1), *gMaterial);
	gScene->addActor(*_groundPlane);
}

void SimulatorBase::CleanPhysX()
{
    _groundPlane->release();
	gScene->release();
	gDispatcher->release();
	gPhysics->release();	
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	gFoundation->release();
	printf("Simulator exits.\n");
}