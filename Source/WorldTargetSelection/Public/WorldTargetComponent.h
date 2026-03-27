// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorldTargetSelectionType.h"

#include "WorldTargetComponent.generated.h"

UCLASS(Blueprintable)
class WORLDTARGETSELECTION_API UWorldTargetComponentBase : public USceneComponent
{
	GENERATED_BODY()

public:

	UWorldTargetComponentBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ConfigTags;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_ReplicatedTags)
	TArray<FName> ReplicatedTags;
	UFUNCTION()
	void OnRep_ReplicatedTags();

	UFUNCTION()
	TArray<FName> GetTags() const;

	UFUNCTION(BlueprintCallable)
	void AddTag(const FName& InTag);

	UFUNCTION(BlueprintCallable)
	void RemoveTag(const FName& InTag);

	UFUNCTION(BlueprintCallable)
	bool HasTag(const FName& InTag);

	virtual void OnTargetStatusChanged(const WorldTargetSelectionType::TargetTagType& InTag, bool bAdd);
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveTargetStatusChanged(const FName& InTag, bool bAdd);

protected:

	void NotifySubsystemRefreshTarget();

};
