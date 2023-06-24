#pragma once
#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Layout/GroupCollapsable.h>
#include <UI/Widgets/InputFields/InputText.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Selection/ComboBox.h>
class Inspector : public UI::Panels::PanelWindow
{
public:

	Inspector
	(
		const std::string& p_title,
		bool p_opened,
		const UI::Settings::PanelWindowSettings& p_windowSettings
	);


	~Inspector();



	void Refresh();

private:

	UI::Widgets::Layout::Group* m_actorInfo;
	UI::Widgets::Layout::Group* m_inspectorHeader;
	UI::Widgets::Selection::ComboBox* m_componentSelectorWidget;
	UI::Widgets::InputFields::InputText* m_scriptSelectorWidget;


};