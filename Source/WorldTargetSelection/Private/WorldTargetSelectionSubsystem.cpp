// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#include "WorldTargetSelectionSubsystem.h"

#include "WorldTargetComponent.h"
#include "WorldTargetSelectionComponent.h"

void UWorldTargetSelectionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

void UWorldTargetSelectionSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	LatentTargetRequestMap.Empty();
	LockCounterMap.Empty();
	TargetSelectionMap.Empty();
	Super::OnWorldEndPlay(InWorld);
}

bool UWorldTargetSelectionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Type::PIE || WorldType == EWorldType::Type::Game;
}

void UWorldTargetSelectionSubsystem::RefreshTarget(UWorldTargetComponentBase* InTarget)
{
	if (!ensureAlways(::IsValid(InTarget)))
	{
		return;
	}
	TArray<WorldTargetSelectionType::TargetTagType> TargetStatusTags;
	TArray<WorldTargetSelectionType::TargetTagType> TargetHasTags = InTarget->GetTags();
	for (const TPair<WorldTargetSelectionType::TargetTagType, WorldTargetSelectionType::TargetArrayType>& TargetPair : TargetMap)
	{
		if (TargetPair.Value.Contains(InTarget))
		{
			TargetStatusTags.Add(TargetPair.Key);
		}
	}
	for (const WorldTargetSelectionType::TargetTagType& MayRemoveTag : TargetStatusTags)
	{
		if (!TargetHasTags.Contains(MayRemoveTag))
		{
			RemoveTarget(MayRemoveTag, InTarget);
		}
	}
	for (const WorldTargetSelectionType::TargetTagType& MayAddTag : TargetHasTags)
	{
		if (!TargetStatusTags.Contains(MayAddTag))
		{
			AddTarget(MayAddTag, InTarget);
		}
	}
}

WorldTargetSelectionType::TargetArrayType UWorldTargetSelectionSubsystem::GetTargets(const WorldTargetSelectionType::TargetTagType& InTag) const
{
	const WorldTargetSelectionType::TargetArrayType* FoundArray = TargetMap.Find(InTag);
	if (FoundArray == nullptr)
	{
		return {};
	}
	return *FoundArray;
}

WorldTargetSelectionType::TargetArrayType UWorldTargetSelectionSubsystem::GetTargetSelections(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetSelectionComponentBase* Selector)
{
	if (!TargetSelectionMap.Contains(InTag))
	{
		return {};
	}
	if (!TargetSelectionMap[InTag].Contains(Selector))
	{
		return {};
	}
	return TargetSelectionMap[InTag][Selector];
}

void UWorldTargetSelectionSubsystem::RegisterTargetSelection(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetSelectionComponentBase* Selector, UWorldTargetComponentBase* Target)
{
	auto& SelectorToTargetSelections = TargetSelectionMap.FindOrAdd(InTag);
	auto& TargetSelections = SelectorToTargetSelections.FindOrAdd(Selector);
	TargetSelections.AddUnique(Target);
}

void UWorldTargetSelectionSubsystem::UnregisterTargetSelection(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetSelectionComponentBase* Selector, UWorldTargetComponentBase* Target, ETargetUnregisterReason InReason)
{
	if (!::IsValid(Selector))
	{
		return;
	}
	if (!TargetSelectionMap.Contains(InTag) || !TargetSelectionMap[InTag].Contains(Selector))
	{
		return;
	}
	if (::IsValid(Target))
	{
		TargetSelectionMap[InTag][Selector].Remove(Target);
		Selector->OnRegisteredTargetRemove(InTag, Target, InReason);
		if (TargetSelectionMap[InTag][Selector].IsEmpty())
		{
			TargetSelectionMap[InTag].Remove(Selector);
			Selector->OnRegisteredTargetEmpty(InTag, InReason);
		}
	}
	else
	{
		for (const TWeakObjectPtr<UWorldTargetComponentBase>& StoredTarget : TargetSelectionMap[InTag][Selector])
		{
			Selector->OnRegisteredTargetRemove(InTag, StoredTarget.Get(), InReason);
		}
		TargetSelectionMap[InTag].Remove(Selector);
		Selector->OnRegisteredTargetEmpty(InTag, InReason);
	}
}

