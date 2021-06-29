
// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "Ability.h"
#include <Engine/ActorChannel.h>
#include <Engine/BlueprintGeneratedClass.h>
#include <Engine/NetDriver.h>
#include <Engine/World.h>
#include <Net/UnrealNetwork.h>
#include <TimerManager.h>

#include "AbilitiesComponent.h"
#include "AbilitiesCooldownCounter.h"
#include "AbilitiesModule.h"
#include "Misc/Macros.h"


void UAbility::BeginPlay()
{
	if(!bHasCast)
	{
		ForbiddenStates.Add(EAbilityState::Cast);
	}

	Super::BeginPlay();
}

bool UAbility::CheckTransition(FAbilityStateTransition Transition, const FStructContainer& Container)
{
	if (!Super::CheckTransition(Transition, Container))
	{
		return false;
	}

	if (Transition.Destination == EAbilityState::Cast ||
		Transition.Destination == EAbilityState::Activation)
	{
		auto* const Comp = GetAbilitiesComponent();
		// Check if we have required and denied tags on the component
		if (!Comp || IsCoolingDown() ||
			!Comp->GetTags().HasAllExact(RequiredTags) ||
			 Comp->GetTags().HasAnyExact(RequiredToNotHaveTags))
		{
			return false;
		}
	}

	switch(Transition.Destination)
	{
	case EAbilityState::Cast:
		return EventCanCast(Container);
	case EAbilityState::Activation:
		return EventCanActivate(Container);
	}
	return true;
}

void UAbility::OnStateChanged(FAbilityStateTransition Transition, const FStructContainer& Container)
{
	Super::OnStateChanged(Transition, Container);

	const bool bPredictionFailed = EnumHasAnyFlags(Transition.Flags, EAbilityTransitionFlag::PredictionFailed);
	const bool bWantsToCooldown  = EnumHasAnyFlags(Transition.Flags, EAbilityTransitionFlag::StartCooldown);

	auto* const Comp = GetAbilitiesComponent();
	if(!Comp)
	{
		return;
	}

	// Stop old state
	switch(Transition.Origin)
	{
	case EAbilityState::Cast:
		// Apply pre-cast effects
		Comp->AddTags(CastFinishAddTags);
		Comp->RemoveTags(CastFinishRemoveTags);

		EventCastFinish();

		if (TickMode == EAbilityTickMode::DuringCastOnly)
		{
			Comp->TickingAbilities.Remove(this);
		}
		break;

	case EAbilityState::Activation:
		// Apply post-deactivation effects
		Comp->AddTags(DeactivationAddTags);
		Comp->RemoveTags(DeactivationRemoveTags);

		EventDeactivate();
		if (TickMode == EAbilityTickMode::DuringActivationOnly ||
			TickMode == EAbilityTickMode::DuringCastAndActivation)
		{
			Comp->TickingAbilities.Remove(this);
		}

		if(bPredictionFailed)
		{
			LocalResetCooldown();

			if(bBackToCastingIfPredictionFailed && bHasCast)
			{
				// Will activate Cast locally and ensure that the server is casting too
				StartCast({});
			}
		}
		else if (bWantsToCooldown && CooldownMode == EAbilityCooldownMode::OnDeactivation)
		{
			LocalStartCooldown();
		}
		break;

	case EAbilityState::BeforeBeginPlay:
		if (TickMode == EAbilityTickMode::Always)
		{
			Comp->TickingAbilities.Add(this);
		}
	}

	// If this events changed the state,
	// don't continue with the destination state events
	if (GetState() != Transition.Destination)
	{
		return;
	}

	// Start new state
	switch(Transition.Destination)
	{
	case EAbilityState::Cast:
		// Apply pre-cast effects
		Comp->AddTags(CastStartAddTags);
		Comp->RemoveTags(CastStartRemoveTags);

		EventCast(Container);

		if (TickMode == EAbilityTickMode::DuringCastOnly ||
			TickMode == EAbilityTickMode::DuringCastAndActivation)
		{
			Comp->TickingAbilities.Add(this);
		}
		break;

	case EAbilityState::Activation:
		if (CooldownMode == EAbilityCooldownMode::OnActivation)
		{
			LocalStartCooldown();
		}

		// Apply pre-activation effects
		Comp->AddTags(ActivationAddTags);
		Comp->RemoveTags(ActivationRemoveTags);

		EventActivate(Container);
		if (TickMode == EAbilityTickMode::DuringActivationOnly)
		{
			Comp->TickingAbilities.Add(this);
		}
		break;

	case EAbilityState::AfterEndPlay:
		if (TickMode == EAbilityTickMode::Always)
		{
			Comp->TickingAbilities.Remove(this);
		}
	}
}

void UAbility::OnCast(const FStructContainer& Container)
{
	ABILITY_VLOG_LOCATION(25.f, FColor::Cyan, TEXT("'%s' casted"), *GetClass()->GetName());
}

void UAbility::ActivateOrCancel(FStructContainer Container)
{
	if (IsCasting() && !Activate(Container))
	{
		// Tried to activate but couldnt, so at least cancel cast
		Cancel(false);
	}
}

void UAbility::MCStartCooldown_Implementation()
{
	LocalStartCooldown();
}

void UAbility::MCResetCooldown_Implementation()
{
	LocalResetCooldown();
}

void UAbility::LocalStartCooldown()
{
	if (!HasCooldown() || IsCoolingDown())
	{
		return;
	}

	if(auto* const Comp = GetAbilitiesComponent())
	{
		Comp->GetCooldowns().Start(GetClass(), CooldownDuration);

		OnCooldownStarted();
		EventOnCooldownStarted();
	}
}

