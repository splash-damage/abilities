// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "AbilityBase.h"
#include <Net/UnrealNetwork.h>

#include "Misc/Macros.h"
#include "AbilitiesComponent.h"


void UAbilityBase::OnRep_Owner()
{
	if (Owner)
	{
		DoBeginPlay(Owner);
	}
}

void UAbilityBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (auto* BPClass = Cast<UBlueprintGeneratedClass>(GetClass()))
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(UAbilityBase, Owner);
}
void UAbilityBase::PreDestroyFromReplication()
{
	if(HasBegunPlay() && State != EAbilityState::AfterEndPlay)
	{
		DoEndPlay();
	}

	Super::PreDestroyFromReplication();
}

void UAbilityBase::EventBeginPlay_Implementation()
{
	BeginPlay();
}

void UAbilityBase::EventEndPlay_Implementation()
{
	EndPlay();
}

bool UAbilityBase::SetState(EAbilityState NewState, FStructContainer Container, EAbilityTransitionFlag Flags)
{
	const bool bHasAuthority = HasAuthority();
	const bool bIsLocallyOwned = IsLocallyOwned();
	const FAbilityStateTransition Transition{State, NewState, Flags};

	if(!bHasAuthority && !bIsLocallyOwned)
	{
		return false;
	}

	PushContainer(MoveTemp(Container));
	auto& CurrContainer = GetCurrentContainer();

	DoPreStateChange(Transition);
	if (TrySetLocalState(Transition, CurrContainer))
	{
		if(bHasAuthority)
		{
			SetCurrentStateId(CurrentStateId + 1);
			OnStateChanged(Transition, CurrContainer);
			// Notify clients if state is still the new one
			if (GetState() == Transition.Destination)
			{
				MCSetState(Transition, CurrContainer, CurrentStateId);
			}
		}
		else if (bIsLocallyOwned)
		{
			OnStateChanged(Transition, CurrContainer);
			// Notify server if state is still the new one
			if (GetState() == Transition.Destination)
			{
				ServerSetState(Transition, CurrContainer, GetNewRequestId());
			}
		}

		PopContainer();
		return true;
	}
	PopContainer();
	return false;
}

bool UAbilityBase::ServerSetState_Validate(FAbilityStateTransition Transition, const FStructContainer& Container, uint32 RequestedStateId)
{
	return RequestedStateId > 0;
}

void UAbilityBase::ServerSetState_Implementation(FAbilityStateTransition Transition, const FStructContainer& Container, uint32 RequestedStateId)
{
	Transition.Origin = State;
	PushContainer(Container);
	auto& CurrContainer = GetCurrentContainer();

	if (RequestedStateId > CurrentStateId &&
		TrySetLocalState(Transition, CurrContainer))
	{
		SetCurrentStateId(RequestedStateId);

		OnStateChanged(Transition, CurrContainer);
		// Notify clients if state is still the new one
		if (GetState() == Transition.Destination)
		{
			MCSetState(Transition, CurrContainer, CurrentStateId);
		}
	}
	else // Reject change request
	{
		FAbilityStateTransition RejectedTransition { Transition.Destination, State, Transition.Flags };
		RejectedTransition.Flags |= EAbilityTransitionFlag::PredictionFailed;
		ClientRejectState(RejectedTransition, RequestedStateId);
	}
	PopContainer();
}

void UAbilityBase::ClientRejectState_Implementation(FAbilityStateTransition Transition, uint32 RequestedStateId)
{
	if (CurrentStateId > RequestedStateId)
	{
		// Another state change arrived before. This one is discarded
		return;
	}

	// State change was rejected. Revert local state and notify
	if(State != Transition.Destination)
	{
		Transition.Origin = State;
		State = Transition.Destination;
		OnStateChanged(Transition, {});
	}
}

void UAbilityBase::MCSetState_Implementation(FAbilityStateTransition Transition, const FStructContainer& Container, uint32 ServerStateId)
{
	if(HasAuthority())
	{
		return;
	}

	SetCurrentStateId(ServerStateId);

	if(State != Transition.Destination)
	{
		Transition.Origin = State;
		State = Transition.Destination;
		OnStateChanged(Transition, Container);
	}
}

bool UAbilityBase::TrySetLocalState(FAbilityStateTransition Transition, const FStructContainer& Container)
{
	if (!HasBegunPlay() ||
		State == Transition.Destination ||
		Transition.Origin == Transition.Destination ||
		ForbiddenStates.Contains(Transition.Destination) ||
		!CheckTransition(Transition, Container))
	{
		return false;
	}

	State = Transition.Destination;
	return true;
}

void UAbilityBase::DoBeginPlay(UAbilitiesComponent* InOwner)
{
	Owner = InOwner;
	if (State != EAbilityState::BeforeBeginPlay)
	{
		return;
	}
	State = EAbilityState::JustEquipped;

	// Notify
	EventBeginPlay();
	OnStateChanged({ EAbilityState::BeforeBeginPlay, State }, {});
}

void UAbilityBase::DoEndPlay()
{
	EventEndPlay();

	const EAbilityState LastState = State;
	State = EAbilityState::AfterEndPlay;
	OnStateChanged({LastState, State}, {});
}

UAbilitiesComponent* UAbilityBase::GetAbilitiesComponent() const
{
	MakeSureMsg(Owner, TEXT("Owner must always be valid (Ability: %s)"), *GetName()) nullptr;
	return Owner;
}

AActor* UAbilityBase::GetOwner() const
{
	return GetAbilitiesComponent()->GetOwner();
}

bool UAbilityBase::HasAuthority() const
{
	return GetAbilitiesComponent()->HasAuthority();
}

bool UAbilityBase::IsLocallyOwned() const
{
	return GetAbilitiesComponent()->IsLocallyOwned();
}
