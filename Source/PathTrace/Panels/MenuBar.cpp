#include <../tools/SystemCallls.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Sliders/SliderInt.h>
#include <UI/Widgets/Sliders/SliderFloat.h>
#include <UI/Widgets/Drags/DragFloat.h>
#include <UI/Widgets/Selection/ColorEdit.h>
#include "MenuBar.h"
using namespace UI::Panels;
using namespace UI::Widgets;
using namespace UI::Widgets::Menu;


MenuBar::MenuBar()
{
	CreateFileMenu();
	CreateWindowMenu();
	CreateResourcesMenu();

}



void MenuBar::CreateFileMenu()
{
	auto& fileMenu = CreateWidget<MenuList>("File");
	fileMenu.CreateWidget<MenuItem>("New Scene", "CTRL + N");
	fileMenu.CreateWidget<MenuItem>("Save Scene", "CTRL + S");
	fileMenu.CreateWidget<MenuItem>("Save Scene As...", "CTRL + SHIFT + S");
	fileMenu.CreateWidget<MenuItem>("Exit", "ALT + F4");
}



void MenuBar::CreateResourcesMenu()
{
	auto& resourcesMenu = CreateWidget<MenuList>("Resources");
	resourcesMenu.CreateWidget<MenuItem>("Compile shaders");
	resourcesMenu.CreateWidget<MenuItem>("Save materials");
}


void MenuBar::CreateWindowMenu()
{
	m_windowMenu = &CreateWidget<MenuList>("Window");
	m_windowMenu->CreateWidget<MenuItem>("Close all").ClickedEvent += std::bind(&MenuBar::OpenEveryWindows, this, false);
	m_windowMenu->CreateWidget<MenuItem>("Open all").ClickedEvent += std::bind(&MenuBar::OpenEveryWindows, this, true);
	m_windowMenu->CreateWidget<Visual::Separator>();

	/* When the menu is opened, we update which window is marked as "Opened" or "Closed" */
	m_windowMenu->ClickedEvent += std::bind(&MenuBar::UpdateToggleableItems, this);
}

void MenuBar::RegisterPanel(const std::string& p_name, UI::Panels::PanelWindow& p_panel)
{
	
	auto& menuItem = m_windowMenu->CreateWidget<MenuItem>(p_name, "", true, true);

	m_panels.emplace(p_name, std::make_pair(std::ref(p_panel), std::ref(menuItem)));
}

void MenuBar::UpdateToggleableItems()
{
	for (auto&[name, panel] : m_panels)
		panel.second.get().checked = panel.first.get().IsOpened();
}

void MenuBar::OpenEveryWindows(bool p_state)
{
	for (auto&[name, panel] : m_panels)
		panel.first.get().SetOpened(p_state);
}
