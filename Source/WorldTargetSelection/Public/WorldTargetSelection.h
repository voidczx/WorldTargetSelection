// Copyright Beijing Bytedance Technology Co., Ltd. All Right Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class  FWorldTargetSelectionModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
