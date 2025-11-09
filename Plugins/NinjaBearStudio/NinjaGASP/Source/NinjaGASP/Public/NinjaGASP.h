// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "Modules/ModuleManager.h"

class FNinjaGASPModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
