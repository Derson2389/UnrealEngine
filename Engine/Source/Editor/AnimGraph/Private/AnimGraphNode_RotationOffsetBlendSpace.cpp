// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "AnimGraphPrivatePCH.h"
#include "AnimGraphNode_RotationOffsetBlendSpace.h"
#include "GraphEditorActions.h"
#include "CompilerResultsLog.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"

/////////////////////////////////////////////////////
// UAnimGraphNode_RotationOffsetBlendSpace

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_RotationOffsetBlendSpace::UAnimGraphNode_RotationOffsetBlendSpace(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}

FText UAnimGraphNode_RotationOffsetBlendSpace::GetTooltipText() const
{
	// FText::Format() is slow, so we utilize the cached list title
	return GetNodeTitle(ENodeTitleType::ListView);
}

FText UAnimGraphNode_RotationOffsetBlendSpace::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (Node.BlendSpace == nullptr)
	{
		if (TitleType == ENodeTitleType::ListView)
		{
			return LOCTEXT("RotationOffsetBlend_NONE_ListTitle", "AimOffset '(None)'");
		}
		else
		{
			return LOCTEXT("RotationOffsetBlend_NONE_Title", "(None)\nAimOffset");
		}
	}
	else if (!CachedNodeTitles.IsTitleCached(TitleType))
	{
		const FText BlendSpaceName = FText::FromString(Node.BlendSpace->GetName());

		FFormatNamedArguments Args;
		Args.Add(TEXT("BlendSpaceName"), BlendSpaceName);

		// FText::Format() is slow, so we cache this to save on performance
		if (TitleType == ENodeTitleType::ListView)
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("AimOffsetListTitle", "AimOffset '{BlendSpaceName}'"), Args));
		}
		else
		{
			CachedNodeTitles.SetCachedTitle(TitleType, FText::Format(LOCTEXT("AimOffsetFullTitle", "{BlendSpaceName}\nAimOffset"), Args));
		}
	}
	return CachedNodeTitles[TitleType];
}

void UAnimGraphNode_RotationOffsetBlendSpace::GetMenuEntries(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const bool bWantAimOffsets = true;
	GetBlendSpaceEntries(bWantAimOffsets, ContextMenuBuilder);
}

void UAnimGraphNode_RotationOffsetBlendSpace::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	auto PostSpawnSetupLambda = [](UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<UBlendSpaceBase> BlendSpace)
	{
		UAnimGraphNode_RotationOffsetBlendSpace* BlendSpaceNode = CastChecked<UAnimGraphNode_RotationOffsetBlendSpace>(NewNode);
		BlendSpaceNode->Node.BlendSpace = BlendSpace.Get();
	};

	for (TObjectIterator<UBlendSpaceBase> BlendSpaceIt; BlendSpaceIt; ++BlendSpaceIt)
	{
		UBlendSpaceBase* BlendSpace = *BlendSpaceIt;

		bool const bIsAimOffset = BlendSpace->IsA(UAimOffsetBlendSpace::StaticClass()) ||
			BlendSpace->IsA(UAimOffsetBlendSpace1D::StaticClass());
		if (bIsAimOffset)
		{
			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			check(NodeSpawner != nullptr);
			// @TODO: Need to file this one under the BlendSpace asset (so it can be cleared/updated when the asset is)
			ActionRegistrar.AddBlueprintAction(NodeSpawner);

			TWeakObjectPtr<UBlendSpaceBase> BlendSpacePtr = BlendSpace;
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(PostSpawnSetupLambda, BlendSpacePtr);
		}
	}
}

void UAnimGraphNode_RotationOffsetBlendSpace::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	if (Node.BlendSpace == NULL)
	{
		MessageLog.Error(TEXT("@@ references an unknown blend space"), this);
	}
	else if (Cast<UAimOffsetBlendSpace>(Node.BlendSpace) == NULL &&
			 Cast<UAimOffsetBlendSpace1D>(Node.BlendSpace) == NULL)
	{
		MessageLog.Error(TEXT("@@ references an invalid blend space (one that is not an aim offset)"), this);
	}
	else
	{
		USkeleton * BlendSpaceSkeleton = Node.BlendSpace->GetSkeleton();
		if (BlendSpaceSkeleton && // if blend space doesn't have skeleton, it might be due to blend space not loaded yet, @todo: wait with anim blueprint compilation until all assets are loaded?
			!BlendSpaceSkeleton->IsCompatible(ForSkeleton))
		{
			MessageLog.Error(TEXT("@@ references blendspace that uses different skeleton @@"), this, BlendSpaceSkeleton);
		}
	}
}

void UAnimGraphNode_RotationOffsetBlendSpace::GetContextMenuActions(const FGraphNodeContextMenuBuilder& Context) const
{
	if (!Context.bIsDebugging)
	{
		// add an option to convert to single frame
		Context.MenuBuilder->BeginSection("AnimGraphNodeBlendSpacePlayer", NSLOCTEXT("A3Nodes", "BlendSpaceHeading", "Blend Space"));
		{
			Context.MenuBuilder->AddMenuEntry(FGraphEditorCommands::Get().OpenRelatedAsset);
		}
		Context.MenuBuilder->EndSection();
	}
}

#undef LOCTEXT_NAMESPACE