void UWorldTargetSelectionSubsystem::UnregisterTargetSelectionBySelector(UWorldTargetSelectionComponentBase* Selector, ETargetUnregisterReason InReason)
{
	if (!::IsValid(Selector))
	{
		return;
	}
	for (auto& TargetSelectionPair : TargetSelectionMap)
	{
		if (TargetSelectionPair.Value.Contains(Selector))
		{
			for (TWeakObjectPtr<UWorldTargetComponentBase> Target : TargetSelectionPair.Value[Selector])
			{
				Selector->OnRegisteredTargetRemove(TargetSelectionPair.Key, Target.Get(), InReason);
			}
			TargetSelectionPair.Value.Remove(Selector);
			Selector->OnRegisteredTargetEmpty(TargetSelectionPair.Key, InReason);
		}
	}
}

void UWorldTargetSelectionSubsystem::AddTarget(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetComponentBase* Target)
{
	if (!ensureAlways(::IsValid(Target)))
	{
		return;
	}
	if (LockCounterMap.Contains(InTag))
	{
		TArray<FLatentTargetRequest>& Requests = LatentTargetRequestMap.FindOrAdd(InTag);
		Requests.Emplace(Target, true);
		return;
	}
	LockTargetTag(InTag);
	WorldTargetSelectionType::TargetArrayType& Targets = TargetMap.FindOrAdd(InTag);
	Targets.Emplace(Target);
	Target->OnTargetStatusChanged(InTag, true);
	ReleaseTargetTag(InTag);
}

void UWorldTargetSelectionSubsystem::RemoveTarget(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetComponentBase* Target)
{
	if (!ensureAlways(::IsValid(Target)))
	{
		return;
	}
	if (!TargetMap.Contains(InTag))
	{
		return;
	}
	if (LockCounterMap.Contains(InTag))
	{
		TArray<FLatentTargetRequest>& Requests = LatentTargetRequestMap.FindOrAdd(InTag);
		Requests.Emplace(Target, false);
		return;
	}
	LockTargetTag(InTag);
	if (TargetMap[InTag].Remove(Target) > 0)
	{

		const auto SelectorToTargetSelections = TargetSelectionMap.Find(InTag);
		if (SelectorToTargetSelections != nullptr)
		{
			TArray<TObjectKey<UWorldTargetSelectionComponentBase>> MayAffectedSelectors;
			SelectorToTargetSelections->GetKeys(MayAffectedSelectors);
			for (const TObjectKey<UWorldTargetSelectionComponentBase>& MayAffectedSelector : MayAffectedSelectors)
			{
				UnregisterTargetSelection(InTag, MayAffectedSelector.ResolveObjectPtr(), Target, ETargetUnregisterReason::ETargetTagRemove);
			}
		}
		Target->OnTargetStatusChanged(InTag, false);
	}
	ReleaseTargetTag(InTag);
}

void UWorldTargetSelectionSubsystem::LockTargetTag(const WorldTargetSelectionType::TargetTagType& InTag)
{
	if (!LockCounterMap.Contains(InTag))
	{
		LockCounterMap.Emplace(InTag, 0);
	}
	LockCounterMap[InTag]++;
}

void UWorldTargetSelectionSubsystem::ReleaseTargetTag(const WorldTargetSelectionType::TargetTagType& InTag)
{
	if (!ensureAlways(LockCounterMap.Contains(InTag)))
	{
		return;
	}
	LockCounterMap[InTag]--;
	if (LockCounterMap[InTag] <= 0)
	{
		LockCounterMap.Remove(InTag);
		if (LatentTargetRequestMap.Contains(InTag))
		{
			TArray<FLatentTargetRequest> Requests = LatentTargetRequestMap[InTag];
			for (const FLatentTargetRequest& Request : Requests)
			{
				RefreshTarget(Request.Component.Get());
			}
		}
	}
}

