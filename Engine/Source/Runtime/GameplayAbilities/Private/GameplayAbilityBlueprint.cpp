// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "AbilitySystemPrivatePCH.h"
#include "GameplayAbilityBlueprint.h"
#include "GameplayAbilityBlueprintGeneratedClass.h"

//////////////////////////////////////////////////////////////////////////
// UGameplayAbilityBlueprint

UGameplayAbilityBlueprint::UGameplayAbilityBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR

UClass* UGameplayAbilityBlueprint::GetBlueprintClass() const
{
	return UGameplayAbilityBlueprintGeneratedClass::StaticClass();
}


/** Returns the most base gameplay ability blueprint for a given blueprint (if it is inherited from another ability blueprint, returning null if only native / non-ability BP classes are it's parent) */
UGameplayAbilityBlueprint* UGameplayAbilityBlueprint::FindRootGameplayAbilityBlueprint(UGameplayAbilityBlueprint* DerivedBlueprint)
{
	UGameplayAbilityBlueprint* ParentBP = NULL;

	// Determine if there is a gameplay ability blueprint in the ancestry of this class
	for (UClass* ParentClass = DerivedBlueprint->ParentClass; ParentClass != UObject::StaticClass(); ParentClass = ParentClass->GetSuperClass())
	{
		if (UGameplayAbilityBlueprint* TestBP = Cast<UGameplayAbilityBlueprint>(ParentClass->ClassGeneratedBy))
		{
			ParentBP = TestBP;
		}
	}

	return ParentBP;
}

#endif