

#include <algorithm>

#include "Editor/Panels/Console.h"
#include "Editor/Core/EditorActions.h"

#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Layout/Spacing.h>

using namespace UI::Panels;
using namespace UI::Widgets;

std::pair<UI::Types::Color, std::string> GetWidgetSettingsFromLogData(const Debug::LogData& p_logData)
{
	UI::Types::Color logColor;
	std::string logHeader;
	std::string logDateFormated = "[";
	bool isSecondPart = false;
	std::for_each(p_logData.date.begin(), p_logData.date.end(), [&logDateFormated, &isSecondPart](char c)
	{ 
		if (isSecondPart)
			logDateFormated.push_back(c == '-' ? ':' : c);

		if (c == '_')
			isSecondPart = true;
	});

	logDateFormated += "] ";

	switch (p_logData.logLevel)
	{
	default:
	case Debug::ELogLevel::LOG_DEFAULT:	return { { 1.f, 1.f, 1.f, 1.f }, logDateFormated };
	case Debug::ELogLevel::LOG_INFO:		return { { 0.f, 1.f, 1.f, 1.f }, logDateFormated };
	case Debug::ELogLevel::LOG_WARNING:	return { { 1.f, 1.f, 0.f, 1.f }, logDateFormated };
	case Debug::ELogLevel::LOG_ERROR:		return { { 1.f, 0.f, 0.f, 1.f }, logDateFormated };
	}
}

Editor::Panels::Console::Console
(
	const std::string& p_title,
	bool p_opened,
	const UI::Settings::PanelWindowSettings& p_windowSettings
) :
	PanelWindow(p_title, p_opened, p_windowSettings)
{
	allowHorizontalScrollbar = true;

	auto& clearButton = CreateWidget<Buttons::Button>("Clear");
	clearButton.size = { 50.f, 0.f };
	clearButton.idleBackgroundColor = { 0.5f, 0.f, 0.f };
	clearButton.ClickedEvent += std::bind(&Console::Clear, this);
	clearButton.lineBreak = false;

	auto& clearOnPlay = CreateWidget<Selection::CheckBox>(m_clearOnPlay, "Auto clear on play");

	CreateWidget<Layout::Spacing>(5).lineBreak = false;

	auto& enableDefault = CreateWidget<Selection::CheckBox>(true, "Default");
	auto& enableInfo = CreateWidget<Selection::CheckBox>(true, "Info");
	auto& enableWarning = CreateWidget<Selection::CheckBox>(true, "Warning");
	auto& enableError = CreateWidget<Selection::CheckBox>(true, "Error");

	clearOnPlay.lineBreak = false;
	enableDefault.lineBreak = false;
	enableInfo.lineBreak = false;
	enableWarning.lineBreak = false;
	enableError.lineBreak = true;

	clearOnPlay.ValueChangedEvent += [this](bool p_value) { m_clearOnPlay = p_value; };
	enableDefault.ValueChangedEvent += std::bind(&Console::SetShowDefaultLogs, this, std::placeholders::_1);
	enableInfo.ValueChangedEvent += std::bind(&Console::SetShowInfoLogs, this, std::placeholders::_1);
	enableWarning.ValueChangedEvent += std::bind(&Console::SetShowWarningLogs, this, std::placeholders::_1);
	enableError.ValueChangedEvent += std::bind(&Console::SetShowErrorLogs, this, std::placeholders::_1);

	CreateWidget<Visual::Separator>();

	m_logGroup = &CreateWidget<Layout::Group>();
    m_logGroup->ReverseDrawOrder();

	EDITOR_EVENT(PlayEvent) += std::bind(&Console::ClearOnPlay, this);

	Debug::Logger::LogEvent += std::bind(&Console::OnLogIntercepted, this, std::placeholders::_1);
}

void Editor::Panels::Console::OnLogIntercepted(const Debug::LogData & p_logData)
{
	auto[logColor, logDate] = GetWidgetSettingsFromLogData(p_logData);

	auto& consoleItem1 = m_logGroup->CreateWidget<Texts::TextColored>(logDate + "\t" + p_logData.message, logColor);

	consoleItem1.enabled = IsAllowedByFilter(p_logData.logLevel);

	m_logTextWidgets[&consoleItem1] = p_logData.logLevel;
}

void Editor::Panels::Console::ClearOnPlay()
{
	if (m_clearOnPlay)
		Clear();
}

void Editor::Panels::Console::Clear()
{
	m_logTextWidgets.clear();
	m_logGroup->RemoveAllWidgets();
}

void Editor::Panels::Console::FilterLogs()
{
	for (const auto&[widget, logLevel] : m_logTextWidgets)
		widget->enabled = IsAllowedByFilter(logLevel);
}

bool Editor::Panels::Console::IsAllowedByFilter(Debug::ELogLevel p_logLevel)
{
	switch (p_logLevel)
	{
	case Debug::ELogLevel::LOG_DEFAULT:	return m_showDefaultLog;
	case Debug::ELogLevel::LOG_INFO:		return m_showInfoLog;
	case Debug::ELogLevel::LOG_WARNING:	return m_showWarningLog;
	case Debug::ELogLevel::LOG_ERROR:		return m_showErrorLog;
	}

	return false;
}

void Editor::Panels::Console::SetShowDefaultLogs(bool p_value)
{
	m_showDefaultLog = p_value;
	FilterLogs();
}

void Editor::Panels::Console::SetShowInfoLogs(bool p_value)
{
	m_showInfoLog = p_value;
	FilterLogs();
}

void Editor::Panels::Console::SetShowWarningLogs(bool p_value)
{
	m_showWarningLog = p_value;
	FilterLogs();
}

void Editor::Panels::Console::SetShowErrorLogs(bool p_value)
{
	m_showErrorLog = p_value;
	FilterLogs();
}
