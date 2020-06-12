// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Engine/World.h>

#include "AbilityTypes.h"
#include "Misc/StructContainer.h"
#include "AbilityBase.generated.h"

class UAbilitiesComponent;


/** Parent Ability class containing replication & helper features.
 * Use UAbility
 */
UCLASS()
class ABILITIES_API UAbilityBase : public UObject
{
	GENERATED_BODY()

	friend UAbilitiesComponent;


	/************************************************************************/
	/* PROPERTIES                                                           */
	/************************************************************************/
protected:

	UPROPERTY()
	TSet<EAbilityState> ForbiddenStates;

	UPROPERTY()
	uint32 CurrentStateId = 0;

	UPROPERTY()
	uint32 LastRequestedStateId = 0;


	/** Runtime Properties */

	UPROPERTY(BlueprintReadWrite, Category = Ability, meta=(NoSetter))
	FStructContainer CurrentContainer;

private:

	UPROPERTY(Transient)
	EAbilityState State = EAbilityState::BeforeBeginPlay;

	UPROPERTY(ReplicatedUsing = "OnRep_Owner")
	UAbilitiesComponent* Owner;


	UFUNCTION()
	void OnRep_Owner();


	/************************************************************************/
	/* METHODS                                                              */
	/************************************************************************/
public:

	UAbilityBase() : Super()
		, ForbiddenStates{ EAbilityState::BeforeBeginPlay, EAbilityState::AfterEndPlay }
		, Owner(nullptr)
	{}

	/** BEGIN UObject */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty> & OutLifetimeProps) const override;
	virtual void PreDestroyFromReplication() override;
	/** END UObject */

protected:

	virtual void BeginPlay() {}

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "Begin Play"))
	void EventBeginPlay();

	virtual void Tick(float DeltaTime) {}

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, meta = (DisplayName = "Tick"))
	void EventTick(float DeltaTime);

	virtual void EndPlay() {}

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "End Play"))
	void EventEndPlay();

	// Tries to set a new state to the ability
	// @network: Owning Client or Server
	UFUNCTION(BlueprintCallable, Category = Ability)
	bool SetState(EAbilityState NewState, FStructContainer Container, EAbilityTransitionFlag Flags = EAbilityTransitionFlag::None);

	bool SetState(EAbilityState NewState, EAbilityTransitionFlag Flags = EAbilityTransitionFlag::None)
	{
		return SetState(NewState, {}, Flags);
	}

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetState(FAbilityStateTransition Transition, const FStructContainer& Container, uint32 RequestedStateId);

	UFUNCTION(Client, Reliable)
	void ClientRejectState(FAbilityStateTransition Transition, uint32 RequestedStateId);

	UFUNCTION(NetMulticast, Reliable)
	void MCSetState(FAbilityStateTransition Transition, const FStructContainer& Container, uint32 ServerStateId);

	bool TrySetLocalState(FAbilityStateTransition Transition, const FStructContainer& Container);

	virtual bool CheckTransition(FAbilityStateTransition Transition, const FStructContainer& Container);

	virtual void OnStateChanged(FAbilityStateTransition Transition, const FStructContainer& Container) {}

	void DoPreStateChange(FAbilityStateTransition Transition);

	// Called before an state is set to fill the container with structs
	virtual void PreStateChange(FAbilityStateTransition Transition) {}

	// Called before an state is set to fill the container with structs
	// @return true if modified
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, meta = (DisplayName = "Pre-State Change"))
	bool EventPreStateChange(FAbilityStateTransition Transition);

private:

	void DoBeginPlay(UAbilitiesComponent* InOwner);
	void DoTick(float DeltaTime)
	{
		Tick(DeltaTime);
		EventTick(DeltaTime);
	}
	void DoEndPlay();

	void SetCurrentStateId(int32 NewId)
	{
		LastRequestedStateId = 0;
		CurrentStateId = NewId;
	}

	int32 GetNewRequestId()
	{
		LastRequestedStateId = (LastRequestedStateId > CurrentStateId? LastRequestedStateId : CurrentStateId) + 1;
		return LastRequestedStateId;
	}


	/************************************************************************/
	/* HELPERS                                                              */
	/************************************************************************/
public:

	UFUNCTION(BlueprintPure, Category = Ability)
	EAbilityState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = Ability)
	UAbilitiesComponent* GetAbilitiesComponent() const;

	UFUNCTION(BlueprintPure, Category = Ability)
	AActor* GetOwner() const;

	UFUNCTION(BlueprintPure, Category = Ability)
	bool HasBegunPlay() const;

	UFUNCTION(BlueprintPure, Category = Ability)
	bool HasAuthority() const;

	UFUNCTION(BlueprintPure, Category = Ability)
	bool IsLocallyOwned() const;

	void PushContainer(const FStructContainer& Container);
	void PopContainer();

	FStructContainer& GetCurrentContainer() { return CurrentContainer; }
	const FStructContainer& GetCurrentContainer() const { return CurrentContainer; }
};

inline bool UAbilityBase::CheckTransition(FAbilityStateTransition Transition, const FStructContainer& Container)
{
	// Dont allow transition to Cast from Activation. Can be changed
	static const FAbilityStateTransition ActivationToCast { EAbilityState::Activation, EAbilityState::Cast };
	return Transition != ActivationToCast;
}

inline void UAbilityBase::DoPreStateChange(FAbilityStateTransition Transition)
{
	PreStateChange(Transition);
	EventPreStateChange(Transition);
}

inline bool UAbilityBase::HasBegunPlay() const
{
	return Owner != nullptr &&
		State != EAbilityState::BeforeBeginPlay;
}

inline void UAbilityBase::PushContainer(const FStructContainer& Container)
{
	CurrentContainer = Container;
}

inline void UAbilityBase::PopContainer()
{
	CurrentContainer = {};
}
