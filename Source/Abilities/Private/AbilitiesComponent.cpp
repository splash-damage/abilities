// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "AbilitiesComponent.h"
#include <Engine/ActorChannel.h>
#include <Engine/World.h>
#include <GameFramework/Controller.h>
#include <Kismet/KismetSystemLibrary.h>
#include <Net/UnrealNetwork.h>


void UAbilitiesComponent::OnRep_Tags()
{
	if(!HasAuthority())
	{
		NotifyTagsChanged();
	}
}


UAbilitiesComponent::UAbilitiesComponent() : Super()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UAbilitiesComponent::Activate(bool bReset /*= false*/)
{
	Super::Activate(bReset);
	if (HasAuthority())
	{
		ApplyBuffs(InitialBuffs);
		EquipAbilities(InitialAbilities);
	}
}

void UAbilitiesComponent::Deactivate()
{
	if(IsActive())
	{
		bIsTearingDown = true;

		if (HasAuthority())
		{
			UnequipAbilities();
			ResetBuffs();
		}
		Cooldowns.ResetAll();

		bIsTearingDown = false;
	}
	Super::Deactivate();
}

void UAbilitiesComponent::OnRegister()
{
	Super::OnRegister();

	Cooldowns.Setup(*this);
	BuffLifetimes.Setup(*this);
}

void UAbilitiesComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Deactivate();
	Super::EndPlay(EndPlayReason);
}

void UAbilitiesComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BuffLifetimes.Tick();

	for (auto* Ability : TickingAbilities)
	{
		if (IsValid(Ability))
		{
			Ability->DoTick(DeltaTime);
		}
	}
}

bool UAbilitiesComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for(int32 I = 0; I < AllAbilities.Num(); ++I)
	{
		UAbility* Ability = AllAbilities[I];
		if (IsValid(Ability))
		{
			// Lets the ability add sub-objects before replicating its own properties.
			bWroteSomething |= Ability->ReplicateSubobjects(Channel, Bunch, RepFlags);
			bWroteSomething |= Channel->ReplicateSubobject(Ability, *Bunch, *RepFlags);
		}
		else
		{
			// Purge unequipped abilities
			AllAbilities.RemoveAtSwap(I, 1, false);
			--I;
		}
	}
	return bWroteSomething;
}

void UAbilitiesComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAbilitiesComponent, Tags);
	DOREPLIFETIME(UAbilitiesComponent, AllAbilities);
}

void UAbilitiesComponent::EquipAbility(TSubclassOf<UAbility> Class)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!Class)
	{
		UE_LOG(LogAbilities, Log, TEXT("No ability given to equip."));
		return;
	}

	if (EquippedAbilities.Contains(Class))
	{
		UE_LOG(LogAbilities, Log, TEXT("%s ability is already equipped."), *Class.Get()->GetName());
		return;
	}

	InternalEquipAbility(Class.Get());
	EquippedAbilities.Add(Class);
}

void UAbilitiesComponent::EquipAbilities(TSet<TSubclassOf<UAbility>> Classes)
{
	for (const auto& Class : Classes)
	{
		// If not yet equipped
		if (Class.Get() && !GetEquippedAbility(Class))
		{
			EquipAbility(Class.Get());
		}
	}
}

void UAbilitiesComponent::EquipAbilities(TArray<TSubclassOf<UAbility>> Classes)
{
	for (const auto& Class : Classes)
	{
		// If not yet equipped
		if (Class.Get() && !GetEquippedAbility(Class))
		{
			EquipAbility(Class.Get());
		}
	}
}

void UAbilitiesComponent::UnequipAbility(TSubclassOf<UAbility> Class)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!Class)
	{
		UE_LOG(LogAbilities, Log, TEXT("No ability given to unequip."));
		return;
	}

	UAbility* Ability = GetEquippedAbility(Class.Get());
	if (!Ability)
	{
		UE_LOG(LogAbilities, Log, TEXT("Ability %s not equipped."), *Class->GetName());
		return;
	}

	Ability->DoEndPlay();

	AbilityToInstance.Remove(Class);
	EquippedAbilities.Remove(Class);
}

