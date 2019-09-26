// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Engine/Canvas.h"
#include "GameFramework/Info.h"

#include <WorkerSDK/improbable/c_worker.h>

#include "SpatialDebugger.generated.h"

class APawn;
class APlayerController;
class USpatialNetDriver;
class UFont;

DECLARE_STATS_GROUP(TEXT("SpatialDebugger"), STATGROUP_SpatialDebugger, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("DrawDebug"), STAT_DrawDebug, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawDebugEntry"), STAT_DrawDebugEntry, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("Projection"), STAT_Projection, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawIcons"), STAT_DrawIcons, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("DrawText"), STAT_DrawText, STATGROUP_SpatialDebugger);
DECLARE_CYCLE_STAT(TEXT("BuildText"), STAT_BuildText, STATGROUP_SpatialDebugger);

UCLASS(SpatialType=Singleton)
class SPATIALGDK_API ASpatialDebugger :
	public AInfo
{
	GENERATED_UCLASS_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;

private:

	void LoadIcons();

	void OnEntityAdded(const Worker_EntityId EntityId);
	void OnEntityRemoved(const Worker_EntityId EntityId);

	void DrawDebug(class UCanvas* Canvas, APlayerController* Controller);
	void DrawEntry(UCanvas* Canvas, const FVector2D& ScreenLocation, const Worker_EntityId EntityId, const FString& ActorName);
	void DrawDebugLocalPlayer(UCanvas* Canvas);

	FString GetVirtualWorkerId(const Worker_EntityId EntityId) const;

	static int32 VirtualWorkerIdToInt(const FString& VirtualWorkerId);
	static int32 HashPosition(const FVector& Position);

	static const int VIRTUAL_WORKER_MAX_COUNT = 4;
	static const int POSITION_HASH_BUCKETS = 1024;

	enum EIcon
	{
		ICON_AUTH,
		ICON_AUTH_INTENT,
		ICON_LOCK_OPEN,
		ICON_LOCK_CLOSED,
		ICON_MAX
	};

	// These mappings are maintained independently on each client
	// Mapping of the entities a client has checked out
	TMap<int64, TWeakObjectPtr<AActor>> EntityActorMapping;
	// Quantized mapping of position -> # actors at that position (1cm grid resolution) to allow us to stack info vertically for co-located actors
	TMap<float, int32> ActorLocationCountMapping;

	FDelegateHandle DrawDebugDelegateHandle;

	APawn* LocalPawn;
	APlayerController* LocalPlayerController;
	APlayerState* LocalPlayerState;
	UFont* RenderFont;

	FFontRenderInfo FontRenderInfo;
	FCanvasIcon Icons[ICON_MAX];

	const FString IconFilenames[ICON_MAX] =
	{
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/Auth.Auth")),
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/AuthIntent.AuthIntent")),
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/LockOpen.LockOpen")),
		FString(TEXT("/SpatialGDK/SpatialDebugger/Icons/LockClosed.LockClosed"))
	};

	const FColor WorkerColors[VIRTUAL_WORKER_MAX_COUNT] =
	{
		FColor::Red,
		FColor::Green,
		FColor::Blue,
		FColor::Yellow
	};

	// TODO: These should all be exposed through both the editor and a runtime UI
	int PlayerPanelStartX = 64;
	int PlayerPanelStartY = 128;

	float MaxRange;

	bool bShowAuth;
	bool bShowAuthIntent;
	bool bShowLock;
	bool bShowEntityId;
	bool bShowActorName;
};
