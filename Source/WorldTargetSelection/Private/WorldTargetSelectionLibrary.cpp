// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#include "WorldTargetSelectionLibrary.h"

#include "WorldTargetComponent.h"
#include "WorldTargetSelectionComponent.h"
#include "WorldTargetSelectionSubsystem.h"

UWorldTargetSelectionSubsystem* UWorldTargetSelectionLibrary::GetSubsystem(const UObject* WorldContext)
{
	if (!::IsValid(WorldContext))
	{
		return nullptr;
	}
	UWorld* World = WorldContext->GetWorld();
	if (!::IsValid(World))
	{
		return nullptr;
	}
	return World->GetSubsystem<UWorldTargetSelectionSubsystem>();
}

bool UWorldTargetSelectionLibrary::IsConditionPass(const FWorldTargetSelectionConditionContainer& ConditionContainer, UWorldTargetSelectionComponentBase* InSelector, UWorldTargetComponentBase* InTarget)
{
	if (!::IsValid(InSelector) || !::IsValid(InTarget))
	{
		return false;
	}
	const uint8 ConditionContainerType = StaticCast<uint8>(ConditionContainer.Type);
	const bool TrueIsOrFalseIsAnd = (ConditionContainerType & 0x01) != 0;
	const bool TrueIsNot = (ConditionContainerType & 0x10) != 0;
	bool Result = TrueIsOrFalseIsAnd ? false : true;
	for (UWorldTargetSelectionConditionBase* ConditionObject : ConditionContainer.InnerObjects)
	{
		if (!::IsValid(ConditionObject))
		{
			continue;
		}
		const bool OneResult = ConditionObject->IsConditionPass(InSelector, InTarget);
		if (TrueIsOrFalseIsAnd && OneResult)
		{
			Result = true;
			break;
		}
		if (!TrueIsOrFalseIsAnd && !OneResult)
		{
			Result = false;
			break;
		}
	}
	Result = TrueIsNot ? !Result : Result;
	return Result;
}

struct FInnerSortFunction
{
	FInnerSortFunction(UWorldTargetSelectionComponentBase* InSelector, UWorldTargetSelectionSortProxyBase* InSortProxy) : Selector(InSelector), SortProxy(InSortProxy) {}
	UWorldTargetSelectionComponentBase* Selector;
	UWorldTargetSelectionSortProxyBase* SortProxy;
	bool operator()(UWorldTargetComponentBase& Left, UWorldTargetComponentBase& Right) const
	{
		if (!ensureAlways(Selector != nullptr && SortProxy != nullptr))
		{
			return false;
		}
		return SortProxy->SortFunction(Selector, &Left, &Right);
	}
};

void UWorldTargetSelectionLibrary::DoSort(UWorldTargetSelectionSortProxyBase* SortProxy, UWorldTargetSelectionComponentBase* InSelector, TArray<UWorldTargetComponentBase*>& InTargets)
{
	if (!::IsValid(SortProxy) || !::IsValid(InSelector) || InTargets.IsEmpty())
	{
		return;
	}
	SortProxy->PreSort(InSelector, InTargets);
	if (InTargets.IsEmpty())
	{
		return;
	}
	const FInnerSortFunction SortFunction(InSelector, SortProxy);
	InTargets.Sort(SortFunction);
	SortProxy->PostSort(InSelector, InTargets);
}

