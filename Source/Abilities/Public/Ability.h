// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Engine/Texture2D.h>
#include <VisualLogger/VisualLogger.h>

#include "AbilitiesModule.h"
#include "AbilityTypes.h"
#include "AbilityBase.h"
#include "Ability.generated.h"


class UAbilitiesComponent;
struct FAbilitiesCooldownCounter;


#define ABILITY_VLOG_LOCATION(Radius, Color, Format, ...) \
	UE_VLOG_LOCATION(this, LogAbilities, Log, GetOwner()->GetActorLocation(), Radius, Color, Format, __VA_ARGS__);


UENUM(Blueprintable)
enum class EAbilityTickMode : uint8
{
	Never,
	DuringCastOnly, // Tick only on Casting state
	DuringActivationOnly, // Tick only on Activation state
	DuringCastAndActivation, // Tick on Cast & Activation state. Recommended option if ticking is needed
	Always, // Ticks when the ability is equipped (and therefore Execution as well)
};

UENUM(Blueprintable)
enum class ECooldownReadyReason : uint8
{
	Finished,
	Resseted
};

UENUM()
enum class EAbilityCooldownMode : uint8
{
	OnActivation, // Starts cooldown just after activation
	OnDeactivation, // Starts cooldown just after deactivation
	Manual // Cooldown will start at anytime
};

UENUM()
enum class EAbilityInputProfile : uint8
{
	CastWhileHolding, // Will cast the ability when pressed, and activate it when released. If there's no cast, this be equal to "ActivateOnPress"
	ActivateOnPress, // Will activate the ability when pressed
	ActivateWhileHolding, // Will activate the ability when pressed, and deactivate it when released
	ToggleActivationOnPress, // Activate when clicked, Deactivate when clicked again
	None
};


/** An ability is an object that contains all logic for game abilities.
 *
 * There are two main execution steps on an ability's lifetime:
 * - Casting: An optional step representing casting time, loading or aiming
 * - Activation: The activation of the ability itself
 *
 * NOTE: Local functions are those that will execute on server and clients.
 * This is to simplify prediction code.
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class ABILITIES_API UAbility : public UAbilityBase
{
	GENERATED_BODY()

	friend FAbilitiesCooldownCounter;
	friend UAbilitiesComponent;


	/************************************************************************/
	/* PROPERTIES                                                           */
	/************************************************************************/
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Ability)
	FName Name;

#if WITH_EDITORONLY_DATA
	// Designer text for this ability that wont be packaged into the game
	UPROPERTY(EditDefaultsOnly, Category = Ability, meta = (MultiLine=true))
	FText DesignerNotes;
#endif

	UPROPERTY(EditDefaultsOnly, Category = "Ability|UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI", meta = (MultiLine=true))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|UI")
	UTexture2D* Icon;

	// If true, this ability will enter cast mode before it can be activated
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Execution")
	bool bHasCast = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,  Category = "Ability|Execution")
	EAbilityTickMode TickMode = EAbilityTickMode::Never;

	// Tags that the system must have to able to execute the ability
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution")
	FGameplayTagContainer RequiredTags;

	// Tags that the system must not have to able to execute the ability
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution")
	FGameplayTagContainer RequiredToNotHaveTags;

	// Tags added when the ability starts casting
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Cast")
	FGameplayTagContainer CastStartAddTags;

	// Tags removed when the ability starts casting
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Cast")
	FGameplayTagContainer CastStartRemoveTags;

	// Tags added when the ability finishes casting
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Cast")
	FGameplayTagContainer CastFinishAddTags;

	// Tags removed when the ability finishes casting
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Cast")
	FGameplayTagContainer CastFinishRemoveTags;

	// Tags added when the ability activates
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Activation")
	FGameplayTagContainer ActivationAddTags;

	// Tags removed when the ability activates
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Activation")
	FGameplayTagContainer ActivationRemoveTags;

	// Tags added when the ability deactivates
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Activation")
	FGameplayTagContainer DeactivationAddTags;

	// Tags removed when the ability deactivates
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Activation")
	FGameplayTagContainer DeactivationRemoveTags;

	// If true, casting is cancelled when InterruptWithTags hits
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Execution|Interrupt")
	bool bInterruptionCancelsCasting = false;

	// If true, activation is cancelled when InterruptWithTags hits
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability|Execution|Interrupt")
	bool bInterruptionCancelsActivation = true;

	// If any of this tags is received this ability will cancel activation
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Execution|Interrupt")
	FGameplayTagContainer InterruptWithTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Execution", meta = (InlineEditConditionToggle))
	bool bHasCooldown = false;

	// Cooldown duration in seconds. If 0 or less, there's no cooldown.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Execution", meta = (ClampMin = 0, EditCondition = bHasCooldown, ForceUnits = s))
	float CooldownDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Execution", meta = (EditCondition = bHasCooldown))
	EAbilityCooldownMode CooldownMode = EAbilityCooldownMode::OnActivation;

	// If the ability has cast, when activation prediction fails, cast will be resumed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Network")
	bool bBackToCastingIfPredictionFailed = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Input")
	EAbilityInputProfile InputProfile = EAbilityInputProfile::CastWhileHolding;

	/** If true, activation or cast will start when cooldown finishes if input is pressed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability|Input")
	bool bInputWaitForCooldown = false;


	/** Runtime Properties */

	UPROPERTY(Transient)
	FName PressedEvent;


	/************************************************************************/
	/* METHODS                                                              */
	/************************************************************************/
