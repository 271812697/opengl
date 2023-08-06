

#pragma once

#include <UI/Panels/PanelMenuBar.h>
#include <UI/Panels/PanelWindow.h>
#include <UI/Widgets/Menu/MenuItem.h>

namespace Editor::Panels
{
	class MenuBar : public UI::Panels::PanelMenuBar
	{
		using PanelMap = std::unordered_map<std::string, std::pair<std::reference_wrapper<UI::Panels::PanelWindow>, std::reference_wrapper<UI::Widgets::Menu::MenuItem>>>;

	public:
		/**
		* Constructor
		*/
		MenuBar();
		
		/**
		* Check inputs for menubar shortcuts
		* @param p_deltaTime
		*/
		void HandleShortcuts(float p_deltaTime);

		/**
		* Register a panel to the menu bar window menu
		*/
		void RegisterPanel(const std::string& p_name, UI::Panels::PanelWindow& p_panel);

	private:
		void CreateFileMenu();
		void CreateBuildMenu();
		void CreateWindowMenu();
		void CreateActorsMenu();
		void CreateResourcesMenu();
		void CreateSettingsMenu();
		void CreateLayoutMenu();
		void CreateHelpMenu();

		void UpdateToggleableItems();
		void OpenEveryWindows(bool p_state);

	private:
		PanelMap m_panels;

		UI::Widgets::Menu::MenuList* m_windowMenu = nullptr;
	};
}