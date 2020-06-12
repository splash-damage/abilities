// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Engine/NetDriver.h>
#include <Net/RepLayout.h>

#include "AbilityTypes.h"


// NEEDED engine changes for non native net serialization.
// Add ENGINE_API to:
//    - FRepLayout::SendProperties_BackwardsCompatible
//    - FRepLayout::ReceiveProperties_BackwardsCompatible
//    - UNetDriver::GetStructRepLayout
#define ENABLE_NON_NATIVE_NETSERIALIZATION true


PRAGMA_DISABLE_DEPRECATION_WARNINGS
class FStructNetSerializer
{
private:

	// This is an acceleration so if we make back to back requests for the same type
	// we don't have to do repeated lookups.
	struct FCachedRequestState
	{
		UScriptStruct* Struct = nullptr;
		TSharedPtr<FRepLayout> RepLayout;
	};


	UNetDriver* Driver;
	FCachedRequestState CachedRequestState;


public:

	FStructNetSerializer() : Driver(nullptr) { check(0); }
	FStructNetSerializer(UNetDriver* InNetDriver) : Driver(InNetDriver) {}

public:

	bool NetSerialize(UStruct* Struct, FArchive& InAr, UPackageMap* Map, void* Data, bool& bSuccess);

private:

	#if ENABLE_NON_NATIVE_NETSERIALIZATION
	void UpdateCachedRepLayout();
	#endif

	void UpdateCachedState(UStruct* Struct);
};
PRAGMA_ENABLE_DEPRECATION_WARNINGS
