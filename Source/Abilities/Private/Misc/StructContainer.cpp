// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "Misc/StructContainer.h"
#include "Misc/StructContainerLibrary.h"
#include <UObject/CoreNet.h>

#include "Misc/Macros.h"
#include "Misc/Serialization.h"


void* FStructContainer::Get(UScriptStruct* Struct)
{
	const int32 Offset = GetOffset(Struct);
	return (Offset != INDEX_NONE)? &Data[Offset] : nullptr;
}

int32 FStructContainer::GetOffset(UScriptStruct* Struct) const
{
	MakeSure(Struct) {};

	auto* const Item = Structs.FindByKey(Struct);
	return Item? Item->Offset : INDEX_NONE;
}

bool FStructContainer::Add(UStructProperty* Property, void* StructData, bool bReplace)
{
	bool bWasAdded;
	const int32 Offset = GetOrAddUninitialized(Property->Struct, bWasAdded);

	if(Offset != INDEX_NONE && (bReplace || bWasAdded))
	{
		Property->Struct->CopyScriptStruct(&Data[Offset], StructData, 1);
		return true;
	}
	return false;
}

int32 FStructContainer::GetOrAddUninitialized(UScriptStruct* Struct, bool& bOutWasAdded)
{
	bOutWasAdded = false;
	if(!Struct)
	{
		return {};
	}
	else if (auto* Layout = Structs.FindByKey(Struct))
	{
		return Layout->Offset;
	}

	const int32 Offset = Data.AddUninitialized(Struct->GetStructureSize());
	Structs.Add({ Struct, Offset });
	bOutWasAdded = true;
	return Offset;
}

void FStructContainer::Empty()
{
	Structs.Empty();
	Data.Empty();
}

bool FStructContainer::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint16 Num = Structs.Num();
	Ar << Num;
	if (Num <= 0)
	{
		if(Ar.IsLoading())
		{
			Structs.Empty();
			Data.Empty();
		}
		bOutSuccess = true;
		return true;
	}

	const auto* World = Map->GetWorld();
	check(World);
	FStructNetSerializer StructSerializer{ World->GetNetDriver() };

	int32 DataSize = Data.Num();
	Ar << DataSize;

	if(Ar.IsLoading())
	{
		// Resize memory to fit serialized size
		Structs.SetNum(Num, true);
		Data.Empty(DataSize);
	}

	for(auto& LayoutStruct : Structs)
	{
		Ar << LayoutStruct.StructType;
	}

	if(Ar.IsLoading())
	{
		// Assign offsets, reserve memory and initialize structs
		for(auto& LayoutStruct : Structs)
		{
			const int32 Size = LayoutStruct.StructType->GetStructureSize();
			LayoutStruct.Offset = Data.AddUninitialized(Size);

			void* const StructPtr = Data.GetData() + LayoutStruct.Offset;
			LayoutStruct.StructType->InitializeStruct(StructPtr, 1);
		}
	}

	// Serialize all structs
	for(auto& LayoutStruct : Structs)
	{
		void* const StructPtr = Data.GetData() + LayoutStruct.Offset;

		bool bStructSuccess = true;
		StructSerializer.NetSerialize(LayoutStruct.StructType, Ar, Map, StructPtr, bStructSuccess);
		bOutSuccess &= bStructSuccess;
	}
	return bOutSuccess;
}


bool UStructContainerLibrary::Generic_AddStruct(FStructContainer& Container, UProperty* Property, void* DataPtr, bool bReplace)
{
	auto* StructProp = Cast<UStructProperty>(Property);
	MakeSureMsg(StructProp, TEXT("UStructContainerLibrary::AddStruct: Only structs can be added. Any other type is not supported.")) false;

	return Container.Add(StructProp, DataPtr, bReplace);
}

bool UStructContainerLibrary::Generic_GetStruct(FStructContainer& Container, UProperty* Property, void* DataPtr)
{
	auto* StructProp = Cast<UStructProperty>(Property);
	MakeSureMsg(StructProp, TEXT("UStructContainerLibrary::GetStruct: Only structs can be contained. Any other type is not supported.")) false;

	if(void* StructPtr = Container.Get(StructProp->Struct))
	{
		StructProp->Struct->CopyScriptStruct(DataPtr, StructPtr, 1);
		return true;
	}
	return false;
}
