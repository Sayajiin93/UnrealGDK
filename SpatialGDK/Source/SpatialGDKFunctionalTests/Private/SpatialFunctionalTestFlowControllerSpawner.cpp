#include "SpatialFunctionalTestFlowControllerSpawner.h"


#include "Engine/World.h"
#include "Engine/NetDriver.h"
#include "GameFramework/PlayerController.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialFunctionalTest.h"

SpatialFunctionalTestFlowControllerSpawner::SpatialFunctionalTestFlowControllerSpawner()
	: SpatialFunctionalTestFlowControllerSpawner(nullptr, TSubclassOf<ASpatialFunctionalTestFlowController>(ASpatialFunctionalTestFlowController::StaticClass()))
{
}

SpatialFunctionalTestFlowControllerSpawner::SpatialFunctionalTestFlowControllerSpawner(ASpatialFunctionalTest* ControllerOwningTest, TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerClassToSpawn)
	: OwningTest(ControllerOwningTest),
	FlowControllerClass(FlowControllerClassToSpawn),
	NextClientControllerId(1)
{
}

void SpatialFunctionalTestFlowControllerSpawner::ModifyFlowControllerClassToSpawn(TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerClassToSpawn)
{
	FlowControllerClass = FlowControllerClassToSpawn;
}

ASpatialFunctionalTestFlowController* SpatialFunctionalTestFlowControllerSpawner::SpawnServerFlowController()
{
	UWorld* World = OwningTest->GetWorld();
	UNetDriver* NetDriver = World->GetNetDriver();
	if (NetDriver != nullptr && !NetDriver->IsServer())
	{
		//Not a server, quit
		return nullptr;
	}

	ASpatialFunctionalTestFlowController* ServerFlowController = World->SpawnActorDeferred<ASpatialFunctionalTestFlowController>(FlowControllerClass, FTransform());
	ServerFlowController->OwningTest = OwningTest;
	ServerFlowController->ControllerType = ESpatialFunctionalTestFlowControllerType::Server;
	ServerFlowController->ControllerInstanceId = OwningServerIntanceId(World);

	ServerFlowController->FinishSpawning(FTransform(), true);
	// TODO: Replace locking with custom LB strategy - UNR-3393
	LockFlowControllerDelegations(ServerFlowController);

	return ServerFlowController;
}

ASpatialFunctionalTestFlowController* SpatialFunctionalTestFlowControllerSpawner::SpawnClientFlowController(APlayerController* OwningClient)
{
	UWorld* World = OwningTest->GetWorld();

	ASpatialFunctionalTestFlowController* FlowController = World->SpawnActorDeferred<ASpatialFunctionalTestFlowController>(FlowControllerClass, OwningTest->GetActorTransform(), OwningClient);
	FlowController->OwningTest = OwningTest;
	FlowController->ControllerType = ESpatialFunctionalTestFlowControllerType::Client;
	FlowController->ControllerInstanceId = NextClientControllerId++;
	
	FlowController->FinishSpawning(OwningTest->GetActorTransform(), true);
	// TODO: Replace locking with custom LB strategy - UNR-3393
	LockFlowControllerDelegations(FlowController);

	return FlowController;
}

uint8 SpatialFunctionalTestFlowControllerSpawner::OwningServerIntanceId(UWorld* World) const
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	if (SpatialNetDriver == nullptr || SpatialNetDriver->LoadBalanceStrategy == nullptr)
	{
		//not loadbalanced test, default instance 1
		return 1;
	}
	else
	{
		return static_cast<uint8>(SpatialNetDriver->LoadBalanceStrategy->GetLocalVirtualWorkerId());
	}
}

void SpatialFunctionalTestFlowControllerSpawner::LockFlowControllerDelegations(ASpatialFunctionalTestFlowController* FlowController) const
{
	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(FlowController->GetNetDriver());
	if(SpatialNetDriver == nullptr || SpatialNetDriver->LoadBalanceStrategy == nullptr)
	{
		return;
	}
	else
	{
		SpatialNetDriver->LockingPolicy->AcquireLock(FlowController);
	}
}
