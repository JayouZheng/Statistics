// ...

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Input/Reply.h"

class SStatisticsWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SStatisticsWidget) {}
	SLATE_ATTRIBUTE(FString, InText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	FString OutputPath;
	FString VisualizationToolPath;
	float _Scale;

	// OnClicked
	FReply OnButtonChooseClicked();

	FReply OnButtonExportClicked();

	FReply OnButtonAnalysisClicked();

	TOptional<float> GetValue() const { return _Scale; }

	void OnSliderValueChanged(float InVal);

	void SaveOutputPath();

	void SaveVisualizationToolPath();

	void SaveScale();
};