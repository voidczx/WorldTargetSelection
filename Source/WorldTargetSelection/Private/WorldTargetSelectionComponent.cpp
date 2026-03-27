// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#include "WorldTargetSelectionComponent.h"

#include "WorldTargetComponent.h"
#include "WorldTargetSelectionLibrary.h"
#include "WorldTargetSelectionSubsystem.h"

TArray<FName> UWorldTargetSelectionComponentBase::DefaultModeNames = {NAME_None, "Default", "Common"};

void UWorldTargetSelectionComponentBase::BeginPlay()
{
	Super::BeginPlay();
	for (const FWorldTargetSelectionComponentRule& ConfigRule : ConfigRules)
	{
		if (DefaultModeNames.Contains(ConfigRule.ModeName))
		{
			Rules.Emplace(ConfigRule.Rule.Tag, ConfigRule.Rule);
		}
	}
}


void UWorldTargetSelectionComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UWorldTargetSelectionSubsystem* Subsystem = UWorldTargetSelectionLibrary::GetSubsystem(this);
	if (ensureAlways(::IsValid(Subsystem)))
	{
		Subsystem->UnregisterTargetSelectionBySelector(this, ETargetUnregisterReason::ESelectorEndPlay);
	}
	Super::EndPlay(EndPlayReason);
}

void UWorldTargetSelectionComponentBase::ChangeSelectionMode(const FName& InTag, const FName& NewModeName)
{
	Rules.Remove(InTag);
	const bool bDefault = DefaultModeNames.Contains(NewModeName);
	for (const FWorldTargetSelectionComponentRule& ConfigRule : ConfigRules)
	{
		if (ConfigRule.Rule.Tag == InTag)
		{
			if ((bDefault && DefaultModeNames.Contains(ConfigRule.ModeName)) || NewModeName == ConfigRule.ModeName)
			{
				Rules.Emplace(InTag, ConfigRule.Rule);
			}
		}
	}
}

bool UWorldTargetSelectionComponentBase::GetRule(const FName& Tag, FWorldTargetSelectionRule& OutRule) const
{
	const FWorldTargetSelectionRule* RulePtr = Rules.Find(Tag);
	if (RulePtr == nullptr)
	{
		return false;
	}
	OutRule = *RulePtr;
	return true;
}

TArray<UWorldTargetComponentBase*> UWorldTargetSelectionComponentBase::GetTargets(const FName& Tag)
{
	FWorldTargetSelectionRule* RulePtr = Rules.Find(Tag);
	if (RulePtr == nullptr)
	{
		return {};
	}
	UWorldTargetSelectionSubsystem* Subsystem = UWorldTargetSelectionLibrary::GetSubsystem(this);
	if (!ensureAlways(::IsValid(Subsystem)))
	{
		return {};
	}
	TArray<UWorldTargetComponentBase*> Targets = UWorldTargetSelectionLibrary::DoSelection(*RulePtr, this);
	return Targets;
}

TArray<UWorldTargetComponentBase*> UWorldTargetSelectionComponentBase::GetRegisteredTargets(const FName& Tag)
{
	return UWorldTargetSelectionLibrary::GetAndRefreshRegisteredTargets(Tag, this, false, false);
}

void UWorldTargetSelectionComponentBase::UnregisterTargets(const FName& Tag, const TArray<UWorldTargetComponentBase*>& Targets)
{
	UWorldTargetSelectionSubsystem* Subsystem = UWorldTargetSelectionLibrary::GetSubsystem(this);
	if (!::IsValid(Subsystem))
	{
		return;
	}
	for (UWorldTargetComponentBase* Target : Targets)
	{
		Subsystem->UnregisterTargetSelection(Tag, this, Target, ETargetUnregisterReason::EClearedByCustomer);
	}
}

void UWorldTargetSelectionComponentBase::OnRegisteredTargetRemove(const WorldTargetSelectionType::TargetTagType& Tag, UWorldTargetComponentBase* Target, ETargetUnregisterReason InReason)
{
	ReceiveRegisteredTargetRemove(Tag, Target, InReason);
}

void UWorldTargetSelectionComponentBase::OnRegisteredTargetEmpty(const WorldTargetSelectionType::TargetTagType& Tag, ETargetUnregisterReason InReason)
{
	ReceiveRegisteredTargetEmpty(Tag, InReason);
}

