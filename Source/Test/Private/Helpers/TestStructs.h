// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include "TestStructs.generated.h"


USTRUCT()
struct FStructTest
{
    GENERATED_BODY()

    UPROPERTY()
	FVector Location {};
};

USTRUCT()
struct FOtherStructTest
{
    GENERATED_BODY()

    UPROPERTY()
	FVector OtherLocation {};
};
