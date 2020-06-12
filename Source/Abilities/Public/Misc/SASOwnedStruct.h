// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include "SASOwnedStruct.generated.h"


class UWorld;

USTRUCT()
struct FSASOwnedStruct
{
	GENERATED_BODY()

private:

	TWeakObjectPtr<UObject> Owner;


public:

	virtual ~FSASOwnedStruct() {}

	template<typename OwnerType>
	OwnerType* GetOwner() const;

	UWorld* GetWorld() const
	{
		return Owner.IsValid()? Owner->GetWorld() : nullptr;
	}

	// Called to assign an owner.
	// OnConstruction(Actors), OnRegister(Components) or on Construction (Other objects)
	template<typename OwnerType>
	void Setup(OwnerType& InOwner)
	{
		static_assert(TIsDerivedFrom<OwnerType, UObject>::IsDerived, "Owner must be an UObject");
		Owner = &InOwner;
		OnSetup();
	}

protected:

	virtual void OnSetup() {}
};


template<typename OwnerType>
inline OwnerType* FSASOwnedStruct::GetOwner() const
{
	static_assert(TIsDerivedFrom<OwnerType, UObject>::IsDerived, "Owner must be an UObject");
	if(ensureMsgf(Owner.IsValid(), TEXT("Owner should always be valid. Make sure Setup() is called OnConstruction(Actors) or OnRegister(Components)")))
	{
		return Cast<OwnerType>(Owner.Get());
	}
	return nullptr;
}
