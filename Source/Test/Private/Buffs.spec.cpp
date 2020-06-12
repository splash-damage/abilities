// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include <CoreMinimal.h>

#include "Helpers/TestHelpers.h"
#include "Helpers/TestBuff.h"


#if WITH_DEV_AUTOMATION_TESTS

/************************************************************************/
/* BUFFS                                                                */
/************************************************************************/

class FAbilityTestSpec_Buffs : public FAbilityTestSpec
{
	GENERATE_SPEC(FAbilityTestSpec_Buffs, "Abilities.Buff",
		EAutomationTestFlags::ProductFilter |
		EAutomationTestFlags::EditorContext |
		EAutomationTestFlags::ServerContext
	);

	template<typename T>
	T* LoadBuffMock()
	{
		T* Buff = NewObject<T>();
		Buff->AddToRoot();
		return Buff;
	}

	void UnloadBuffMock(UBuff* BuffAsset)
	{
		if (BuffAsset)
		{
			BuffAsset->RemoveFromRoot();
			BuffAsset->MarkPendingKill();
		}
	}

	UAbilitiesComponent* Component = nullptr;
};

void FAbilityTestSpec_Buffs::Define()
{
	BeforeEach([this]()
	{
		CreateWorld();
		Component = AddTestComponent();
	});

	It("Checks if there's no buff", [this]()
	{
		UTestBuff* Buff = LoadBuffMock<UTestBuff>();
		TestFalse("Has Buff", Component->HasBuff(Buff));
		UnloadBuffMock(Buff);
	});

	It("Can apply", [this]()
	{
		UTestBuff* Buff = LoadBuffMock<UTestBuff>();

		const bool bApplied = Component->ApplyBuff(Buff);

		TestTrue("Added", bApplied);
		TestTrue("Has buff", Component->HasBuff(Buff));
		UnloadBuffMock(Buff);
	});

	It("Cant apply twice", [this]()
	{
		UTestBuff* Buff = LoadBuffMock<UTestBuff>();

		TestTrue("Added", Component->ApplyBuff(Buff));
		TestTrue("Has buff", Component->HasBuff(Buff));

		TestFalse("Added again", Component->ApplyBuff(Buff));

		UnloadBuffMock(Buff);
	});

	It("Can remove", [this]()
	{
		UTestBuff* Buff = LoadBuffMock<UTestBuff>();

		TestFalse("Removed before apply", Component->RemoveBuff(Buff));

		TestTrue("Returned apply", Component->ApplyBuff(Buff));
		TestTrue("Has buff", Component->HasBuff(Buff));

		TestTrue("Removed after apply", Component->RemoveBuff(Buff));
		TestFalse("Has buff after remove", Component->HasBuff(Buff));

		UnloadBuffMock(Buff);
	});

	It("Cant remove twice", [this]()
	{
		UTestBuff* Buff = LoadBuffMock<UTestBuff>();

		Component->ApplyBuff(Buff);

		TestTrue("Removed after apply", Component->RemoveBuff(Buff));
		TestFalse("Has buff after remove", Component->HasBuff(Buff));

		TestFalse("Removed again", Component->RemoveBuff(Buff));

		UnloadBuffMock(Buff);
	});

	It("Can remove two buffs", [this]()
	{
		UTestBuff* Buff1 = LoadBuffMock<UTestBuff>();
		UTestBuff* Buff2 = LoadBuffMock<UTestBuff>();

		Component->ApplyBuff(Buff1);
		Component->ApplyBuff(Buff2);

		TestTrue("Removed buff 1", Component->RemoveBuff(Buff1));
		TestTrue("Removed buff 2", Component->RemoveBuff(Buff2));

		UnloadBuffMock(Buff1);
		UnloadBuffMock(Buff2);
	});

	It("Can Apply changes", [this]()
	{
		UTestBuff* Buff = LoadBuffMock<UTestBuff>();

		TestFalse("Applied changes before Add", Buff->bChangesApplied);

		TestTrue("Added", Component->ApplyBuff(Buff));
		TestTrue("Applied changes after Add", Buff->bChangesApplied);

		UnloadBuffMock(Buff);
	});

	It("Can Revert changes", [this]()
	{
		UTestBuff* Buff = LoadBuffMock<UTestBuff>();

		TestTrue("Added", Component->ApplyBuff(Buff));
		TestTrue("Applied changes after Add", Buff->bChangesApplied);

		TestTrue("Removed", Component->RemoveBuff(Buff));
		TestFalse("Applied changes after remove", Buff->bChangesApplied);

		UnloadBuffMock(Buff);
	});

	Describe("Unique", [this]()
	{
		It("Can apply two of the same type if not unique", [this]()
		{
			UTestBuff* Buff1 = LoadBuffMock<UTestBuff>();
			UTestBuff* Buff2 = LoadBuffMock<UTestBuff>();

			Component->ApplyBuff(Buff1);
			Component->ApplyBuff(Buff2);

			TestTrue("Has buff 1", Component->HasBuff(Buff1));
			TestTrue("Has buff 2", Component->HasBuff(Buff2));

			UnloadBuffMock(Buff1);
			UnloadBuffMock(Buff2);
		});

		It("Wont apply another if unique", [this]()
		{
			UTestBuff_Unique* Buff1 = LoadBuffMock<UTestBuff_Unique>();
			UTestBuff_Unique* Buff2 = LoadBuffMock<UTestBuff_Unique>();

			TestTrue("Added first unique", Component->ApplyBuff(Buff1));
			TestFalse("Added second unique", Component->ApplyBuff(Buff2));

			UnloadBuffMock(Buff1);
			UnloadBuffMock(Buff2);
		});

		It("Will apply and replace another unique", [this]()
		{
			UTestBuff_UniqueReplace* Buff1 = LoadBuffMock<UTestBuff_UniqueReplace>();
			UTestBuff_UniqueReplace* Buff2 = LoadBuffMock<UTestBuff_UniqueReplace>();

			TestTrue("Added first unique", Component->ApplyBuff(Buff1));
			TestTrue("Added second unique", Component->ApplyBuff(Buff2));

			TestFalse("Has first buff", Component->HasBuff(Buff1));
			TestTrue("Has second buff", Component->HasBuff(Buff2));

			UnloadBuffMock(Buff1);
			UnloadBuffMock(Buff2);
		});
	});

	Describe("Stackable", [this]()
	{
		It("Can apply one", [this]()
		{
			auto* Buff = LoadBuffMock<UTestBuff_Stackable>();

			TestTrue("Applied a buff", Component->ApplyBuff(Buff));
			TestTrue("Changes applied", Buff->bChangesApplied);
			TestTrue("Buff Count", Buff->LastCount == 1);

			UnloadBuffMock(Buff);
		});

		It("Can apply two", [this]()
		{
			auto* Buff = LoadBuffMock<UTestBuff_Stackable>();

			TestTrue("Applied a buff once", Component->ApplyBuff(Buff));
			TestTrue("Buff Count", Buff->LastCount == 1);
			TestTrue("Applied a buff twice", Component->ApplyBuff(Buff));
			TestTrue("Changes applied", Buff->bChangesApplied);
			TestTrue("Buff Count", Buff->LastCount == 2);

			UnloadBuffMock(Buff);
		});

		It("Can remove one", [this]()
		{
			auto* Buff = LoadBuffMock<UTestBuff_Stackable>();

			TestTrue("Applied a buff", Component->ApplyBuff(Buff));
			TestTrue("Changes applied", Buff->bChangesApplied);

			TestTrue("Removed a buff", Component->RemoveBuff(Buff));
			TestFalse("Changes applied", Buff->bChangesApplied);
			TestTrue("Buff Count", Buff->LastCount == 1);

			UnloadBuffMock(Buff);
		});

		It("Can remove two", [this]()
		{
			auto* Buff = LoadBuffMock<UTestBuff_Stackable>();

			TestTrue("Applied a buff once", Component->ApplyBuff(Buff));
			TestTrue("Applied a buff twice", Component->ApplyBuff(Buff));
			TestTrue("Changes applied", Buff->bChangesApplied);
			TestTrue("Buff Count", Buff->LastCount == 2);

			// ApplyEffects with count 1 should be called here
			TestTrue("Removed a buff", Component->RemoveBuff(Buff));
			TestTrue("Changes applied", Buff->bChangesApplied);
			TestTrue("Buff Count", Buff->LastCount == 1);

			TestTrue("Removed a buff", Component->RemoveBuff(Buff));
			TestFalse("Changes applied", Buff->bChangesApplied);
			TestTrue("Buff Count", Buff->LastCount == 1);

			UnloadBuffMock(Buff);
		});
	});

	AfterEach([this]()
	{
		RemoveTestComponent(Component);
		ShutdownWorld();
	});
}

#endif //WITH_DEV_AUTOMATION_TESTS
