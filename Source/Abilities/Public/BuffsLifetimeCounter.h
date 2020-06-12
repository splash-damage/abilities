// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include "Misc/SASOwnedStruct.h"
#include "BuffsLifetimeCounter.generated.h"


class UBuff;
struct FBuffCount;

USTRUCT()
struct FBuffsLifetimeCounter : public FSASOwnedStruct
{
	GENERATED_BODY()

protected:

	// Stores the GameTime at which each buff will finish its effect
	UPROPERTY()
	TMap<UBuff*, float> LifetimePerBuff;


public:

	void Start(const TSet<FBuffCount>& Buffs);
	void Reset(const TSet<FBuffCount>& Buffs);
	void ResetAll();

	float GetRemaining(const UBuff* Buff) const;

	void Tick();
};
