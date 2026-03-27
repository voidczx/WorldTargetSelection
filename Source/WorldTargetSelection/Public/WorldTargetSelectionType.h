// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WorldTargetSelectionType.generated.h"

class UWorldTargetComponentBase;
class UWorldTargetSelectionComponentBase;
class UWorldTargetSelectionSubsystem;

namespace WorldTargetSelectionType
{
	typedef FName TargetTagType;
	typedef TArray<TWeakObjectPtr<UWorldTargetComponentBase>> TargetArrayType;
	typedef TMap<TargetTagType, TargetArrayType> TargetMapType;
}


UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class WORLDTARGETSELECTION_API UWorldTargetSelectionConditionBase : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	bool IsConditionPass(UWorldTargetSelectionComponentBase* InSelector, UWorldTargetComponentBase* InTarget);

};

UENUM(BlueprintType)
enum class EWorldTargetSelectionConditionType : uint8
{
	EAnd = 0, // 00 (A && B && C && ...)
	EOr = 1, // 01 (A || B || C || ...)
	ENotAnd = 2, // 10 !(A && B && C && ...)
	ENotOr = 3, // 11 !(A || B || C || ...)
};

USTRUCT(BlueprintType)
struct WORLDTARGETSELECTION_API FWorldTargetSelectionConditionContainer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EWorldTargetSelectionConditionType Type;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Instanced)
	TArray<UWorldTargetSelectionConditionBase*> InnerObjects;

};

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class WORLDTARGETSELECTION_API UWorldTargetSelectionSortProxyBase : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintImplementableEvent)
	void PreSort(UWorldTargetSelectionComponentBase* InSelector, UPARAM(ref) TArray<UWorldTargetComponentBase*>& InTargets);

	UFUNCTION(BlueprintImplementableEvent)
	bool SortFunction(UWorldTargetSelectionComponentBase* InSelector, UWorldTargetComponentBase* Left, UWorldTargetComponentBase* Right);

	UFUNCTION(BlueprintImplementableEvent)
	void PostSort(UWorldTargetSelectionComponentBase* InSelector, UPARAM(ref) TArray<UWorldTargetComponentBase*>& InTargets);

};

UENUM(BlueprintType)
enum class EWorldTargetSelectionReplaceType : uint8{
	EReplaceAnyway,
	EReplaceWhenNewTargetIsNotEmpty,
	EAppend,
	DoNothing,
};

USTRUCT(BlueprintType)
struct WORLDTARGETSELECTION_API FWorldTargetSelectionRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWorldTargetSelectionReplaceType SelectionReplaceType = EWorldTargetSelectionReplaceType::EReplaceAnyway;

	// need pass every condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FWorldTargetSelectionConditionContainer> ConditionContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	UWorldTargetSelectionSortProxyBase* SortProxy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTargetNumber = -1;
};

UENUM(BlueprintType)
enum class ETargetUnregisterReason : uint8
{
	EReplacedInSelection,
	ESelectorEndPlay,
	EFilteredInRefresh,
	ETargetTagRemove,
	EClearedByCustomer,
};

