// Copyright 2019 Splash Damage Ltd. All Rights Reserved

#pragma once

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>

#include "AbilitiesSettings.generated.h"


UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Abilities"))
class ABILITIES_API UAbilitiesSettings : public UDeveloperSettings
{
	GENERATED_BODY()
};
