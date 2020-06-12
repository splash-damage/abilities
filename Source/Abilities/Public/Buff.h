// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <GameplayTagContainer.h>
#include <Engine/Texture2D.h>

#include "Buff.generated.h"


class UAbilitiesComponent;
class UBuff;


USTRUCT(BlueprintType)
struct FBuffCount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Buff)
	UBuff* Buff = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Buff)
	int32 Count = 1;


	FBuffCount() {}
	FBuffCount(UBuff* Buff, int32 Count = 1) : Buff(Buff), Count(Count) {}

	operator bool() const { return Buff != nullptr; }
	bool operator==(UBuff* Other) const { return Buff == Other; }
	bool operator==(const FBuffCount& Other) const { return Buff == Other.Buff; }

	friend int64 GetTypeHash(const FBuffCount& Item)
	{
		return GetTypeHash(Item.Buff);
	}
};


UCLASS(BlueprintType, Blueprintable, Abstract)
class ABILITIES_API UBuff : public UDataAsset
{
	GENERATED_BODY()

	/************************************************************************/
	/* PROPERTIES                                                           */
	/************************************************************************/
protected:

	UPROPERTY(EditAnywhere, Category = Info)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Info, meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Info)
	UTexture2D* Icon;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = Info, meta = (MultiLine = true))
	FText DesignerNotes;
#endif

	// Tags added when applied, removed when reverted
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Effects)
	FGameplayTagContainer TagsToApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = Effects)
	FGameplayTagContainer ApplyTagsOnApply;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = Effects)
	FGameplayTagContainer RemoveTagsOnApply;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = Effects)
	FGameplayTagContainer ApplyTagsOnRevert;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = Effects)
	FGameplayTagContainer RemoveTagsOnRevert;


	/** If true, more than one of the same buff can be applied
	 * When a second stackable buff is applied, the previous amount is reverted, then the new count (previous + 1) is applied
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Buff)
	bool bStackable = false;

	// If true, only one buff of this type will be applied at the same time
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Buff)
	bool bUnique = false;

	// If true, when unique, this buff will have priority and replace any other buff of the same type.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Buff, meta=(EditCondition="bUnique"))
	bool bReplacePreviousUnique = false;

	// If true, the buff will be removed after "LifetimeDuration" seconds.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Buff, meta = (InlineEditConditionToggle))
	bool bHasLifetime = false;

	// Seconds until the buff is removed. Only if enabled and more than 0.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Buff, meta = (ClampMin = 0, EditCondition = "bHasLifetime", ForceUnits = s))
	float LifetimeDuration = 1.f;

	// Tags used to identify this buff. They wont be applied or reverted.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Buff)
	FGameplayTagContainer Tags;


	/************************************************************************/
	/* METHODS                                                              */
	/************************************************************************/
public:

#if WITH_EDITOR
	virtual bool CanEditChange(const UProperty* InProperty) const override;
#endif

	// Called from the asset object. Buffs don't get instanced in runtime.
	void DoApplyEffects(UAbilitiesComponent* Component, int32 Count) const;
	void DoRevertEffects(UAbilitiesComponent* Component, int32 Count) const;

protected:

	virtual void ApplyEffects(UAbilitiesComponent* Component, int32 Count) const {}
	virtual void RevertEffects(UAbilitiesComponent* Component, int32 Count) const {}

	/**
	 * Called when a buff is applied.
	 * @param Component that has the buff
	 * @param Count of buffs to apply (if stackable)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = Buff, meta = (DisplayName = "Apply Effects"))
	void EventApplyEffects(UAbilitiesComponent* Component, int32 Count) const;

	/**
	 * Called when a buff is reverted.
	 * @param Component that has the buff
	 * @param Count of buffs to revert (if stackable)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = Buff, meta = (DisplayName = "Revert Effects"))
	void EventRevertEffects(UAbilitiesComponent* Component, int32 Count) const;

public:

	UFUNCTION(BlueprintPure, Category = Buff)
	FText GetDisplayName() const;
	UTexture2D* GetIcon() const { return Icon; }

	UFUNCTION(BlueprintPure, Category = Buff)
	float GetLifetimeDuration() const
	{
		return (bHasLifetime && !bStackable)? FMath::Max(LifetimeDuration, 0.f) : 0.f;
	}

	bool IsUnique() const { return bUnique; }
	bool ReplacesPreviousUnique() const { return bReplacePreviousUnique; }

	bool IsStackable() const { return bStackable; }

	const FGameplayTagContainer& GetTags() const { return Tags; }

};

inline FText UBuff::GetDisplayName() const
{
	if(DisplayName.IsEmpty())
	{
		return FText::FromString(GetName());
	}
	return DisplayName;
}
