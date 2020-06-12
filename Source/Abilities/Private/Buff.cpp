// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "Buff.h"
#include "AbilitiesComponent.h"

#if WITH_EDITOR
bool UBuff::CanEditChange(const UProperty* InProperty) const
{
	bool bCanEdit = Super::CanEditChange(InProperty);

	FName PropertyName = InProperty ? InProperty->GetFName() : NAME_None;
	if (GET_MEMBER_NAME_CHECKED(UBuff, LifetimeDuration) == PropertyName)
	{
		bCanEdit &= bHasLifetime && !bStackable;
	}
	return bCanEdit;
}
#endif //WITH_EDITOR

void UBuff::DoApplyEffects(UAbilitiesComponent* Component, int32 Count) const
{
	Component->AddTags(TagsToApply);
	Component->AddTags(ApplyTagsOnApply);
	Component->RemoveTags(RemoveTagsOnApply);

	EventApplyEffects(Component, Count);
}

void UBuff::DoRevertEffects(UAbilitiesComponent* Component, int32 Count) const
{
	EventRevertEffects(Component, Count);

	Component->RemoveTags(TagsToApply);
	Component->AddTags(ApplyTagsOnRevert);
	Component->RemoveTags(RemoveTagsOnRevert);
}

void UBuff::EventApplyEffects_Implementation(UAbilitiesComponent* Component, int32 Count) const
{
	ApplyEffects(Component, Count);
}

void UBuff::EventRevertEffects_Implementation(UAbilitiesComponent* Component, int32 Count) const
{
	RevertEffects(Component, Count);
}
