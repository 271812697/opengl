#pragma once
#include <UI/Panels/Inspector.h>
#include <UI/Widgets/Layout/GroupCollapsable.h>
#include <UI/Widgets/InputFields/InputText.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Selection/ComboBox.h>
class TransformInspector : public UI::Panels::Inspector
{
public:

	TransformInspector
	(
		const std::string& p_title,
		bool p_opened,
		const UI::Settings::PanelWindowSettings& p_windowSettings
	);
	~TransformInspector();
	void InstallUI() override;
private:
};