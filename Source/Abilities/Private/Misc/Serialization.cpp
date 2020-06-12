// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "Misc/Serialization.h"
#include <Engine/PackageMapClient.h>

#include "AbilitiesModule.h"


PRAGMA_DISABLE_DEPRECATION_WARNINGS
bool FStructNetSerializer::NetSerialize(UStruct* Struct, FArchive& InAr, UPackageMap* Map, void* Data, bool& bSuccess)
{
	UpdateCachedState(Struct);
	FBitArchive& Ar = static_cast<FBitArchive&>(InAr);

	if (EnumHasAnyFlags(CachedRequestState.Struct->StructFlags, STRUCT_NetSerializeNative))
	{
		UScriptStruct::ICppStructOps* CppStructOps = CachedRequestState.Struct->GetCppStructOps();
		check(CppStructOps);

		return CppStructOps->NetSerialize(Ar, Map, bSuccess, Data);
	}
	else
	{
		#if ENABLE_NON_NATIVE_NETSERIALIZATION
		UpdateCachedRepLayout();
		auto* PackageMapClient = Cast<UPackageMapClient>(Map);

		if (PackageMapClient && PackageMapClient->GetConnection()->InternalAck)
		{
			if (Ar.IsSaving())
			{
				TArray<uint16> Changed;
				CachedRequestState.RepLayout->SendProperties_BackwardsCompatible(nullptr, nullptr, (uint8*)Data, PackageMapClient->GetConnection(), static_cast<FNetBitWriter&>(Ar), Changed);
			}
			else
			{
				bool bHasGuidsChanged = false;
				CachedRequestState.RepLayout->ReceiveProperties_BackwardsCompatible(PackageMapClient->GetConnection(), nullptr, Data, static_cast<FNetBitReader&>(Ar), bSuccess, false, bHasGuidsChanged);
			}
		}
		else
		{
			CachedRequestState.RepLayout->SerializePropertiesForStruct(Struct, Ar, Map, Data, bSuccess);
		}
		return true;
		#else
		UE_LOG(LogAbilities, Error, TEXT("Tried to serialize an struct '%s' without native NetSerialize. Non-native net serialization can be enabled with ENABLE_NON_NATIVE_NETSERIALIZATION"), *Struct->GetName());
		bSuccess = false;
		return false;
		#endif
	}
}

#if ENABLE_NON_NATIVE_NETSERIALIZATION
void FStructNetSerializer::UpdateCachedRepLayout()
{
	if (!CachedRequestState.RepLayout.IsValid())
	{
		CachedRequestState.RepLayout = Driver->GetStructRepLayout(CachedRequestState.Struct);
	}
}
#endif

void FStructNetSerializer::UpdateCachedState(UStruct* Struct)
{
	if (CachedRequestState.Struct != Struct)
	{
		CachedRequestState.Struct = CastChecked<UScriptStruct>(Struct);
		CachedRequestState.RepLayout.Reset();
	}
}
PRAGMA_ENABLE_DEPRECATION_WARNINGS
