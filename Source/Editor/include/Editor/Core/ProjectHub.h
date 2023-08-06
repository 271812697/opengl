#pragma once
#include <Rendering/Core/Renderer.h>
#include <Window/Device.h>
#include <Window/Window.h>
#include <Rendering/Context/Driver.h>
#include <UI/Core/UIManager.h>
#include <UI/Panels/PanelWindow.h>

namespace Editor::Core
{
	/**
	* A simple panel that allow the user to select the project to launch
	*/
	class ProjectHub
	{
	public:
		/**
		* Constructor
		*/
		ProjectHub();

		/**
		* Run the project hub logic
		*/
		std::tuple<bool, std::string, std::string> Run();

		/**
		* Setup the project hub specific context (minimalist context)
		*/
		void SetupContext();

		/**
		* Register the project (identified from the given path) into the project hub
		* @param p_path
		*/
		void RegisterProject(const std::string& p_path);

	private:
		std::unique_ptr<Windowing::Context::Device>		m_device;
		std::unique_ptr<Windowing::Window>				m_window;
		std::unique_ptr<Rendering::Context::Driver>		m_driver;
		std::unique_ptr<Rendering::Core::Renderer>		m_renderer;
		std::unique_ptr<UI::Core::UIManager>				m_uiManager;

		UI::Modules::Canvas m_canvas;
		std::unique_ptr<UI::Panels::PanelWindow>			m_mainPanel;

		std::string m_projectPath = "";
		std::string m_projectName = "";
		bool m_readyToGo = false;
	};
}