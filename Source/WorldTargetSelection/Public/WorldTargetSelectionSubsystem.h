// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorldTargetSelectionType.h"

#include "WorldTargetSelectionSubsystem.generated.h"

UCLASS()
class WORLDTARGETSELECTION_API UWorldTargetSelectionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void OnWorldEndPlay(UWorld& InWorld) override;

	void RefreshTarget(UWorldTargetComponentBase* InTarget);
	WorldTargetSelectionType::TargetArrayType GetTargets(const WorldTargetSelectionType::TargetTagType& InTag) const;
	WorldTargetSelectionType::TargetArrayType GetTargetSelections(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetSelectionComponentBase* Selector);
	void RegisterTargetSelection(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetSelectionComponentBase* Selector, UWorldTargetComponentBase* Target);
	void UnregisterTargetSelection(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetSelectionComponentBase* Selector, UWorldTargetComponentBase* Target, ETargetUnregisterReason InReason);
	void UnregisterTargetSelectionBySelector(UWorldTargetSelectionComponentBase* Selector, ETargetUnregisterReason InReason);

protected:

	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	void AddTarget(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetComponentBase* Target);
	void RemoveTarget(const WorldTargetSelectionType::TargetTagType& InTag, UWorldTargetComponentBase* Target);
	void LockTargetTag(const WorldTargetSelectionType::TargetTagType& InTag);
	void ReleaseTargetTag(const WorldTargetSelectionType::TargetTagType& InTag);

	WorldTargetSelectionType::TargetMapType TargetMap;
	TMap<WorldTargetSelectionType::TargetTagType, TMap<TObjectKey<UWorldTargetSelectionComponentBase>, WorldTargetSelectionType::TargetArrayType>> TargetSelectionMap;

private:

	TMap<WorldTargetSelectionType::TargetTagType, uint32> LockCounterMap;

	struct FLatentTargetRequest
	{
		FLatentTargetRequest(TWeakObjectPtr<UWorldTargetComponentBase> InComponent, bool InbAdd) : Component(InComponent), bAdd(InbAdd) {}
		TWeakObjectPtr<UWorldTargetComponentBase> Component = nullptr;
		bool bAdd = false;
	};
	TMap<WorldTargetSelectionType::TargetTagType, TArray<FLatentTargetRequest>> LatentTargetRequestMap;

};