void UAbilitiesComponent::UnequipAbilities()
{
	if (!HasAuthority())
	{
		return;
	}

	TArray<TSubclassOf<UAbility>> CachedEquipped = EquippedAbilities.Array();
	for (const auto& Class : CachedEquipped)
	{
		UnequipAbility(Class.Get());
	}
}

bool UAbilitiesComponent::IsEquipped(TSubclassOf<UAbility> AbilityClass) const
{
	return GetEquippedAbility(AbilityClass) != nullptr;
}

bool UAbilitiesComponent::CanCast(TSubclassOf<UAbility> Class, FStructContainer Container)
{
	if (UAbility* Ability = GetEquippedAbility(Class))
	{
		return Ability->CanCast(Container);
	}
	return false;
}

bool UAbilitiesComponent::CanActivate(TSubclassOf<UAbility> Class, FStructContainer Container)
{
	if (UAbility* Ability = GetEquippedAbility(Class))
	{
		return Ability->CanActivate(Container);
	}
	return false;
}

bool UAbilitiesComponent::IsRunning(TSubclassOf<UAbility> Class) const
{
	UAbility* Ability = GetEquippedAbility(Class);
	return Ability && Ability->IsRunning();
}

bool UAbilitiesComponent::IsCoolingDown(TSubclassOf<UAbility> Class) const
{
	return Cooldowns.IsCoolingDown(Class);
}

float UAbilitiesComponent::GetRemainingCooldown(TSubclassOf<UAbility> Class) const
{
	return Cooldowns.GetRemaining(Class);
}

float UAbilitiesComponent::GetCooldownDuration(TSubclassOf<UAbility> Class) const
{
	if (Class.Get())
	{
		return GetDefault<UAbility>(Class)->CooldownDuration;
	}
	return 0.f;
}

float UAbilitiesComponent::GetBuffRemainingLifetime(UBuff* Buff) const
{
	return BuffLifetimes.GetRemaining(Buff);
}

void UAbilitiesComponent::OnRep_AllAbilities()
{
	AbilityToInstance.Empty(AllAbilities.Num());
	AbilityToInstance.Reserve(AllAbilities.Num());
	for (auto* Ability : AllAbilities)
	{
		if (IsValid(Ability))
		{
			AbilityToInstance.Add(Ability->GetClass(), Ability);
		}
	}
}

bool UAbilitiesComponent::CastAbility(TSubclassOf<UAbility> Class)
{
	if (UAbility* Ability = GetEquippedAbility(Class))
	{
		return Ability->StartCast();
	}
	return false;
}

void UAbilitiesComponent::PressInput(TSubclassOf<UAbility> Class, FName InputEvent)
{
	if (InputEvent.IsNone())
	{
		return;
	}

	if (UAbility* Ability = GetEquippedAbility(Class))
	{
		UClass*& InputClass = PressedInputs.FindOrAdd(InputEvent);
		if (InputClass)
		{
			// Release any other ability with the same input
			ReleaseInputByClass(InputClass);
		}
		InputClass = Class;

		FName PreviousEvent;
		Ability->PressInput(InputEvent, PreviousEvent);

		if (!PreviousEvent.IsNone())
		{
			// Free previously occupied input
			PressedInputs.Remove(PreviousEvent);
		}
	}
}

bool UAbilitiesComponent::ReleaseInput(FName InputEvent)
{
	if (InputEvent.IsNone())
	{
		return false;
	}

	UClass** InputClass = PressedInputs.Find(InputEvent);
	if (InputClass && *InputClass)
	{
		if (UAbility* Ability = GetEquippedAbility(*InputClass))
		{
			PressedInputs.Remove(InputEvent);
			Ability->ReleaseInput();
			return true;
		}
	}
	return false;
}

