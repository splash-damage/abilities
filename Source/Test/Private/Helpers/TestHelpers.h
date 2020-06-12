// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Automatron.h>

#include "TestSpec.h"
#include "Helpers/AbilityTestActor.h"


class FAbilityTestSpec : public FTestSpec
{
private:
	TWeakObjectPtr<UWorld> World;


public:

	FAbilityTestSpec() = default;
	virtual ~FAbilityTestSpec();

protected:


	UWorld* FindSimpleEngineAutomationTestWorld();

	AAbilityTestActor* AddTestActor();

	UAbilitiesComponent* AddTestComponent()
	{
		auto* TestActor = AddTestActor();
		return TestActor ? TestActor->Abilities : nullptr;
	}

	void RemoveTestComponent(UAbilitiesComponent* Component)
	{
		if (IsValid(Component))
		{
			Component->GetOwner()->Destroy();
		}
	}

	UWorld* CreateWorld();
	UWorld* CreatePIE();
	void ShutdownWorld();
	void ExitPIE();

	void TickWorld() const
	{
		if (World.IsValid())
		{
			World->SetShouldTick(true);
		}
	}

	void TestNotImplemented()
	{
		AddWarning(TEXT("Test not implemented"), 1);
	}

	UWorld* GetWorld() const { return World.Get(); }
};
