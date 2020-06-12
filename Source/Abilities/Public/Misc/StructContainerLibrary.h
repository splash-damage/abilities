// Copyright 2020 Splash Damage, Ltd. - All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Kismet/KismetSystemLibrary.h>
#include "StructContainerLibrary.generated.h"


UCLASS()
class ABILITIES_API UStructContainerLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Abilities|Serialization")
	static void Empty(UPARAM(Ref) FStructContainer& Container)
	{
		Container.Empty();
	}

	UFUNCTION(BlueprintPure, Category = "Abilities|Serialization")
	static bool HasStruct(const FStructContainer& Container, UScriptStruct* Struct)
	{
		return Container.Has(Struct);
	}

	UFUNCTION(BlueprintCallable, Category = "Abilities|Serialization", CustomThunk, meta = (CustomStructureParam = "Struct"))
	static bool AddStruct(UPARAM(Ref) FStructContainer& Container, const FGenericStruct& Struct, bool bReplace = true);

	DECLARE_FUNCTION(execAddStruct)
	{
		P_GET_STRUCT_REF(FStructContainer, Container);

		// Get "Struct"
		Stack.Step(Stack.Object, nullptr);
		UProperty* Property = Stack.MostRecentProperty;
		void* DataPtr = Stack.MostRecentPropertyAddress;

		P_GET_UBOOL(bReplace);

		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result = Generic_AddStruct(Container, Property, DataPtr, bReplace);
		P_NATIVE_END;
	}

	UFUNCTION(BlueprintPure, Category = "Abilities|Serialization", CustomThunk, meta = (CustomStructureParam = "Struct", AutoCreateRefTerm = "Struct"))
	static bool GetStruct(UPARAM(Ref) FStructContainer& Container, FGenericStruct& Struct);

	DECLARE_FUNCTION(execGetStruct)
	{
		P_GET_STRUCT_REF(FStructContainer, Container);

		// Get "Struct"
		Stack.Step(Stack.Object, nullptr);
		UProperty* Property = Stack.MostRecentProperty;
		void* DataPtr = Stack.MostRecentPropertyAddress;

		P_FINISH;
		P_NATIVE_BEGIN;
		*(bool*)Z_Param__Result = Generic_GetStruct(Container, Property, DataPtr);
		P_NATIVE_END;
	}

	static bool Generic_AddStruct(FStructContainer& Container, UProperty* Property, void* DataPtr, bool bReplace);
	static bool Generic_GetStruct(FStructContainer& Container, UProperty* Property, void* DataPtr);
};