bool UAbilitiesComponent::ReleaseInputByClass(TSubclassOf<UAbility> Class)
{
	if (UAbility* Ability = GetEquippedAbility(Class))
	{
		if (Ability->IsPressed())
		{
			PressedInputs.Remove(Ability->PressedEvent);
			Ability->ReleaseInput();
			return true;
		}
	}
	return false;
}

FCancelInputReturn UAbilitiesComponent::CancelInput(FName InputEvent)
{
	if (InputEvent.IsNone())
	{
		return {};
	}

	UClass** InputClass = PressedInputs.Find(InputEvent);
	if (InputClass && *InputClass)
	{
		if (UAbility* Ability = GetEquippedAbility(*InputClass))
		{
			PressedInputs.Remove(InputEvent);

			FCancelInputReturn Return;
			Return.bInputCancelled = true;
			Return.bAbilityCancelled = Ability->CancelInput();
			return Return;
		}
	}
	return {};
}

bool UAbilitiesComponent::Cancel(TSubclassOf<UAbility> Class)
{
	if (!HasAuthority() && !IsLocallyOwned())
	{
		return false;
	}

	if (UAbility* Ability = GetEquippedAbility(Class))
	{
		Ability->Cancel();
		return true;
	}
	return false;
}

void UAbilitiesComponent::CancelAll()
{
	if (HasAuthority() || IsLocallyOwned())
	{
		for (auto* Ability : AllAbilities)
		{
			Ability->Cancel();
		}
	}
}

void UAbilitiesComponent::InternalEquipAbility(UClass* Class)
{
	UAbility* Ability = NewObject<UAbility>(GetOuter(), Class);

	AllAbilities.Add(Ability);
	AbilityToInstance.Add(Class, Ability);

	Ability->DoBeginPlay(this);
}

void UAbilitiesComponent::AddTag(const FGameplayTag& NewTag)
{
	if (NewTag.IsValid())
	{
		Tags.AddTag(NewTag);
		NotifyTagsChanged();
	}
}

void UAbilitiesComponent::AddTags(const FGameplayTagContainer& NewTags)
{
	if(NewTags.Num() > 0)
	{
		Tags.AppendTags(NewTags);
		NotifyTagsChanged();
	}
}

bool UAbilitiesComponent::RemoveTag(const FGameplayTag& NewTag)
{
	if (NewTag.IsValid() && Tags.RemoveTag(NewTag))
	{
		NotifyTagsChanged();
		return true;
	}
	return false;
}

void UAbilitiesComponent::RemoveTags(const FGameplayTagContainer& NewTags)
{
	if(NewTags.Num() > 0)
	{
		Tags.RemoveTags(NewTags);
		NotifyTagsChanged();
	}
}

void UAbilitiesComponent::NotifyTagsChanged()
{
	OnTagsChanged.Broadcast();

	for(auto* Ability : AllAbilities)
	{
		if(ensure(Ability))
		{
			Ability->OnTagsChanged(Tags);
		}
	}
}

bool UAbilitiesComponent::ApplyBuffs(const TSet<FBuffCount>& InBuffs)
{
	TSet<FBuffCount> AppliedBuffs{};
	if (HasAuthority() && InternalApplyBuffs(InBuffs, AppliedBuffs))
	{
		NotifyBuffsChanged(AppliedBuffs, EBuffOperation::Added);
		return true;
	}
	return false;
}

bool UAbilitiesComponent::ApplySingleBuffs(const TSet<UBuff*>& InBuffs)
{
	TSet<FBuffCount> InBuffCounts;
	InBuffCounts.Reserve(InBuffs.Num());
	for (auto* Buff : InBuffs)
	{
		InBuffCounts.Add({ Buff, 1 });
	}
	return ApplyBuffs(InBuffCounts);
}

