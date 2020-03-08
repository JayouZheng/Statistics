// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "StatisticsStyle.h"

class FStatisticsCommands : public TCommands<FStatisticsCommands>
{
public:

	FStatisticsCommands()
		: TCommands<FStatisticsCommands>(TEXT("Statistics"), NSLOCTEXT("Contexts", "Statistics", "Statistics Plugin"), NAME_None, FStatisticsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};