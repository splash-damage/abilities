// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "AbilityTypes.generated.h"


UENUM(Blueprintable)
enum class EAbilityState : uint8
{
	None UMETA(Hidden),
	BeforeBeginPlay UMETA(Hidden),
	JustEquipped, // After begin play the ability will be "JustEquipped"
	Cancelled, // If the ability canceled last time it executed
	Succeeded,  // If the ability finished successfully last time it executed
	Cast, // State in which the player will drag a curve or wait for some time
	Activation,
	AfterEndPlay UMETA(Hidden)
};


UENUM(Blueprintable, meta = (BitFlags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAbilityTransitionFlag : uint8
{
	None             = 0,
	StartCooldown    = 1 << 0,
	PredictionFailed = 1 << 1 UMETA(Hidden)
};
ENUM_CLASS_FLAGS(EAbilityTransitionFlag);


USTRUCT(BlueprintType)
struct FAbilityStateTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Transition)
	EAbilityState Origin = EAbilityState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Transition)
	EAbilityState Destination = EAbilityState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Transition, meta = (Bitmask, BitmaskEnum = EAbilityTransitionFlag))
	EAbilityTransitionFlag Flags;


	void Swap();

	bool operator==(const FAbilityStateTransition& Other) const
	{
		return Other.Origin == Origin && Other.Destination == Destination;
	}

	bool operator!=(const FAbilityStateTransition& Other) const
	{
		return !(*this == Other);
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	friend uint32 GetTypeHash(const FAbilityStateTransition& Item)
	{
		// Combine both enums into 16 bits
		return (uint32(Item.Destination) << 8) | uint32(Item.Origin);
	}
};

template<>
struct TStructOpsTypeTraits<FAbilityStateTransition> : TStructOpsTypeTraitsBase2<FAbilityStateTransition>
{
	enum { WithNetSerializer = true };
};

inline void FAbilityStateTransition::Swap()
{
	::Swap(Origin, Destination);
}