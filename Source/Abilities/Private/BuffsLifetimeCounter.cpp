
// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "BuffsLifetimeCounter.h"

#include <Engine/World.h>

#include "Buff.h"
#include "AbilitiesComponent.h"


void FBuffsLifetimeCounter::Start(const TSet<FBuffCount>& Buffs)
{
	auto* Component = GetOwner<UAbilitiesComponent>();
	if (!ensureMsgf(Component, TEXT("Component must be valid when starting Buff Lifetimes")))
	{
		return;
	}

	const float GameTime = GetWorld()->GetTimeSeconds();

	LifetimePerBuff.Reserve(LifetimePerBuff.Num() + Buffs.Num());
	for (const FBuffCount& BuffCount : Buffs)
	{
		const float Duration = BuffCount.Buff->GetLifetimeDuration();
		if (Duration > 0.f)
		{
			float& Lifetime = LifetimePerBuff.FindOrAdd(BuffCount.Buff);
			Lifetime = GameTime + Duration;
		}
	}
}

void FBuffsLifetimeCounter::Reset(const TSet<FBuffCount>& Buffs)
{
	for (const FBuffCount& BuffCount : Buffs)
	{
		LifetimePerBuff.Remove(BuffCount.Buff);
	}
	LifetimePerBuff.Shrink();
}

void FBuffsLifetimeCounter::ResetAll()
{
	LifetimePerBuff.Empty();
}


float FBuffsLifetimeCounter::GetRemaining(const UBuff* Buff) const
{
	if (const float* FinalTime = LifetimePerBuff.Find(Buff))
	{
		const float GameTime = GetWorld()->GetTimeSeconds();
		return FMath::Max(*FinalTime - GameTime, 0.f);
	}
	return 0.f;
}

void FBuffsLifetimeCounter::Tick()
{
	auto* Component = GetOwner<UAbilitiesComponent>();
	if (!ensureMsgf(Component, TEXT("Component must be valid when ticking Buff Lifetimes")))
	{
		return;
	}

	const float GameTime = GetWorld()->GetTimeSeconds();

	TSet<UBuff*> BuffsToRemove;
	for (const auto& Lifetime : LifetimePerBuff)
	{
		// Check if time of removal has arrived
		if (Lifetime.Value <= GameTime)
		{
			BuffsToRemove.Add(Lifetime.Key);
		}
	}

	if (BuffsToRemove.Num() > 0)
	{
		Component->RemoveAllBuffs(BuffsToRemove);
	}
}
