// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include <CoreMinimal.h>

#include "Helpers/TestHelpers.h"
#include "Helpers/TestAbility.h"


#if WITH_DEV_AUTOMATION_TESTS

/************************************************************************/
/* ABILITY SPEC                                                         */
/************************************************************************/

class FAbilityTestSpec_Abilities : public FAbilityTestSpec
{
	GENERATE_SPEC(FAbilityTestSpec_Abilities, "Abilities.Ability",
		EAutomationTestFlags::ProductFilter |
		EAutomationTestFlags::EditorContext |
		EAutomationTestFlags::ServerContext
	);

	UAbilitiesComponent* Component = nullptr;
};

void FAbilityTestSpec_Abilities::Define()
{
	Describe("Execution", [this]()
	{
		BeforeEach([this]()
		{
			CreateWorld();
			Component = AddTestComponent();
			Component->EquipAbility<UTestAbility>();
			Component->EquipAbility<UTestAbility2>();
		});

		It("Can be equipped", [this]()
		{
			TestTrue(TEXT("Is Equipped"), Component->IsEquipped<UTestAbility>());
		});

		It("Can be unequipped", [this]()
		{
			Component->UnequipAbility<UTestAbility>();

			TestFalse(TEXT("Is Equipped after Unequip"), Component->IsEquipped<UTestAbility>());
		});

		It("Can equip again", [this]()
		{
			Component->UnequipAbility<UTestAbility>();
			Component->EquipAbility<UTestAbility>();

			TestTrue(TEXT("Is Equipped after Reequip"), Component->IsEquipped<UTestAbility>());
		});

		It("Calls Beginplay", [this]()
		{
			UTestAbility* Ability = Component->GetEquippedAbility<UTestAbility>();
			TestTrue(TEXT("Called BeginPlay"), Ability->bCalledBeginPlay);
		});

		It("Calls Beginplay on two abilities", [this]()
		{
			UTestAbility* Ability = Component->GetEquippedAbility<UTestAbility>();
			UTestAbility2* Ability2 = Component->GetEquippedAbility<UTestAbility2>();
			TestTrue(TEXT("Called first BeginPlay"), Ability->bCalledBeginPlay);
			TestTrue(TEXT("Called second BeginPlay"), Ability2->bCalledBeginPlay);
		});

		It("Instance is created", [this]()
		{
			TestNotNull(TEXT("Ability Instance after Equip"), Component->GetEquippedAbility<UTestAbility>());
		});

		It("Instance is destroyed", [this]()
		{
			Component->UnequipAbility<UTestAbility>();

			TestNull(TEXT("Ability Instance after Unequip"), Component->GetEquippedAbility<UTestAbility>());
		});

		It("Can cast", [this]()
		{
			UAbility* Ability = Component->GetEquippedAbility<UTestAbility2>();

			TestFalse(TEXT("Is Casting before Cast"), Ability->IsCasting());
			Component->CastAbility<UTestAbility2>();
			TestTrue(TEXT("Is Running after Cast"), Ability->IsCasting());
		});

		It("Can cancel cast", [this]()
		{
			UAbility* Ability = Component->GetEquippedAbility<UTestAbility2>();

			Component->CastAbility<UTestAbility2>();
			TestTrue(TEXT("Is Casting"), Ability->IsCasting());

			Ability->Cancel();

			TestFalse(TEXT("IsRunning after cancel Cast"), Ability->IsRunning());
			TestTrue(TEXT("Ability was Cancelled"), Ability->IsCancelled());
			TestFalse(TEXT("Ability Succeeded"), Ability->HasSucceeded());
		});

		It("Can activate after cast", [this]()
		{
			UAbility* Ability = Component->GetEquippedAbility<UTestAbility2>();

			Component->CastAbility<UTestAbility2>();
			TestTrue(TEXT("Is Casting"), Ability->IsCasting());
			TestFalse(TEXT("IsActivated"), Ability->IsActivated());

			const bool bResult = Ability->Activate();

			TestTrue(TEXT("ActivatedReturn"), bResult);
			TestTrue(TEXT("IsActivated"), Ability->IsActivated());
		});

		It("Can activate directly (no cast required)", [this]()
		{
			UAbility* Ability = Component->GetEquippedAbility<UTestAbility>();

			TestFalse(TEXT("Is Running before Activation"), Ability->IsRunning());
			Component->CastAbility<UTestAbility>();
			TestTrue(TEXT("Is Running after Activation"), Ability->IsRunning());
		});

		It("Can not activate directly (cast required)", [this]()
		{
			UAbility* Ability = Component->GetEquippedAbility<UTestAbility2>();

			const bool bResult = Ability->Activate();
			TestFalse(TEXT("ActivatedReturn"), bResult);
			TestFalse(TEXT("IsActivated"), Ability->IsActivated());
		});

		It("Won't activate if CanActivate is false", [this]()
		{
			UTestAbility* Ability = Component->GetEquippedAbility<UTestAbility>();
			Ability->bEnableActivation = false;

			const bool bResult = Component->CastAbility<UTestAbility>();
			TestFalse(TEXT("ActivatedReturn"), bResult);
			TestFalse(TEXT("IsActivated"), Ability->IsActivated());

			Ability->bEnableActivation = true;
		});

		It("Can deactivate", [this]()
		{
			Component->CastAbility<UTestAbility>();

			UAbility* Ability = Component->GetEquippedAbility<UTestAbility>();

			TestTrue(TEXT("Is Running before Deactivation"), Ability->IsRunning());
			Component->Cancel<UTestAbility>();
			TestFalse(TEXT("Is Running after Deactivation"), Ability->IsRunning());
		});

		It("Can deactivate from OnActivate", [this]()
		{
			Component->EquipAbility<UTestAbility3>();
			TestTrue(TEXT("Activated"), Component->CastAbility<UTestAbility3>());

			UAbility* Ability = Component->GetEquippedAbility<UTestAbility3>();

			// TestAbility3 will deactivate on activation
			TestFalse(TEXT("Is Activated"), Ability->IsActivated());
		});

		It("Can deny activation", [this]()
		{
			UAbility* Ability = Component->GetEquippedAbility<UTestAbility>();
			Cast<UTestAbility>(Ability)->bEnableActivation = false;

			const bool bStarted = Component->CastAbility<UTestAbility>();

			TestFalse(TEXT("Activation result"), bStarted);
			TestFalse(TEXT("Is Running"), Ability->IsRunning());
		});

		AfterEach([this]()
		{
			RemoveTestComponent(Component);
			ShutdownWorld();
		});
	});
}

#endif //WITH_DEV_AUTOMATION_TESTS
