// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include <CoreMinimal.h>

#include "Helpers/TestHelpers.h"
#include "Helpers/TestStructs.h"


#if WITH_DEV_AUTOMATION_TESTS

/************************************************************************/
/* STRUCT CONTAINER SPEC                                                */
/************************************************************************/


class FAbilityTestSpec_StructContainer : public FAbilityTestSpec
{
	GENERATE_SPEC(FAbilityTestSpec_StructContainer, "Abilities.StructContainer",
		EAutomationTestFlags::ProductFilter |
		EAutomationTestFlags::EditorContext |
		EAutomationTestFlags::ServerContext
	);

	FStructContainer Container;

	FStructTest One;
	FOtherStructTest Other;

	FAbilityTestSpec_StructContainer()
	{
		bUseWorld = false;
	}
};

void FAbilityTestSpec_StructContainer::Define()
{
	BeforeEach([this]()
	{
		Container = {};
		One = {};
		One.Location = {1.f, 2.f, 3.f};
		Other = {};
		Other.OtherLocation = {2.f, 0.f, 2.f};
	});

	It("Can be empty", [this]()
	{
		TestEqual(TEXT("Container is empty"), Container.Num(), 0);
	});

	It("Can add an struct", [this]()
	{
		TestTrue(TEXT("Container added an struct"), Container.Add<FStructTest>(One));

		TestTrue(TEXT("Container has the struct"), Container.Has<FStructTest>());

		TestEqual(TEXT("Container size"), Container.Num(), 1);
		FStructTest* Value = Container.Get<FStructTest>();
		TestNotNull(TEXT("Container has the struct"), Value);
		if(!HasAnyErrors())
		{
			TestEqual(TEXT("Value is kept"), Value->Location, FVector{1.f, 2.f, 3.f});
		}
	});

	It("Can't add the same struct", [this]()
	{
		TestTrue(TEXT("Container added an struct"), Container.Add<FStructTest>(One));
		One.Location.Z = 10.f;
		TestFalse(TEXT("Container added another struct"), Container.Add<FStructTest>(One, false));

		TestEqual(TEXT("Container size"), Container.Num(), 1);
		FStructTest* Value = Container.Get<FStructTest>();
		TestNotNull(TEXT("Container has the struct"), Value);
		if(!HasAnyErrors())
		{
			TestEqual(TEXT("Value is kept"), Value->Location, FVector{1.f, 2.f, 3.f});
		}
	});

	It("Can replace an struct", [this]()
	{
		TestTrue(TEXT("Container added an struct"), Container.Add<FStructTest>(One));
		One.Location.Z = 10.f;
		TestTrue(TEXT("Container added another struct"), Container.Add<FStructTest>(One, true));

		TestEqual(TEXT("Container size"), Container.Num(), 1);
		FStructTest* Value = Container.Get<FStructTest>();
		TestNotNull(TEXT("Container has the struct"), Value);
		if(!HasAnyErrors())
		{
			TestEqual(TEXT("Value is kept"), Value->Location, FVector{1.f, 2.f, 10.f});
		}
	});

	It("Can add two different structs", [this]()
	{
		TestTrue(TEXT("Container added an struct"), Container.Add<FStructTest>(One));
		TestTrue(TEXT("Container added a second struct"), Container.Add<FOtherStructTest>(Other));

		TestEqual(TEXT("Container size"), Container.Num(), 2);
	});

	AfterEach([this]()
	{
	});
}

#endif //WITH_DEV_AUTOMATION_TESTS
