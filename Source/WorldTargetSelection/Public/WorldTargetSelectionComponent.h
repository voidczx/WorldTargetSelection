// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#pragma once

#include "WorldTargetSelectionType.h"

#include "WorldTargetSelectionComponent.generated.h"

USTRUCT(BlueprintType)
struct WORLDTARGETSELECTION_API FWorldTargetSelectionComponentRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ModeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWorldTargetSelectionRule Rule;

};

UCLASS(Blueprintable)
class WORLDTARGETSELECTION_API UWorldTargetSelectionComponentBase : public USceneComponent
{
	GENERATED_BODY()
public:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	static TArray<FName> DefaultModeNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWorldTargetSelectionComponentRule> ConfigRules;

	UFUNCTION(BlueprintCallable)
	void ChangeSelectionMode(const FName& InTag, const FName& NewModeName);

	UFUNCTION(BlueprintCallable)
	bool GetRule(const FName& Tag, UPARAM(ref) FWorldTargetSelectionRule& OutRule) const;

	// select targets with rules, and register all of them to subsystem.
	UFUNCTION(BlueprintCallable)
	TArray<UWorldTargetComponentBase*> GetTargets(const FName& Tag);

	// will not do selection, just return registered targets with re-filter or re-sort.
	UFUNCTION(BlueprintCallable)
	TArray<UWorldTargetComponentBase*> GetRegisteredTargets(const FName& Tag);

	UFUNCTION(BlueprintCallable)
	void UnregisterTargets(const FName& Tag, const TArray<UWorldTargetComponentBase*>& Targets);

	virtual void OnRegisteredTargetRemove(const WorldTargetSelectionType::TargetTagType& Tag, UWorldTargetComponentBase* Target, ETargetUnregisterReason InReason);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveRegisteredTargetRemove(const FName& Tag, UWorldTargetComponentBase* Target, ETargetUnregisterReason InReason);

	virtual void OnRegisteredTargetEmpty(const WorldTargetSelectionType::TargetTagType& Tag, ETargetUnregisterReason InReason);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveRegisteredTargetEmpty(const FName& Tag, ETargetUnregisterReason InReason);

protected:

	TMap<WorldTargetSelectionType::TargetTagType, FName> SelectionModes;
	TMap<WorldTargetSelectionType::TargetTagType, FWorldTargetSelectionRule> Rules;

};
