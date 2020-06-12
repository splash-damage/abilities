// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "AbilityTypes.h"


bool FAbilityStateTransition::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Ar << Origin;
	Ar << Destination;
	Ar << Flags;
	bOutSuccess = true;
	return true;
}