bool UAbilitiesComponent::ResetBuffs()
{
	if (HasAuthority())
	{
		TSet<FBuffCount> RemovedBuffs = Buffs;
		BuffsByClass.Empty();
		Buffs.Empty();

		for (const FBuffCount& BuffCount : Buffs)
		{
			BuffCount.Buff->DoRevertEffects(this, BuffCount.Count);
		}

		NotifyBuffsChanged(RemovedBuffs, EBuffOperation::Removed);
		return RemovedBuffs.Num() > 0;
	}
	return false;
}

bool UAbilitiesComponent::RemoveBuffs(const TSet<FBuffCount>& InBuffs)
{
	TSet<FBuffCount> RemovedBuffs{};
	if (HasAuthority() && InternalRemoveBuffs(InBuffs, RemovedBuffs))
	{
		NotifyBuffsChanged(RemovedBuffs, EBuffOperation::Removed);
		return true;
	}
	return false;
}

bool UAbilitiesComponent::RemoveAllBuffs(const TSet<UBuff*>& InBuffs)
{
	TSet<FBuffCount> InBuffCounts;
	InBuffCounts.Reserve(InBuffs.Num());
	for (auto* Buff : InBuffs)
	{
		InBuffCounts.Add({ Buff, TNumericLimits<int32>::Max() });
	}
	return RemoveBuffs(InBuffCounts);
}

bool UAbilitiesComponent::RemoveBuffsByTag(FGameplayTag Tag, bool bExact)
{
	if (!Tag.IsValid())
	{
		return false;
	}

	TSet<FBuffCount> BuffsToRemove;
	if(bExact)
	{
		for(const auto& Buff : Buffs)
		{
			check(Buff.Buff);
			if(Buff.Buff->GetTags().HasTagExact(Tag))
			{
				BuffsToRemove.Add(Buff);
			}
		}
	}
	else
	{
		for(const auto& Buff : Buffs)
		{
			check(Buff.Buff);
			if(Buff.Buff->GetTags().HasTag(Tag))
			{
				BuffsToRemove.Add(Buff);
			}
		}
	}
	return RemoveBuffs(BuffsToRemove);
}

