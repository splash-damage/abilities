// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <AssetTypeActions_Base.h>

#include "Factories/Factory.h"
#include "Buff.h"

#include "BuffFactory.generated.h"


UCLASS()
class ABILITIESEDITOR_API UBuffFactory : public UFactory
{
	GENERATED_BODY()

	UPROPERTY()
	UClass* BuffClass;

public:

	UBuffFactory();

	bool ConfigureProperties();
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	bool CanCreateNew() const;
};
