// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include <CoreMinimal.h>

#include "Helpers/TestHelpers.h"


#if WITH_DEV_AUTOMATION_TESTS

/************************************************************************/
/* COMPONENT SPEC                                                       */
/************************************************************************/

class FAbilityTestSpec_Component : public FAbilityTestSpec
{
	GENERATE_SPEC(FAbilityTestSpec_Component, "Abilities.Component",
		EAutomationTestFlags::ProductFilter |
		EAutomationTestFlags::EditorContext |
		EAutomationTestFlags::ServerContext
	);
};

void FAbilityTestSpec_Component::Define()
{
	It("Can be created", [this]()
	{
		AAbilityTestActor* TestActor = AddTestActor();

		TestNotNull(TEXT("Abilities Component"), TestActor->Abilities);

		TestActor->Destroy();
	});
}

#endif //WITH_DEV_AUTOMATION_TESTS
