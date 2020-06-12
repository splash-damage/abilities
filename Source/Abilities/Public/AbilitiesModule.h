// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <Modules/ModuleManager.h>


DECLARE_LOG_CATEGORY_EXTERN(LogAbilities, All, All);

class FAbilitiesModule : public IModuleInterface
{
public:

	/** Begin IModuleInterface implementation */
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
	virtual bool SupportsDynamicReloading() override { return true; }
	/** End IModuleInterface implementation */
};