// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "Buff.h"
#include "BuffTypeContainer.generated.h"


USTRUCT()
struct FBuffTypeContainer
{
	GENERATED_BODY()

	UPROPERTY()
	UClass* Class;

	UPROPERTY()
	TSet<UBuff*> Buffs;


	FBuffTypeContainer() : Class(nullptr) {}
	FBuffTypeContainer(UClass* Class) : Class(Class) {}

	friend uint32 GetTypeHash(const FBuffTypeContainer& Container)
	{
		return GetTypeHash(Container.Class);
	}

	bool operator==(const FBuffTypeContainer& Other) const
	{
		return Class == Other.Class;
	}
};
