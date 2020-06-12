// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "TestHelpers.h"
#include <GameFramework/Actor.h>
#include <EngineUtils.h>
#include <Tests/AutomationCommon.h>
#include <GameFramework/PlayerController.h>


#if WITH_DEV_AUTOMATION_TESTS

FAbilityTestSpec::~FAbilityTestSpec()
{
	ShutdownWorld();
}

UWorld* FAbilityTestSpec::FindSimpleEngineAutomationTestWorld()
{
	const int32 TestFlags = GetTestFlags();

	// Accessing the game world is only valid for game-only
	if (((TestFlags & EAutomationTestFlags::EditorContext) || (TestFlags & EAutomationTestFlags::ClientContext)) == false)
	{
		return nullptr;
	}

	if (FWorldContext* PIEContext = GEngine->GetWorldContextFromPIEInstance(0))
	{
		return PIEContext->World();
	}

	if (GEngine->GetWorldContexts().Num() == 0)
	{
		return nullptr;
	}
	if (GEngine->GetWorldContexts()[0].WorldType != EWorldType::Game && GEngine->GetWorldContexts()[0].WorldType != EWorldType::Editor)
	{
		return nullptr;
	}
	return GEngine->GetWorldContexts()[0].World();
}

AAbilityTestActor* FAbilityTestSpec::AddTestActor()
{
	UWorld* OwnerWorld;
	if (World.IsValid())
	{
		OwnerWorld = World.Get();
	}
	else
	{
		OwnerWorld = FindSimpleEngineAutomationTestWorld();
	}

	if (!OwnerWorld)
	{
		AddError(TEXT("Could not find a test world"));
		return nullptr;
	}

	FActorSpawnParameters Params{};
	Params.ObjectFlags |= RF_Transient;
	return OwnerWorld->SpawnActor<AAbilityTestActor>(Params);
}

UWorld* FAbilityTestSpec::CreateWorld()
{
	if (World.IsValid())
	{
		return World.Get();
	}

	const bool bInformEngineOfWorld = true;
	UWorld* NewWorld = UWorld::CreateWorld(EWorldType::Game, bInformEngineOfWorld, TEXT("FAbilitySpec::World"));

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(NewWorld);

	if (!NewWorld)
	{
		AddError(TEXT("CreateWorld() failed"));
		return nullptr;
	}

	FURL URL;
	NewWorld->InitializeActorsForPlay(URL);

	// Setting this bool allows all actors and components to initialize properly using
	// the expected PostInitProperties() / BeginPlay() flow, without manual intervention.
	NewWorld->bBegunPlay = true;

	World = NewWorld;
	return GetWorld();
}

UWorld* FAbilityTestSpec::CreatePIE()
{
	if (AutomationOpenMap("Template_Default"))
	{
		World = FindSimpleEngineAutomationTestWorld();
		return World.Get();
	}
	return nullptr;
}

void FAbilityTestSpec::ShutdownWorld()
{
	// Destroy world if there's any
	if (World.IsValid())
	{
		for (TActorIterator<AActor> Iter(World.Get()); Iter; ++Iter)
		{
			Iter->Destroy();
		}

		const bool InformEngineOfWorld = true;
		GEngine->DestroyWorldContext(World.Get());
		World->DestroyWorld(InformEngineOfWorld);
		World = nullptr;

		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	}
}

void FAbilityTestSpec::ExitPIE()
{
	if (World.IsValid() && World->IsPlayInEditor())
	{
		if (APlayerController* TargetPC = World->GetFirstPlayerController())
		{
			TargetPC->ConsoleCommand(TEXT("Exit"), true);
		}
	}
}

#endif //WITH_DEV_AUTOMATION_TESTS
