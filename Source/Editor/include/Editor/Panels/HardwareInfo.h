
#pragma once

#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Plots/PlotLines.h>
#include <UI/Widgets/Plots/PlotHistogram.h>

namespace Analytics::Hardware { class HardwareInfo; }

namespace Editor::Panels
{
	class HardwareInfo : public UI::Panels::PanelWindow
	{
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		* @param p_logFrequency
		* @param p_maxElements
		* @param p_displayMode
		*/
		HardwareInfo
		(
			const std::string& p_title,
			bool p_opened,
			const UI::Settings::PanelWindowSettings& p_windowSettings,
			float p_logFrequency,
			size_t p_maxElements
		);

		/**
		* Destructor
		*/
		~HardwareInfo();

		/**
		* Update hardware info
		* @param p_deltaTime
		*/
		void Update(float p_deltaTime);

	private:
		float p_updateTimer = 0.f;
		float m_logFrequency;
		size_t m_maxElements;
		UI::Widgets::Plots::APlot* m_cpuUsage;
		UI::Widgets::Plots::APlot* m_gpuUsage;
		UI::Widgets::Plots::APlot* m_ramUsage;
		Analytics::Hardware::HardwareInfo* m_hardwareInfo;
	};
}