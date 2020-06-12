// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Engine/EngineTypes.h>
#include "Misc/SASOwnedStruct.h"
#include "AbilitiesCooldownCounter.generated.h"


USTRUCT()
struct ABILITIES_API FAbilitiesCooldownCounter : public FSASOwnedStruct
{
	GENERATED_BODY()

protected:

	UPROPERTY()
	TMap<UClass*, FTimerHandle> Handles;


public:

	// Starts the cooldown of an ability locally. See UAbility::StartCooldown
	// @param Ability to start
	// @param Duration in seconds of the cooldown
	void Start(UClass* Ability, float Duration);

	// Resets the cooldown of an ability locally. See UAbility::ResetCooldown
	// @param Ability to reset
	// @return true if the cooldown was active
	bool Reset(UClass* Ability);
	void ResetAll();

	bool IsCoolingDown(UClass* Ability) const { return Handles.Contains(Ability); }
	float GetRemaining(UClass* Ability) const;

private:

	FTimerManager* GetTimerManager() const;
};
