// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#include "WorldTargetComponent.h"

#include "WorldTargetSelectionLibrary.h"
#include "WorldTargetSelectionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"


UWorldTargetComponentBase::UWorldTargetComponentBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UWorldTargetComponentBase::BeginPlay()
{
	Super::BeginPlay();
	if (!IsNetSimulating())
	{
		ReplicatedTags.Append(ConfigTags);
		if (IsRunningDedicatedServer())
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(UWorldTargetComponentBase, ReplicatedTags, this);
		}
	}
	NotifySubsystemRefreshTarget();
}

void UWorldTargetComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReplicatedTags.Empty();
	NotifySubsystemRefreshTarget();
	Super::EndPlay(EndPlayReason);
}

void UWorldTargetComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UWorldTargetComponentBase, ReplicatedTags, SharedParams);
}

void UWorldTargetComponentBase::OnRep_ReplicatedTags()
{
	if (!HasBegunPlay() || !IsNetSimulating())
	{
		return;
	}
	NotifySubsystemRefreshTarget();
}

TArray<FName> UWorldTargetComponentBase::GetTags() const
{
	return ReplicatedTags;
}

void UWorldTargetComponentBase::AddTag(const FName& InTag)
{
	if (IsNetSimulating() || HasTag(InTag))
	{
		return;
	}
	ReplicatedTags.Add(InTag);
	if (IsRunningDedicatedServer())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UWorldTargetComponentBase, ReplicatedTags, this);
	}
}

void UWorldTargetComponentBase::RemoveTag(const FName& InTag)
{
	if (IsNetSimulating() || !HasTag(InTag))
	{
		return;
	}
	ReplicatedTags.Remove(InTag);
	if (IsRunningDedicatedServer())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UWorldTargetComponentBase, ReplicatedTags, this);
	}
}

bool UWorldTargetComponentBase::HasTag(const FName& InTag)
{
	return ReplicatedTags.Contains(InTag);
}

void UWorldTargetComponentBase::OnTargetStatusChanged(const WorldTargetSelectionType::TargetTagType& InTag, bool bAdd)
{
	ReceiveTargetStatusChanged(InTag, bAdd);
}

void UWorldTargetComponentBase::NotifySubsystemRefreshTarget()
{
	UWorldTargetSelectionSubsystem* Subsystem = UWorldTargetSelectionLibrary::GetSubsystem(this);
	if (!ensureAlways(::IsValid(Subsystem)))
	{
		return;
	}
	Subsystem->RefreshTarget(this);
}

