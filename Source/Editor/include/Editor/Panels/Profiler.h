
#pragma once

#include <Analytics/Profiling/Profiler.h>

#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Texts/TextColored.h>
#include <UI/Widgets/Selection/CheckBox.h>
#include <UI/Widgets/Layout/Group.h>
#include <UI/Widgets/Layout/Columns.h>
#include <UI/Widgets/Buttons/Button.h>

namespace Editor::Panels
{
	class Profiler : public UI::Panels::PanelWindow
	{
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		* @param p_frequency
		*/
		Profiler
		(
			const std::string& p_title,
			bool p_opened,
			const UI::Settings::PanelWindowSettings& p_windowSettings,
			float p_frequency
		);

		/**
		* Update profiling information
		* @param p_deltaTime
		*/
		void Update(float p_deltaTime);

		/**
		* Enable or disable the profiler
		* @param p_value
		* @param p_disableLog
		*/
		void Enable(bool p_value, bool p_disableLog = false);

	private:
		UI::Types::Color CalculateActionColor(double p_percentage) const;
		std::string GenerateActionString(Analytics::Profiling::ProfilerReport::Action& p_action);

	private:
		enum class EProfilingMode
		{
			DEFAULT,
			CAPTURE
		};

		float m_frequency;
		float m_timer = 0.f;
		float m_fpsTimer = 0.f;
		EProfilingMode m_profilingMode = EProfilingMode::DEFAULT;

		Analytics::Profiling::Profiler m_profiler;

		UI::Widgets::AWidget* m_separator;
		UI::Widgets::Buttons::Button* m_captureResumeButton;
		UI::Widgets::Texts::TextColored* m_fpsText;
		UI::Widgets::Texts::TextColored* m_elapsedFramesText;
		UI::Widgets::Texts::TextColored* m_elapsedTimeText;
		UI::Widgets::Layout::Columns<5>* m_actionList;
	};
}