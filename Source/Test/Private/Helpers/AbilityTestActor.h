// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "Ability.h"
#include "AbilitiesComponent.h"
#include "AbilityTestActor.generated.h"


UCLASS()
class ABILITIESTEST_API AAbilityTestActor : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY()
	UAbilitiesComponent* Abilities;

	AAbilityTestActor() : Super()
	{
		Abilities = CreateDefaultSubobject<UAbilitiesComponent>(TEXT("Abilities"));
		PrimaryActorTick.bCanEverTick = true;
	}
};