protected:

	virtual void BeginPlay() override;

	virtual bool CheckTransition(FAbilityStateTransition Transition, const FStructContainer& Container) override;
	virtual void OnStateChanged(FAbilityStateTransition Transition, const FStructContainer& Container) override;
	virtual void EndPlay() override;


	/** BEGIN Cast */
public:

	// Start casting. If no cast, it will activate
	// @network: Owning Client or Server
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool StartCast(FStructContainer Container)
	{
		return SetState(bHasCast? EAbilityState::Cast : EAbilityState::Activation, Container);
	}
	bool StartCast() { return StartCast({}); }

	// Finishes cast and continues with activation ONLY if the ability was casting.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability")
	void ActivateOrCancel(FStructContainer Container);
	void ActivateOrCancel() { ActivateOrCancel({}); }

protected:

	virtual bool CanCast(const FStructContainer& Container) const { return true; }
	virtual void OnCast(const FStructContainer& Container);
	virtual void OnCastFinish() {}

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "Can Cast"))
	bool EventCanCast(const FStructContainer& Container) const;

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "Cast"))
	void EventCast(const FStructContainer& Container);

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "Cast Finish"))
	void EventCastFinish();
	/** END Cast */


	/** BEGIN Activation */
public:

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability")
	bool Activate(FStructContainer Container);
	bool Activate() { return Activate({}); }

	// Finishes an activated ability. Doesn't affect casting in any way.
	// @network: Owning Client or Server
	UFUNCTION(BlueprintCallable, Category = "Ability")
	void Deactivate();

protected:

	virtual bool CanActivate(const FStructContainer& Container) const
	{
		return !bHasCast || IsCasting();
	}
	virtual void OnActivation(const FStructContainer& Container);
	virtual void OnDeactivation();

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "Can Activate"))
	bool EventCanActivate(const FStructContainer& Container) const;

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "Activate"))
	void EventActivate(const FStructContainer& Container);

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "Deactivate"))
	void EventDeactivate();
	/** END Activation */

	/** BEGIN Cooldowns */
public:

	// Used to manually reset a cooldown from server
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Ability)
	void ResetCooldown();

protected:

	// Used to manually start a cooldown from server
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Ability)
	void StartCooldown();

	virtual void OnCooldownStarted() {}
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, meta = (DisplayName = "On Cooldown Started"))
	void EventOnCooldownStarted();

	virtual void OnCooldownReady(ECooldownReadyReason Reason) {}
	UFUNCTION(BlueprintImplementableEvent, Category = Ability, meta = (DisplayName = "On Cooldown Ready"))
	void EventOnCooldownReady(ECooldownReadyReason Reason);

private:

	// Called from StartCooldown after checks have been done
	UFUNCTION(NetMulticast, Reliable)
	void MCStartCooldown();

	// Called from ResetCooldown after checks have been done
	UFUNCTION(NetMulticast, Reliable)
	void MCResetCooldown();

	void LocalStartCooldown();
	void LocalResetCooldown();

	void NotifyCooldownReady(ECooldownReadyReason Reason);
	/** END Cooldowns */


	/** BEGIN Tags */
