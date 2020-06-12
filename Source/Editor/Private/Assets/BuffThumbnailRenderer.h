// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <UObject/ObjectMacros.h>
#include <ThumbnailRendering/TextureThumbnailRenderer.h>
#include <UObject/ConstructorHelpers.h>

#include "Buff.h"
#include "BuffThumbnailRenderer.generated.h"


UCLASS(MinimalAPI)
class UBuffThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()


	UPROPERTY()
	UTexture2D* Default;


	UBuffThumbnailRenderer() : Super()
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultFinder{ TEXT("Texture2D'/Engine/EditorResources/S_VectorFieldVol.S_VectorFieldVol'") };
		Default = DefaultFinder.Object;
	}

	// Begin UThumbnailRenderer Object
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override
	{
		// Display icon as thumbnail
		UBuff* Buff = Cast<UBuff>(Object);
		UTexture2D* Icon = Buff? Buff->GetIcon() : nullptr;
		if (!Icon)
		{
			Icon = GetDefaultIcon();
		}

		Super::GetThumbnailSize(Icon, Zoom, OutWidth, OutHeight);
	}

	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas) override
	{
		// Display icon as thumbnail
		UBuff* Buff = Cast<UBuff>(Object);
		UTexture2D* Icon = Buff? Buff->GetIcon() : nullptr;
		if (!Icon)
		{
			Icon = GetDefaultIcon();
		}

		Super::Draw(Icon, X, Y, Width, Height, Target, Canvas);
	}
	// End UThumbnailRenderer Object

	UTexture2D* GetDefaultIcon() const
	{
		return Default;
	}
};