void UAbility::LocalResetCooldown()
{
	auto* const Comp = GetAbilitiesComponent();

	// No need to check if its cooling down as just reset is more efficient
	if (Comp && Comp->GetCooldowns().Reset(GetClass()))
	{
		NotifyCooldownReady(ECooldownReadyReason::Resseted);
	}
}

void UAbility::NotifyCooldownReady(ECooldownReadyReason Reason)
{
	OnCooldownReady(Reason);
	EventOnCooldownReady(Reason);

	if (Reason == ECooldownReadyReason::Finished &&
		bInputWaitForCooldown && IsPressed())
	{
		switch (InputProfile)
		{
		case EAbilityInputProfile::CastWhileHolding:
			StartCast();
			break;
		case EAbilityInputProfile::ActivateWhileHolding:
			Activate();
			break;
		}
	}
}

void UAbility::Cancel(bool bApplyCooldown)
{
	SetState(EAbilityState::Cancelled, bApplyCooldown?
		EAbilityTransitionFlag::StartCooldown : EAbilityTransitionFlag::None
	);
}

void UAbility::OnTagsChanged(const FGameplayTagContainer& Tags)
{
	EventOnTagsChanged(Tags);

	if (InterruptWithTags.Num() <= 0)
	{
		return;
	}

	// Interrupt ability by tag
	if ((bInterruptionCancelsActivation && IsActivated()) ||
	   ( bInterruptionCancelsCasting && IsCasting()))
	{
		if (Tags.HasAnyExact(InterruptWithTags))
		{
			Cancel(true);
		}
	}
}

bool UAbility::IsCoolingDown() const
{
	const auto* const Comp = GetAbilitiesComponent();
	return Comp && Comp->GetCooldowns().IsCoolingDown(GetClass());
}

void UAbility::PressInput(FName Event, FName& PreviousEvent)
{
	if (PressedEvent == Event)
	{
		return;
	}
	else if (IsPressed())
	{
		// Replace event
		PreviousEvent = PressedEvent;
		PressedEvent = Event;
		return;
	}

	PressedEvent = Event;
	OnInputPressed();
}

void UAbility::ReleaseInput()
{
	if (PressedEvent.IsNone())
	{
		return;
	}

	PressedEvent = {};
	OnInputReleased();
}

bool UAbility::CancelInput()
{
	if (PressedEvent.IsNone())
	{
		return false;
	}

	PressedEvent = {};
	return EventOnCancelInput();
}

void UAbility::OnInputPressed()
{
	switch (InputProfile)
	{
	case EAbilityInputProfile::CastWhileHolding:
		StartCast();
		break;
	case EAbilityInputProfile::ActivateOnPress:
	case EAbilityInputProfile::ActivateWhileHolding:
		Activate();
		break;
	case EAbilityInputProfile::ToggleActivationOnPress:
		if(!IsActivated())
		{
			Activate();
		}
		else
		{
			Deactivate();
		}
		break;
	}

	EventOnInputPressed();
}

void UAbility::OnInputReleased()
{
	switch (InputProfile)
	{
	case EAbilityInputProfile::CastWhileHolding:
		if (bHasCast && IsCasting())
		{
			if(!Activate())
			{
				Cancel(false);
			}
		}
		break;
	case EAbilityInputProfile::ActivateWhileHolding:
		Deactivate();
		break;
	}

	EventOnInputReleased();
}

bool UAbility::OnCancelInput()
{
	return false;
}

bool UAbility::EventOnCancelInput_Implementation()
{
	return OnCancelInput();
}

bool UAbility::EventCanCast_Implementation(const FStructContainer& Container) const
{
	return CanCast(Container);
}

void UAbility::EventCast_Implementation(const FStructContainer& Container)
{
	OnCast(Container);
}

void UAbility::EventCastFinish_Implementation()
{
	OnCastFinish();
}

bool UAbility::EventCanActivate_Implementation(const FStructContainer& Container) const
{
	return CanActivate(Container);
}

void UAbility::EventActivate_Implementation(const FStructContainer& Container)
{
	OnActivation(Container);
}

void UAbility::EventDeactivate_Implementation()
{
	OnDeactivation();
}

UWorld* UAbility::GetWorld() const
{
	// If we are a CDO, we must return nullptr to fool UObject::ImplementsGetWorld.
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	const auto* const Comp = GetAbilitiesComponent();
	return Comp? Comp->GetWorld() : nullptr;
}

int32 UAbility::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return FunctionCallspace::Local;
	}
	check(GetOuter() != nullptr);
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UAbility::CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter() != nullptr);

	AActor* LocalOwner = CastChecked<AActor>(GetOuter());
	if (UNetDriver* NetDriver = LocalOwner->GetNetDriver())
	{
		NetDriver->ProcessRemoteFunction(LocalOwner, Function, Parameters, OutParms, Stack, this);
		return true;
	}

	return false;
}

void UAbility::PreDestroyFromReplication()
{
	Cancel(false);
	Super::PreDestroyFromReplication();
}

#if WITH_EDITOR
bool UAbility::CanEditChange(const FProperty* InProperty) const
{
	bool bCanEdit = Super::CanEditChange(InProperty);

	const FName PropertyName = InProperty ? InProperty->GetFName() : NAME_None;
	if (GET_MEMBER_NAME_CHECKED(UAbility, bInputWaitForCooldown) == PropertyName)
	{
		bCanEdit &= InputProfile != EAbilityInputProfile::ActivateOnPress;
	}
	return bCanEdit;
}
#endif //WITH_EDITOR