public:

	virtual void OnTagsChanged(const FGameplayTagContainer& Tags);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName="On Tags Changed"))
	void EventOnTagsChanged(const FGameplayTagContainer& Tags);
	/** END Tags */


	/** Cancels casting or activation
	 * @param bApplyCooldown if false, will ignore cooldown activation
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability", meta = (Keywords = "Deactivate Cast"))
	void Cancel(bool bApplyCooldown = true);

	UFUNCTION(BlueprintPure, Category = "Ability|UI")
	FName GetRawName() const
	{
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = "Ability|UI")
	FText GetDisplayName() const
	{
		return DisplayName.IsEmpty() ? FText::FromName(Name) : DisplayName;
	}

	const FName& GetIDName() const
	{
		return Name;
	}

	UFUNCTION(BlueprintPure, Category = Ability)
	bool IsRunning() const;

	UFUNCTION(BlueprintPure, Category = Ability)
	bool IsCasting() const
	{
		return HasBegunPlay() && GetState() == EAbilityState::Cast;
	}

	UFUNCTION(BlueprintPure, Category = Ability)
	bool IsActivated() const
	{
		return HasBegunPlay() && GetState() == EAbilityState::Activation;
	}

	UFUNCTION(BlueprintPure, Category = Ability)
	bool IsCancelled() const
	{
		return HasBegunPlay() && GetState() == EAbilityState::Cancelled;
	}

	UFUNCTION(BlueprintPure, Category = Ability)
	bool HasSucceeded() const
	{
		return HasBegunPlay() && GetState() == EAbilityState::Succeeded;
	}

	UFUNCTION(BlueprintPure, Category = Ability)
	bool HasCooldown() const { return bHasCooldown && CooldownDuration > 0.f; }

	UFUNCTION(BlueprintPure, Category = Ability)
	bool IsCoolingDown() const;

	// True when casting and activation have finished
	UFUNCTION(BlueprintPure, Category = Ability)
	bool HasFinished() const;


	/** BEGIN Input */
protected:

	void PressInput(FName Event, FName& PreviousEvent);
	void ReleaseInput();
	bool CancelInput();

	virtual void OnInputPressed();
	virtual void OnInputReleased();
	virtual bool OnCancelInput();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, meta = (DisplayName = "On Input Pressed"))
	void EventOnInputPressed();

	UFUNCTION(BlueprintImplementableEvent, Category = Ability, meta = (DisplayName = "On Input Released"))
	void EventOnInputReleased();

	UFUNCTION(BlueprintNativeEvent, Category = Ability, meta = (DisplayName = "On Cancel Input"))
	bool EventOnCancelInput();

public:

	UFUNCTION(BlueprintPure, Category = "Ability|Input")
	EAbilityInputProfile GetInputProfile() const { return InputProfile; }

	UFUNCTION(BlueprintPure, Category = "Ability|Input")
	FORCEINLINE bool IsPressed() const { return !PressedEvent.IsNone(); }
	/** END Input*/


	/** BEGIN UObject */
	virtual UWorld* GetWorld() const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	// Support for RPCs
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack) override;

	virtual bool ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags) { return false; }

	virtual void PreDestroyFromReplication() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif //WITH_EDITOR
	/** END UObject */
};


inline void UAbility::EndPlay()
{
	if(HasAuthority() && IsRunning())
	{
		Cancel(false);
	}
	Super::EndPlay();
}

inline bool UAbility::Activate(FStructContainer Container)
{
	return SetState(EAbilityState::Activation, Container);
}

inline void UAbility::Deactivate()
{
	if (IsActivated())
	{
		Cancel(true);
	}
}

inline void UAbility::OnActivation(const FStructContainer& Container)
{
	ABILITY_VLOG_LOCATION(25.f, FColor::Green, TEXT("'%s' activated"), *GetClass()->GetName());
}

inline void UAbility::OnDeactivation()
{
	ABILITY_VLOG_LOCATION(25.f, FColor::Red, TEXT("'%s' deactivated"), *GetClass()->GetName());
}

inline void UAbility::StartCooldown()
{
	if(HasAuthority() && !IsCoolingDown())
	{
		MCStartCooldown();
	}
}

inline void UAbility::ResetCooldown()
{
	if(HasAuthority() && IsCoolingDown())
	{
		MCResetCooldown();
	}
}

inline bool UAbility::IsRunning() const
{
	return HasBegunPlay() &&
		(GetState() == EAbilityState::Cast ||
		 GetState() == EAbilityState::Activation);
}

inline bool UAbility::HasFinished() const
{
	return GetState() == EAbilityState::Succeeded
		|| GetState() == EAbilityState::Cancelled;
}