// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Statistics.h"
#include "StatisticsStyle.h"
#include "StatisticsCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "StatisticsWidget.h"

static const FName StatisticsTabName("Statistics");

#define LOCTEXT_NAMESPACE "FStatisticsModule"

void FStatisticsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FStatisticsStyle::Initialize();
	FStatisticsStyle::ReloadTextures();

	FStatisticsCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FStatisticsCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FStatisticsModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FStatisticsModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FStatisticsModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(StatisticsTabName, FOnSpawnTab::CreateRaw(this, &FStatisticsModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FStatisticsTabTitle", "Statistics"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FStatisticsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FStatisticsStyle::Shutdown();

	FStatisticsCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StatisticsTabName);
}

TSharedRef<SDockTab> FStatisticsModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FStatisticsModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("Statistics.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab).ShouldAutosize(true)
		[
			SNew(SStatisticsWidget).InText(FString("Hello Slate!"))
		];
}

void FStatisticsModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(StatisticsTabName);
}

void FStatisticsModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FStatisticsCommands::Get().OpenPluginWindow);
}

void FStatisticsModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FStatisticsCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStatisticsModule, Statistics)