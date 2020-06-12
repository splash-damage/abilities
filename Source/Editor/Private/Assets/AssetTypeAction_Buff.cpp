// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "AssetTypeAction_Buff.h"

#include <Engine/Blueprint.h>
#include <ThumbnailRendering/SceneThumbnailInfo.h>


#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FAssetTypeAction_Buff

FText FAssetTypeAction_Buff::GetName() const
{
	  return LOCTEXT("FAssetTypeAction_BuffName", "Buff");
}

FColor FAssetTypeAction_Buff::GetTypeColor() const
{
	// Color of the asset in RGB format
	return FColor(222, 203, 27);
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE