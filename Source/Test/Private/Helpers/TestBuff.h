// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "Buff.h"
#include "TestBuff.generated.h"


UCLASS(NotBlueprintable, NotBlueprintType)
class ABILITIESTEST_API UTestBuff : public UBuff
{
	GENERATED_BODY()

public:

	mutable bool bChangesApplied = false;
	mutable int32 LastCount;

protected:

	virtual void ApplyEffects(UAbilitiesComponent* Component, int32 Count) const override
	{
		bChangesApplied = true;
		LastCount = Count;
	}

	virtual void RevertEffects(UAbilitiesComponent* Component, int32 Count) const override
	{
		bChangesApplied = false;
		LastCount = Count;
	}
};

UCLASS(NotBlueprintable, NotBlueprintType)
class ABILITIESTEST_API UTestBuff_Unique : public UBuff
{
	GENERATED_BODY()

	UTestBuff_Unique() : Super()
	{
		bUnique = true;
		bReplacePreviousUnique = false;
	}
};

UCLASS(NotBlueprintable, NotBlueprintType)
class ABILITIESTEST_API UTestBuff_UniqueReplace : public UBuff
{
	GENERATED_BODY()

	UTestBuff_UniqueReplace() : Super()
	{
		bUnique = true;
		bReplacePreviousUnique = true;
	}
};

UCLASS(NotBlueprintable, NotBlueprintType)
class ABILITIESTEST_API UTestBuff_Stackable : public UTestBuff
{
	GENERATED_BODY()

	UTestBuff_Stackable() : Super()
	{
		bStackable = true;
	}
};