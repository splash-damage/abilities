// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "Ability.h"
#include "TestAbility.generated.h"


UCLASS(NotBlueprintable, NotBlueprintType)
class ABILITIESTEST_API UTestAbility : public UAbility
{
	GENERATED_BODY()

public:

	bool bCalledBeginPlay = false;

	UPROPERTY()
	bool bEnableActivation = true;

	DECLARE_EVENT(UTestAbility, FOnTick);
	FOnTick OnTick;


	UTestAbility() : Super()
	{
		bHasCast = false;
		TickMode = EAbilityTickMode::DuringActivationOnly;
	}

protected:

	virtual bool CanActivate(const FStructContainer& Container) const override
	{
		return bEnableActivation;
	}

	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		bCalledBeginPlay = true;
	}

	virtual void Tick(float DeltaTime) override
	{
		OnTick.Broadcast();
	}
};


UCLASS(NotBlueprintable, NotBlueprintType)
class ABILITIESTEST_API UTestAbility2 : public UAbility
{
	GENERATED_BODY()

public:

	bool bCalledBeginPlay = false;


	UTestAbility2() : Super() { bHasCast = true; }

	virtual void BeginPlay() override {
		Super::BeginPlay();
		bCalledBeginPlay = true;
	}
};

UCLASS(NotBlueprintable, NotBlueprintType)
class ABILITIESTEST_API UTestAbility3 : public UAbility
{
	GENERATED_BODY()

public:

	UTestAbility3() : Super() { bHasCast = false; }

	virtual void BeginPlay() override {
		Super::BeginPlay();
	}

	virtual void OnActivation(const FStructContainer& Container) override
	{
		Super::OnActivation(Container);
		Deactivate();
	}
};