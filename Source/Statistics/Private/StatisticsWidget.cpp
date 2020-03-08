// ...

#include "StatisticsWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/SBoxPanel.h"
#include "ExporterHelper.h"
#include "Editor/UnrealEd/Public/Dialogs/SOutputLogDialog.h"
#include "Developer/SlateFileDialogs/Public/SlateFileDialogs.h"
#include "HAL/PlatformProcess.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
// #include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h" // PrintString()

DEFINE_LOG_CATEGORY_STATIC(Ansys_Zheng, Warning, All)

#define LOCTEXT_NAMESPACE "SStatisticsWidget"
#define LOCTEXTEX(InKey, InTextLiteral) FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(InTextLiteral, TEXT(LOCTEXT_NAMESPACE), TEXT(InKey))

void SStatisticsWidget::Construct(const FArguments& InArgs)
{
	FString InString = InArgs._InText.Get();

	this->ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("OutputPath", "当前输出路径 -> [未配置]"))
			]				
		]
		+SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				SNew(STextBlock).Text(LOCTEXT("VisualizationToolPath", "当前工具路径 -> [未配置]"))
			]
		]
		+SVerticalBox::Slot().AutoHeight().Padding(FMargin(0.0f, 4.0f, 0.0f, 4.0f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
			[
				SNew(STextBlock).Text(LOCTEXT("ScaleTip", "调整输入场景比例 "))
			]
			+SHorizontalBox::Slot()
			.FillWidth(1)
			.MaxWidth(100.f)
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
			[
				SNew(SNumericEntryBox<float>)
				.AllowSpin(true)
				.ToolTipText(LOCTEXT("ScaleTip", "调整输入场景比例"))
				.Value(this, &SStatisticsWidget::GetValue)
				.OnValueChanged(this, &SStatisticsWidget::OnSliderValueChanged)
				.MinValue(0.0001f)
				.MaxValue(1000.0f)
				.MinSliderValue(0.0001f)
				.MaxSliderValue(100.f)
				.SliderExponent(1.0f)
			]
		]
		+SVerticalBox::Slot()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				SNew(SButton).Text(LOCTEXT("PathPiker", "配置工具路径"))
				.OnClicked(this, &SStatisticsWidget::OnButtonChooseClicked)
			]		
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				SNew(SButton).Text(LOCTEXT("Export", "导出场景数据"))
				.OnClicked(this, &SStatisticsWidget::OnButtonExportClicked)
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f)
			[
				SNew(SButton).Text(LOCTEXT("Ansys", "启动可视化"))
				.OnClicked(this, &SStatisticsWidget::OnButtonAnalysisClicked)
			]
		]
		+SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock).Text(LOCTEXT("Hints", "Click to do something..."))
		]
	];

	// Init data...
	OutputPath = TEXT("未配置");
	VisualizationToolPath = TEXT("未配置");
	_Scale = 0.0002f;
	GConfig->GetString(TEXT("Output"), TEXT("Path"), OutputPath, 
		FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");
	GConfig->GetString(TEXT("VisualizationTool"), TEXT("Path"), VisualizationToolPath, 
		FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");
	GConfig->GetFloat(TEXT("Scale"), TEXT("Value"), _Scale,
		FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");

	FString ShowOutputPath = TEXT("当前输出路径 -> [") + OutputPath + "]";
	LOCTEXTEX("OutputPath", ShowOutputPath.GetCharArray().GetData());
	FString ShowToolPath = TEXT("当前工具路径 -> [") + VisualizationToolPath + "]";
	LOCTEXTEX("VisualizationToolPath", ShowToolPath.GetCharArray().GetData());
}

FReply SStatisticsWidget::OnButtonChooseClicked()
{
	SaveOutputPath();
	SaveVisualizationToolPath();

	return FReply::Handled();
}

FReply SStatisticsWidget::OnButtonExportClicked()
{
	if (OutputPath == TEXT("未配置"))
		SaveOutputPath();

	LOCTEXT("Hints", "请转到 [ Window ->Developer Tools ->Output Log ] 以查看详细信息!");

	// to do export...
	FPlatformProcess::ExploreFolder(OutputPath.GetCharArray().GetData());

	TMap<FString, bool> ResultPathsStates;
	FExporterHelper::ExportSceneDataToCSV(ResultPathsStates, OutputPath);
	FString OutputLogs; OutputLogs.Empty();
	for (TMap<FString, bool>::TIterator It(ResultPathsStates); It; ++It)
	{
		if ((*It).Value) 
		{ 
			OutputLogs += "-> Save to [" + (*It).Key + "] Success!\n";
			UE_LOG(Ansys_Zheng, Warning, TEXT("Save to [%s] Success!"), (*It).Key.GetCharArray().GetData());
		} 
		else
		{ 
			OutputLogs += "-> Save to [" + (*It).Key + "] Failed!\n";
			UE_LOG(Ansys_Zheng, Warning, TEXT("Save to [%s] Failed!"), (*It).Key.GetCharArray().GetData()); 
		}
	}
	
	SOutputLogDialog::Open(FText::FromString("Hint"), FText::FromString("Export Scene data..."), FText::FromString(OutputLogs));
	
	return FReply::Handled();
}

FReply SStatisticsWidget::OnButtonAnalysisClicked()
{
	if (OutputPath == TEXT("未配置"))
		SaveOutputPath();
	if (VisualizationToolPath == TEXT("未配置"))
		SaveVisualizationToolPath();

	LOCTEXT("Hints", "启动可视化");	
	
	FString WorldName = FExporterHelper::GetWorld()->GetName();
	FString PlatformName;
	PlatformName = UGameplayStatics::GetPlatformName();
	// UKismetSystemLibrary::PrintString(nullptr, platformName, true, true, FLinearColor(0.000000, 0.660000, 1.000000, 1.000000), 2.000000);
	if (PlatformName == "Windows")
	{
		_wsystem(FString(VisualizationToolPath + "/D3DVisualizationTool.exe -dir [" + OutputPath + "/World_" + WorldName + "] -scale [" + FString::SanitizeFloat(_Scale) + "]").GetCharArray().GetData());
	}

	SaveScale();

	return FReply::Handled();
}

void SStatisticsWidget::OnSliderValueChanged(float InVal)
{
	_Scale = InVal;
}

void SStatisticsWidget::SaveOutputPath()
{
	IDesktopPlatform* DP = FDesktopPlatformModule::Get();
	const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	const TSharedPtr<FGenericWindow> ParentGenericWindow = ParentWindow.IsValid() ? ParentWindow->GetNativeWindow() : nullptr;
	const void* ParentWindowHandle = ParentGenericWindow.IsValid() ? ParentGenericWindow->GetOSWindowHandle() : nullptr;

	DP->OpenDirectoryDialog(ParentWindowHandle, TEXT("选择一个文件夹作为输出目录"), "C:", OutputPath);

	FString ShowOutputPath = TEXT("当前输出路径 -> [") + OutputPath + "]";
	LOCTEXTEX("OutputPath", ShowOutputPath.GetCharArray().GetData());

	GConfig->SetString(TEXT("Output"), TEXT("Path"), OutputPath.GetCharArray().GetData(),
		FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");

	GConfig->Flush(false, FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");
}

void SStatisticsWidget::SaveVisualizationToolPath()
{
	IDesktopPlatform* DP = FDesktopPlatformModule::Get();
	const TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	const TSharedPtr<FGenericWindow> ParentGenericWindow = ParentWindow.IsValid() ? ParentWindow->GetNativeWindow() : nullptr;
	const void* ParentWindowHandle = ParentGenericWindow.IsValid() ? ParentGenericWindow->GetOSWindowHandle() : nullptr;

	DP->OpenDirectoryDialog(ParentWindowHandle, TEXT("请选择 D3DVisualizationTool.exe 所在的文件夹"), "C:", VisualizationToolPath);

	FString ShowToolPath = TEXT("当前工具路径 -> [") + VisualizationToolPath + "]";
	LOCTEXTEX("VisualizationToolPath", ShowToolPath.GetCharArray().GetData());

	GConfig->SetString(TEXT("VisualizationTool"), TEXT("Path"), VisualizationToolPath.GetCharArray().GetData(),
		FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");

	GConfig->Flush(false, FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");
}

void SStatisticsWidget::SaveScale()
{
	GConfig->SetFloat(TEXT("Scale"), TEXT("Value"), _Scale,
		FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");

	GConfig->Flush(false, FPaths::ProjectPluginsDir() + "Statistics/Config/PluginSetting.ini");
}

#undef LOCTEXT_NAMESPACE