bool UAbilitiesComponent::HasBuffs(const TSet<UBuff*>& InBuffs, EAllAny Mode) const
{
	if (Mode == EAllAny::Any)
	{
		for (auto* Buff : InBuffs)
		{
			if (Buffs.Contains(Buff))
			{
				return true;
			}
		}
		return false;
	}
	else if (Mode == EAllAny::All)
	{
		for (auto* Buff : InBuffs)
		{
			if (!Buffs.Contains(Buff))
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void UAbilitiesComponent::GetBuffsOfClass(TSubclassOf<UBuff> Class, TSet<FBuffCount>& OutBuffs) const
{
	OutBuffs.Reset();
	if (!Class)
	{
		return;
	}
	if (const auto* Container = BuffsByClass.Find({ Class.Get() }))
	{
		OutBuffs.Reserve(Container->Buffs.Num());
		for (auto* Buff : Container->Buffs)
		{
			const FBuffCount* BuffCount = Buffs.Find(Buff);

			if (ensureMsgf(BuffCount, TEXT("Container had a buff that is not present in the global list of buffs.")))
			{
				OutBuffs.Add(*BuffCount);
			}
		}
	}
}

bool UAbilitiesComponent::InternalApplyBuffs(const TSet<FBuffCount>& InBuffs, TSet<FBuffCount>& BuffsToApply)
{
	BuffsToApply.Reset();
	if (InBuffs.Num() <= 0)
	{
		return false;
	}
	BuffsToApply.Reserve(InBuffs.Num());

	TSet<UBuff*> BuffsToRemove;
	for (const FBuffCount& BuffCount : InBuffs)
	{
		UBuff* Buff = BuffCount.Buff;
		if (!Buff || (!Buff->IsStackable() && HasBuff(Buff)))
		{
			continue;
		}

		FBuffTypeContainer SearchContainer{ Buff->GetClass() };
		auto* Container = BuffsByClass.Find(SearchContainer);
		if (!Container)
		{
			// Add new container
			SearchContainer.Buffs.Add(Buff);
			BuffsByClass.Add(MoveTemp(SearchContainer));
		}
		else
		{
			if (Buff->IsUnique())
			{
				// Don't add this buff since theres already another one
				// of the same type
				if (!Buff->ReplacesPreviousUnique())
				{
					continue;
				}

				// Replace previous buffs
				BuffsToRemove.Append(Container->Buffs);
				Container->Buffs = { Buff };
			}
			else
			{
				Container->Buffs.Add(Buff);
			}
		}

		BuffsToApply.Add(BuffCount);
	}

	// Remove conflicting Unique Buffs
	RemoveAllBuffs(BuffsToRemove);

	// Apply effects and count
	const bool bHasAuthority = HasAuthority();
	Buffs.Reserve(Buffs.Num() + BuffsToApply.Num());
	if (HasAuthority())
	{
		for (const FBuffCount& InBuffCount : BuffsToApply)
		{
			UBuff* Buff = InBuffCount.Buff;

			if (FBuffCount* BuffCount = Buffs.Find(InBuffCount))
			{
				Buff->DoRevertEffects(this, BuffCount->Count);
				BuffCount->Count += InBuffCount.Count;
				Buff->DoApplyEffects(this, BuffCount->Count);
			}
			else
			{
				Buffs.Add(InBuffCount);
				Buff->DoApplyEffects(this, InBuffCount.Count);
			}
		}
	}
	else
	{
		for (const FBuffCount& InBuffCount : BuffsToApply)
		{
			if (FBuffCount* BuffCount = Buffs.Find(InBuffCount))
			{
				BuffCount->Count += InBuffCount.Count;
			}
			else
			{
				Buffs.Add(InBuffCount);
			}
		}
	}
	Buffs.Shrink();

	BuffLifetimes.Start(BuffsToApply);

	return BuffsToApply.Num() > 0;
}

bool UAbilitiesComponent::InternalRemoveBuffs(const TSet<FBuffCount>& InBuffs, TSet<FBuffCount>& RemovedBuffs)
{
	RemovedBuffs.Reset();
	if (InBuffs.Num() <= 0)
	{
		return false;
	}
	RemovedBuffs.Reserve(InBuffs.Num());

	const bool bHasAuthority = HasAuthority();
	for (FBuffCount InBuffCount : InBuffs)
	{
		UBuff* Buff = InBuffCount.Buff;
		if (!Buff) { continue; }

		FSetElementId Id = Buffs.FindId(InBuffCount);
		if (!Id.IsValidId())
		{
			continue; // This buff was not found
		}

		FBuffCount& BuffCount = Buffs[Id];
		int32& Count = BuffCount.Count;

		if (bHasAuthority)
		{
			Buff->DoRevertEffects(this, Count);
		}

		if (Count > InBuffCount.Count)
		{
			// Removed desired buffs
			RemovedBuffs.Add(InBuffCount);

			// Reapply with remaining count
			Count -= InBuffCount.Count;

			if (bHasAuthority)
			{
				Buff->DoApplyEffects(this, Count);
			}
			continue;
		}

		// Removed count buffs, witch is less than desired
		RemovedBuffs.Add(BuffCount);
		Buffs.Remove(Id);

		{ // Remove from ClassToBuff cache
			FSetElementId ClassId = BuffsByClass.FindId({ Buff->GetClass() });

			if (!ensureMsgf(ClassId.IsValidId(), TEXT("Buff was removed but didn't find a category")))
			{
				continue;
			}

			auto& ContainerBuffs = BuffsByClass[ClassId].Buffs;
			if (ContainerBuffs.Remove(Buff) > 0)
			{
				// Remove container if empty
				if (ContainerBuffs.Num() <= 0)
				{
					BuffsByClass.Remove(ClassId);
				}
			}
		}
	}
	Buffs.Shrink();

	BuffLifetimes.Reset(RemovedBuffs);
	return RemovedBuffs.Num() > 0;
}

void UAbilitiesComponent::NotifyBuffsChanged(const TSet<FBuffCount>& ModifiedBuffs, EBuffOperation Change)
{
	LocalOnBuffsChanged(ModifiedBuffs, Change);

	// Constexpr optimized away at compile-time
	switch (BuffReplication)
	{
	case EBuffReplicationMode::OwningClient:
		if (!IsLocallyOwned())
		{
			ClientOnBuffsChanged(ModifiedBuffs.Array(), Change);
		}
		break;
	case EBuffReplicationMode::AllClients:
		MCOnBuffsChanged(ModifiedBuffs.Array(), Change);
		break;
	}
}

void UAbilitiesComponent::ClientOnBuffsChanged_Implementation(const TArray<FBuffCount>& ModifiedBuffs, EBuffOperation Change)
{
	if (!HasAuthority()) // Ignore server
	{
		const TSet<FBuffCount> BuffsSet{ ModifiedBuffs };
		LocalOnBuffsChanged(BuffsSet, Change);
	}
}

void UAbilitiesComponent::MCOnBuffsChanged_Implementation(const TArray<FBuffCount>& ModifiedBuffs, EBuffOperation Change)
{
	if (!HasAuthority()) // Ignore server
	{
		const TSet<FBuffCount> BuffsSet{ ModifiedBuffs };
		LocalOnBuffsChanged(BuffsSet, Change);
	}
}

void UAbilitiesComponent::LocalOnBuffsChanged(const TSet<FBuffCount>& ModifiedBuffs, EBuffOperation Change)
{
	if (!HasAuthority())
	{
		TSet<FBuffCount> ResultBuffs{};
		switch(Change)
		{
		case EBuffOperation::Added:
			InternalApplyBuffs(ModifiedBuffs, ResultBuffs);
			break;
		case EBuffOperation::Removed:
			InternalRemoveBuffs(ModifiedBuffs, ResultBuffs);
			break;
		}
	}

	switch(Change)
	{
	case EBuffOperation::Added:
		OnBuffsApplied.Broadcast(ModifiedBuffs);
		break;
	case EBuffOperation::Removed:
		OnBuffsRemoved.Broadcast(ModifiedBuffs);
		break;
	}
}

bool UAbilitiesComponent::IsLocallyOwned() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// Let Pawns check with their controller
	if (const APawn* OwnerAsPawn = Cast<APawn>(Owner))
	{
		return OwnerAsPawn->IsLocallyControlled();
	}

	// The following may be inaccurate in some environments but we fallback to it
	switch (GetNetMode())
	{
	case NM_Standalone:
		// Not networked, always owned
		return true;

	case NM_Client:
		// Is Networked client in control?
		return Owner->GetLocalRole() == ROLE_AutonomousProxy;

	default:
		// Is Local authority in control?
		// AuthonomousProxy will not be set until PostInitializeComponents
		return Owner->GetLocalRole() == ROLE_Authority && Owner->GetRemoteRole() != ROLE_AutonomousProxy;
	}
}

UAbility* UAbilitiesComponent::GetEquippedAbility(UClass* Class) const
{
	if (UAbility* const * Ability = AbilityToInstance.Find(Class))
	{
		return *Ability;
	}
	return nullptr;
}

UAbility* UAbilitiesComponent::GetEquippedAbilityByName(FName AbilityName) const
{
	for(auto* Ability : AllAbilities)
	{
		check(Ability);
		if(Ability->GetRawName() == AbilityName)
		{
			return Ability;
		}
	}
	return nullptr;
}

void UAbilitiesComponent::DebugPrint()
{
	for (const auto* Ability : AllAbilities)
	{
		if (IsValid(Ability))
		{
			UE_LOG(LogAbilities, Warning, TEXT("%s"), *Ability->GetName());
		}
	}
}