TArray<UWorldTargetComponentBase*> UWorldTargetSelectionLibrary::DoSelection(const FWorldTargetSelectionRule& SelectionRule, UWorldTargetSelectionComponentBase* InSelector)
{
	UWorldTargetSelectionSubsystem* Subsystem = GetSubsystem(InSelector);
	if (!::IsValid(Subsystem))
	{
		return {};
	}
	WorldTargetSelectionType::TargetArrayType AllTargets = Subsystem->GetTargets(SelectionRule.Tag);
	if (AllTargets.IsEmpty())
	{
		return {};
	}
	TArray<UWorldTargetComponentBase*> FilteredTargets;
	for(const WorldTargetSelectionType::TargetArrayType::ElementType& Target : AllTargets)
	{
		bool bPass = true;
		for (const FWorldTargetSelectionConditionContainer& ConditionContainer : SelectionRule.ConditionContainer)
		{
			if (!IsConditionPass(ConditionContainer, InSelector, Target.Get()))
			{
				bPass = false;
				break;
			}
		}
		if (bPass)
		{
			FilteredTargets.Add(Target.Get());
		}
	}
	DoSort(SelectionRule.SortProxy, InSelector, FilteredTargets);
	const int32 Num = FilteredTargets.Num();
	if (SelectionRule.MaxTargetNumber > 0 && SelectionRule.MaxTargetNumber < Num)
	{
		for (int32 Index = Num - 1; Index >= SelectionRule.MaxTargetNumber; Index--)
		{
			FilteredTargets.RemoveAt(Index);
		}
	}
	bool bNeedRegister = (SelectionRule.SelectionReplaceType == EWorldTargetSelectionReplaceType::EAppend);
	if (SelectionRule.SelectionReplaceType == EWorldTargetSelectionReplaceType::EReplaceAnyway
		|| (SelectionRule.SelectionReplaceType == EWorldTargetSelectionReplaceType::EReplaceWhenNewTargetIsNotEmpty && FilteredTargets.Num() > 0))
	{
		Subsystem->UnregisterTargetSelection(SelectionRule.Tag, InSelector, nullptr, ETargetUnregisterReason::EReplacedInSelection);
		bNeedRegister =  true;
	}
	if (bNeedRegister)
	{
		for (UWorldTargetComponentBase* FilteredTarget : FilteredTargets)
		{
			Subsystem->RegisterTargetSelection(SelectionRule.Tag, InSelector, FilteredTarget);
		}
	}
	return FilteredTargets;
}

TArray<UWorldTargetComponentBase*> UWorldTargetSelectionLibrary::GetAndRefreshRegisteredTargets(const FName& Tag, UWorldTargetSelectionComponentBase* InSelector, bool bRefreshCondition, bool bRefreshSort)
{
	TArray<UWorldTargetComponentBase*> OutArray;
	if (!::IsValid(InSelector))
	{
		return OutArray;
	}
	UWorldTargetSelectionSubsystem* Subsystem = UWorldTargetSelectionLibrary::GetSubsystem(InSelector);
	if (!ensureAlways(::IsValid(Subsystem)))
	{
		return OutArray;
	}
	TArray<UWorldTargetComponentBase*> RemoveArray;
	WorldTargetSelectionType::TargetArrayType RegisteredTargets = Subsystem->GetTargetSelections(Tag, InSelector);
	FWorldTargetSelectionRule OutRule;
	const bool bHasRule = InSelector->GetRule(Tag, OutRule);
	for (const WorldTargetSelectionType::TargetArrayType::ElementType& RegisteredTarget : RegisteredTargets)
	{
		bool bPass = true;
		if (bRefreshCondition && bHasRule)
		{
			for (const FWorldTargetSelectionConditionContainer& ConditionContainer : OutRule.ConditionContainer)
			{
				if (!UWorldTargetSelectionLibrary::IsConditionPass(ConditionContainer, InSelector, RegisteredTarget.Get()))
				{
					bPass = false;
					break;
				}
			}
		}
		if (bPass)
		{
			OutArray.Add(RegisteredTarget.Get());
		}
		else
		{
			RemoveArray.Add(RegisteredTarget.Get());
		}
	}

	for (UWorldTargetComponentBase* RemovedTarget : RemoveArray)
	{
		Subsystem->UnregisterTargetSelection(Tag, InSelector, RemovedTarget, ETargetUnregisterReason::EFilteredInRefresh);
	}
	if (bRefreshSort && bHasRule)
	{
		UWorldTargetSelectionLibrary::DoSort(OutRule.SortProxy, InSelector, OutArray);
	}
	return OutArray;
}

