// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <AssetTypeActions_Base.h>

#include "AbilitiesEditor.h"
#include "Buff.h"


class FAssetTypeAction_Buff : public FAssetTypeActions_Base
{
public:

	virtual uint32 GetCategories() override {
		return FAbilitiesEditor::Get().GetAssetCategory();
	}

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;

	virtual UClass* GetSupportedClass() const override
	{
		return UBuff::StaticClass();
	}
};
