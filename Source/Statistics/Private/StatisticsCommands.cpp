// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "StatisticsCommands.h"

#define LOCTEXT_NAMESPACE "FStatisticsModule"

void FStatisticsCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Statistics", "Bring up Statistics window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
