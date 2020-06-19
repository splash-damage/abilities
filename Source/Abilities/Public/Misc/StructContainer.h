// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include "StructContainer.generated.h"


USTRUCT()
struct FStructContainerItem
{
	GENERATED_BODY()

	UPROPERTY()
	UScriptStruct* StructType = nullptr;

	UPROPERTY()
	int32 Offset = INDEX_NONE;


	bool operator==(const FStructContainerItem& Other)
	{
		return StructType == Other.StructType;
	}

	bool operator==(UScriptStruct* Other) const
	{
		return StructType == Other;
	}

	friend int32 GetTypeHash(const FStructContainerItem& Item)
	{
		return GetTypeHash(Item.StructType);
	}
};


USTRUCT(BlueprintType)
struct ABILITIES_API FStructContainer
{
	GENERATED_BODY()

private:

	UPROPERTY()
	TArray<FStructContainerItem> Structs;

	UPROPERTY()
	TArray<uint8> Data;


public:
	// @TODO: miguel.fernandez - (09/06/20) Add custom Move semantics

	// NOTE: Don't add or remove structs while pointing to a live struct or it will become garbage
	void* Get(UScriptStruct* Struct);

	// NOTE: Don't add or remove structs while pointing to a live struct or it will become garbage
	template<typename Type>
	Type* Get();

	template<typename Type>
	const Type* Get() const;

	int32 GetOffset(UScriptStruct* Struct) const;

	bool Has(UScriptStruct* Struct) const;

	template<typename T>
	bool Has() const { return Has(T::StaticStruct()); }

	int32 Num() const;

	template<typename Type>
	bool Add(const Type& Struct, bool bReplace = true);

	template<typename Type>
	bool Add(Type&& Struct, bool bReplace = true);

	bool Add(FStructProperty* Property, void* StructData, bool bReplace = true);

	void Empty();

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

private:

	int32 GetOrAddUninitialized(UScriptStruct* Struct, bool& bWasAdded);

	template<typename Type>
	Type* GetPtr(int32 Offset)
	{
		return reinterpret_cast<Type*>(&Data[Offset]);
	}

	template<typename Type>
	const Type* GetPtr(int32 Offset) const
	{
		return reinterpret_cast<const Type*>(&Data[Offset]);
	}
};

template<>
struct TStructOpsTypeTraits<FStructContainer> : TStructOpsTypeTraitsBase2<FStructContainer>
{
	enum { WithNetSerializer = true };
};


template<typename Type>
Type* FStructContainer::Get()
{
	const int32 Offset = GetOffset(Type::StaticStruct());
	if(Offset != INDEX_NONE)
	{
		return GetPtr<Type>(Offset);
	}
	return nullptr;
}

template<typename Type>
const Type* FStructContainer::Get() const
{
	const int32 Offset = GetOffset(Type::StaticStruct());
	if (Offset != INDEX_NONE)
	{
		return GetPtr<Type>(Offset);
	}
	return nullptr;
}

inline bool FStructContainer::Has(UScriptStruct* Struct) const
{
	return Struct && Structs.Contains(Struct);
}

inline int32 FStructContainer::Num() const
{
	return Structs.Num();
}

template<typename Type>
inline bool FStructContainer::Add(const Type& Struct, bool bReplace)
{
	bool bWasAdded;
	const int32 Offset = GetOrAddUninitialized(Type::StaticStruct(), bWasAdded);

	if (Offset != INDEX_NONE && (bReplace || bWasAdded))
	{
		Type* const Ptr = GetPtr<Type>(Offset);
		new(Ptr) Type(Struct);
		return true;
	}
	return false;
}

template<typename Type>
inline bool FStructContainer::Add(Type&& Struct, bool bReplace)
{
	bool bWasAdded;
	const int32 Offset = GetOrAddUninitialized(Type::StaticStruct(), bWasAdded);

	if (Offset != INDEX_NONE && (bReplace || bWasAdded))
	{
		Type* const Ptr = GetPtr<Type>(Offset);
		new(Ptr) Type(MoveTempIfPossible(Struct));
		return true;
	}
	return false;
}