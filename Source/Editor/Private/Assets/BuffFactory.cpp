// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#include "BuffFactory.h"

#include <Kismet2/KismetEditorUtilities.h>
#include <ClassViewerModule.h>
#include <Kismet2/SClassPickerDialog.h>
#include <ClassViewerFilter.h>


#define LOCTEXT_NAMESPACE "BuffFactory"

class FAssetClassParentFilter : public IClassViewerFilter
{
public:
	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return !InClass->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags)
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};


UBuffFactory::UBuffFactory() : Super()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UBuff::StaticClass();
}

bool UBuffFactory::ConfigureProperties()
{
	BuffClass = nullptr;

	// Load the class viewer module to display a class picker
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

	FAssetClassParentFilter;
	TSharedPtr<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
	Options.ClassFilter = Filter;

	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists;
	Filter->AllowedChildrenOfClasses.Add(UBuff::StaticClass());

	const FText TitleText = LOCTEXT("CreateBuffOptions", "Pick Buff Class");
	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UBuff::StaticClass());

	if (bPressedOk)
	{
		BuffClass = ChosenClass;
	}
	return bPressedOk;
}

UObject* UBuffFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	if (BuffClass)
	{
		Class = BuffClass;
	}

	// If we have no asset class, use the passed-in class instead
	check(Class == UBuff::StaticClass() || Class->IsChildOf(UBuff::StaticClass()));
	return NewObject<UBuff>(InParent, Class, Name, Flags);
}

bool UBuffFactory::CanCreateNew() const
{
	return UBuff::StaticClass()->Children != nullptr;
}

#undef LOCTEXT_NAMESPACE
