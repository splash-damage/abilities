// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "AbilitiesEditor.h"

#include <AssetToolsModule.h>
#include <Kismet2/KismetEditorUtilities.h>
#include "Ability.h"

#include "Assets/AssetTypeAction_Buff.h"
#include "Assets/BuffThumbnailRenderer.h"


#define LOCTEXT_NAMESPACE "AbilitiesEditor"

IMPLEMENT_MODULE(FAbilitiesEditor, AbilitiesEditor);

void FAbilitiesEditor::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName("Gameplay"), LOCTEXT("Category", "Gameplay"));

	AssetTools.RegisterAssetTypeActions(MakeShared<FAssetTypeAction_Buff>());

	UThumbnailManager::Get().RegisterCustomRenderer(UBuff::StaticClass(), UBuffThumbnailRenderer::StaticClass());

	RegisterPropertyTypeCustomizations();

	RegisterDefaultEvent(UAbility, EventCast);
	RegisterDefaultEvent(UAbility, EventCastFinish);
	RegisterDefaultEvent(UAbility, EventActivate);
	RegisterDefaultEvent(UAbility, EventDeactivate);
	RegisterDefaultEvent(UAbility, EventTick);
}

void FAbilitiesEditor::ShutdownModule()
{
	// Unregister all pin customizations
	for (auto& FactoryPtr : CreatedPinFactories)
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(FactoryPtr);
	}
	CreatedPinFactories.Empty();

	FKismetEditorUtilities::UnregisterAutoBlueprintNodeCreation(this);
}

void FAbilitiesEditor::RegisterPropertyTypeCustomizations()
{
	//RegisterCustomPropertyTypeLayout("ClassName", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FClassNameCustomization::MakeInstance));
}

void FAbilitiesEditor::RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
{
	check(ClassName != NAME_None);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
}

void FAbilitiesEditor::RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
{
	check(PropertyTypeName != NAME_None);

	static FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
}

template<class T>
void FAbilitiesEditor::RegisterCustomPinFactory()
{
	TSharedPtr<T> PinFactory = MakeShared<T>();
	FEdGraphUtilities::RegisterVisualPinFactory(PinFactory);
	CreatedPinFactories.Add(PinFactory);
}

#undef LOCTEXT_NAMESPACE
