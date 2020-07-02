// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include <Components/ActorComponent.h>
#include <GameplayTagContainer.h>

#include "AbilitiesModule.h"
#include "Ability.h"
#include "Buff.h"
#include "AbilitiesCooldownCounter.h"
#include "BuffsLifetimeCounter.h"
#include "BuffTypeContainer.h"
#include "Misc/Helpers.h"
#include "AbilitiesComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuffsAppliedDelegate, const TSet<FBuffCount>&, Buffs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuffsRemovedDelegate, const TSet<FBuffCount>&, Buffs);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTagsChangedDelegate);


class UAbilitiesComponent;

// Defines how buffs replicate at compile-time
// See UAbilitiesComponent::BuffReplication
enum class EBuffReplicationMode : uint8
{
	OnlyServer,
	// To check for buffs on client ability activation, they should replicate.
	// But, you can still use tags instead.
	OwningClient,
	AllClients
};

UENUM(BlueprintType)
enum class EBuffOperation : uint8
{
	Added,
	Removed
};

USTRUCT(BlueprintType)
struct FCancelInputReturn
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool bInputCancelled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool bAbilityCancelled = false;
};


/** Component that owns abilities and ability effects */
UCLASS(Blueprintable, ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class ABILITIES_API UAbilitiesComponent : public UActorComponent
{
	GENERATED_BODY()

	friend UAbility;


	/************************************************************************/
	/* PROPERTIES                                                           */
	/************************************************************************/

	// Compile-time settings
	// What model of buff replication to use. Should it not replicate? Only to owning client or all clients?
	static constexpr EBuffReplicationMode BuffReplication = EBuffReplicationMode::AllClients;


protected:

	/** Tags that the system must have to able to execute an ability. Can only be edited from server. */
	UPROPERTY(ReplicatedUsing="OnRep_Tags", EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	FGameplayTagContainer Tags;

	UFUNCTION()
	void OnRep_Tags();


	/** Begin ABILITIES */

	/** The abilities that the character will have at first activate */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta=(DisplayName = "Abilities"))
	TSet<TSubclassOf<UAbility>> InitialAbilities;

	/** List of abilities that can be used by the player. This list can change in runtime. */
	UPROPERTY()
	TSet<TSubclassOf<UAbility>> EquippedAbilities;

	UPROPERTY(ReplicatedUsing = OnRep_AllAbilities)
	TArray<UAbility*> AllAbilities;

	UPROPERTY()
	TMap<UClass*, UAbility*> AbilityToInstance;

	/** Cached list of abilities that will tick */
	UPROPERTY(Transient)
	TSet<UAbility*> TickingAbilities;

	UPROPERTY()
	FAbilitiesCooldownCounter Cooldowns;
	/** End ABILITIES */


	/** Begin BUFFS */

	// Map containing all buffs and their amounts (if stackable)
	UPROPERTY()
	TSet<FBuffCount> Buffs;

	UPROPERTY()
	TSet<FBuffTypeContainer> BuffsByClass;

	// Buffs applied at initialize for the first time
	UPROPERTY(EditAnywhere, Category = "Buffs", meta=(DisplayName = "Buffs"))
	TSet<FBuffCount> InitialBuffs;

	UPROPERTY()
	FBuffsLifetimeCounter BuffLifetimes;
	/** End BUFFS */


	UPROPERTY(Transient)
	TMap<FName, UClass*> PressedInputs;

private:

	UPROPERTY(Transient)
	bool bIsTearingDown = false;

public:

	/** Begin EVENTS */

	UPROPERTY(BlueprintAssignable, Category = Buffs)
	FOnBuffsAppliedDelegate OnBuffsApplied;

	UPROPERTY(BlueprintAssignable, Category = Buffs)
	FOnBuffsRemovedDelegate OnBuffsRemoved;

	UPROPERTY(BlueprintAssignable, Category = Buffs)
	FOnTagsChangedDelegate OnTagsChanged;
	/** End EVENTS */


	/************************************************************************/
	/* METHODS                                                              */
	/************************************************************************/

	UAbilitiesComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

protected:

	virtual void OnRegister() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	/** BEGIN ABILITIES */
public:

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Abilities")
	void EquipAbility(TSubclassOf<UAbility> Class);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Abilities")
	void EquipAbilities(TSet<TSubclassOf<UAbility>> Classes);

	void EquipAbilities(TArray<TSubclassOf<UAbility>> Classes);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Abilities")
	void UnequipAbility(TSubclassOf<UAbility> Class);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Abilities")
	void UnequipAbilities();

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	bool IsEquipped(TSubclassOf<UAbility> AbilityClass) const;

	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Abilities")
	bool CastAbility(TSubclassOf<UAbility> Class);


	/**
	 * Called when an input has been pressed and an ability must react to it.
	 * @param Class of the ability to affect
	 * @param InputEvent is the name of the input used. The ability will only
	 * accept release from this event and will release if another ability is pressed using it.
	 */
	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Abilities")
	void PressInput(TSubclassOf<UAbility> Class, FName InputEvent = "Default");

	// Release any ability using this input event
	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Abilities")
	bool ReleaseInput(FName InputEvent = "Default");

	// Release the ability with type Class
	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Abilities")
	bool ReleaseInputByClass(TSubclassOf<UAbility> Class);

	// Removes an input event and its ability is cancelled if running
	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Abilities")
	FCancelInputReturn CancelInput(FName InputEvent = "Default");

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	bool IsInputPressed(FName InputEvent = "Default") const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	bool IsAnyInputPressed() const { return PressedInputs.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	TSubclassOf<UAbility> GetPressedAbilityFromInput(FName InputEvent = "Default") const;


	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Abilities")
	bool Cancel(TSubclassOf<UAbility> Class);

	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Abilities")
	void CancelAll();

	/** Checks if an ability can activate without trying to do it. */
	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	bool CanCast(TSubclassOf<UAbility> Class, FStructContainer Container);
	bool CanCast(TSubclassOf<UAbility> Class)
	{
		return CanCast(Class, {});
	}

	/** Checks if an ability can activate without trying to do it. */
	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	bool CanActivate(TSubclassOf<UAbility> Class, FStructContainer Container);
	bool CanActivate(TSubclassOf<UAbility> Class)
	{
		return CanActivate(Class, {});
	}

	// @return true if any ability of a class is executing
	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	bool IsRunning(TSubclassOf<UAbility> Class) const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	bool IsCoolingDown(TSubclassOf<UAbility> Class) const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	float GetRemainingCooldown(TSubclassOf<UAbility> Class) const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	float GetCooldownDuration(TSubclassOf<UAbility> Class) const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Abilities")
	float GetBuffRemainingLifetime(UBuff* Buff) const;

	UFUNCTION()
	void OnRep_AllAbilities();

private:

	void InternalEquipAbility(UClass* Class);


public:

	/** Begin TAGS */

	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Tags")
	void AddTag(const FGameplayTag& NewTag);

	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Tags")
	void AddTags(const FGameplayTagContainer& NewTags);

	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Tags")
	bool RemoveTag(const FGameplayTag& NewTag);

	UFUNCTION(BlueprintCallable, Category = "AbilityComponent|Tags")
	void RemoveTags(const FGameplayTagContainer& NewTags);

	const FGameplayTagContainer& GetTags() const { return Tags; }

protected:

	void NotifyTagsChanged();
	/** End TAGS */


	/** Begin BUFFS */
public:

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	FORCEINLINE bool ApplyBuff(FBuffCount Buff)
	{
		return ApplyBuffs({ Buff });
	}

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	bool ApplyBuffs(const TSet<FBuffCount>& InBuffs);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	bool ApplySingleBuffs(const TSet<UBuff*>& InBuffs);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	FORCEINLINE bool RemoveBuff(FBuffCount Buff)
	{
		return RemoveBuffs({ Buff });
	}

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	bool ResetBuffs();

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	bool RemoveBuffs(const TSet<FBuffCount>& InBuffs);

	// Removes a list of buffs, no matter their count
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	bool RemoveAllBuffs(const TSet<UBuff*>& InBuffs);

	// Remove all buffs with "Tag" inside Identifying tags
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "AbilityComponent|Buffs")
	bool RemoveBuffsByTag(FGameplayTag Tag, bool bExact = false);

	// Checks if the component currently has an specific buff
	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Buffs")
	bool HasBuff(UBuff* Buff) const
	{
		return Buffs.Contains(Buff);
	}

	// Checks if the component currently has a list of buffs
	// @return true if one or all buffs are contained
	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Buffs")
	bool HasBuffs(const TSet<UBuff*>& InBuffs, EAllAny Mode = EAllAny::Any) const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Buffs")
	int32 GetBuffCount(const UBuff* Buff) const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Buffs")
	bool HasBuffOfClass(TSubclassOf<UBuff> Class) const
	{
		return BuffsByClass.Contains({ Class.Get() });
	}

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Buffs")
	void GetBuffsOfClass(TSubclassOf<UBuff> Class, TSet<FBuffCount>& OutBuffs) const;

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Buffs")
	const TSet<FBuffCount>& GetAllBuffs() const
	{
		return Buffs;
	}

	UFUNCTION(BlueprintPure, Category = "AbilityComponent|Buffs")
	int32 GetNumBuffs() const
	{
		return Buffs.Num();
	}

protected:

	virtual void LocalOnBuffsChanged(const TSet<FBuffCount>& ModifiedBuffs, EBuffOperation Change);

private:

	void NotifyBuffsChanged(const TSet<FBuffCount>& ModifiedBuffs, EBuffOperation Change);

	bool InternalApplyBuffs(const TSet<FBuffCount>& InBuffs, TSet<FBuffCount>& AppliedBuffs);
	bool InternalRemoveBuffs(const TSet<FBuffCount>& InBuffs, TSet<FBuffCount>& RemovedBuffs);

	UFUNCTION(Client, Reliable)
	void ClientOnBuffsChanged(const TArray<FBuffCount>& ModifiedBuffs, EBuffOperation Change);
	UFUNCTION(NetMulticast, Reliable)
	void MCOnBuffsChanged(const TArray<FBuffCount>& ModifiedBuffs, EBuffOperation Change);
	/**End BUFFS */


	/** HELPERS */
public:

	UFUNCTION(BlueprintPure, Category = AbilityComponent)
	bool HasAuthority() const;

	UFUNCTION(BlueprintPure, Category = AbilityComponent)
	bool IsLocallyOwned() const;

	UFUNCTION(BlueprintPure, Category = AbilityComponent)
	UAbility* GetEquippedAbility(TSubclassOf<UAbility> Class) const;

	UAbility* GetEquippedAbility(UClass* Class) const;

	UFUNCTION(BlueprintPure, Category = AbilityComponent)
	UAbility* GetEquippedAbilityByName(FName AbilityName) const;

	UFUNCTION(BlueprintPure, Category = AbilityComponent)
	TArray<TSubclassOf<UAbility>> GetEquippedAbilities() const;

	FAbilitiesCooldownCounter& GetCooldowns() { return Cooldowns; }
	const FAbilitiesCooldownCounter& GetCooldowns() const { return Cooldowns; }

	UFUNCTION(BlueprintPure, Category = AbilityComponent)
	bool IsTearingDown() const { return bIsTearingDown; }

	UFUNCTION(CallInEditor, Category = AbilityComponent)
	void DebugPrint();


	/** Template API helpers */

	template<typename T>
	void EquipAbility()
	{
		static_assert(TIsDerivedFrom<T, UAbility>::IsDerived, "Must provide an ability class");
		EquipAbility(T::StaticClass());
	}

	template<typename T>
	void UnequipAbility()
	{
		static_assert(TIsDerivedFrom<T, UAbility>::IsDerived, "Must provide an ability class");
		UnequipAbility(T::StaticClass());
	}

	template<typename T>
	bool CastAbility()
	{
		static_assert(TIsDerivedFrom<T, UAbility>::IsDerived, "Must provide an ability class");
		return CastAbility(T::StaticClass());
	}

	template<typename T>
	bool Cancel()
	{
		static_assert(TIsDerivedFrom<T, UAbility>::IsDerived, "Must provide an ability class");
		return Cancel(T::StaticClass());
	}

	template<typename T>
	FORCEINLINE bool IsEquipped() const
	{
		static_assert(TIsDerivedFrom<T, UAbility>::IsDerived, "Must provide an ability class");
		return IsEquipped(T::StaticClass());
	}

	template<typename T>
	FORCEINLINE T* GetEquippedAbility() const
	{
		static_assert(TIsDerivedFrom<T, UAbility>::IsDerived, "Must provide an ability class");
		return static_cast<T*>(GetEquippedAbility(T::StaticClass()));
	}
};

inline bool UAbilitiesComponent::HasAuthority() const
{
	if (!ensureMsgf(GetOwner(), TEXT("Owner must always be valid when calling HasAuthority.")))
	{
		return false;
	}
	return GetOwner()->HasAuthority();
}

inline int32 UAbilitiesComponent::GetBuffCount(const UBuff* Buff) const
{
	if (const auto* BuffCount = Buffs.Find(Buff))
	{
		return BuffCount->Count;
	}
	return 0;
}

inline UAbility* UAbilitiesComponent::GetEquippedAbility(TSubclassOf<UAbility> Class) const
{
	return GetEquippedAbility(Class.Get());
}

inline TArray<TSubclassOf<UAbility>> UAbilitiesComponent::GetEquippedAbilities() const
{
	return EquippedAbilities.Array();
}

inline bool UAbilitiesComponent::IsInputPressed(FName InputEvent) const
{
	return PressedInputs.Contains(InputEvent);
}

inline TSubclassOf<UAbility> UAbilitiesComponent::GetPressedAbilityFromInput(FName InputEvent) const
{
	if (auto* const * Input = PressedInputs.Find(InputEvent))
	{
		return { *Input };
	}
	return {};
}
