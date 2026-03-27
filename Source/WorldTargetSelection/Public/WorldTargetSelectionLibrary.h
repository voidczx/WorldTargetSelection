// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorldTargetSelectionType.h"

#include "WorldTargetSelectionLibrary.generated.h"

UCLASS()
class WORLDTARGETSELECTION_API UWorldTargetSelectionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure)
	static UWorldTargetSelectionSubsystem* GetSubsystem(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable)
	static bool IsConditionPass(const FWorldTargetSelectionConditionContainer& ConditionContainer, UWorldTargetSelectionComponentBase* InSelector, UWorldTargetComponentBase* InTarget);

	UFUNCTION(BlueprintCallable)
	static void DoSort(UWorldTargetSelectionSortProxyBase* SortProxy, UWorldTargetSelectionComponentBase* InSelector, UPARAM(ref) TArray<UWorldTargetComponentBase*>& InTargets);

	UFUNCTION(BlueprintCallable)
	static TArray<UWorldTargetComponentBase*> DoSelection(const FWorldTargetSelectionRule& SelectionRule, UWorldTargetSelectionComponentBase* InSelector);

	UFUNCTION(BlueprintCallable)
	static TArray<UWorldTargetComponentBase*> GetAndRefreshRegisteredTargets(const FName& Tag, UWorldTargetSelectionComponentBase* InSelector, bool bRefreshCondition = false, bool bRefreshSort = false);

};
