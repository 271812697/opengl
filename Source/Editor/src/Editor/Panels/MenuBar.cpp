
#include"tools/SystemCallls.h"

#include <Core/ECS/Components/CCamera.h>
#include <Core/ECS/Components/CPointLight.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CSpotLight.h>
#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/ECS/Components/CPhysicalBox.h>
#include <Core/ECS/Components/CPhysicalSphere.h>
#include <Core/ECS/Components/CPhysicalCapsule.h>
#include <Core/ECS/Components/CAudioSource.h>
#include <Core/ECS/Components/CAudioListener.h>

#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Sliders/SliderInt.h>
#include <UI/Widgets/Sliders/SliderFloat.h>
#include <UI/Widgets/Drags/DragFloat.h>
#include <UI/Widgets/Selection/ColorEdit.h>

#include "Editor/Panels/MenuBar.h"
#include "Editor/Panels/SceneView.h"
#include "Editor/Panels/AssetView.h"
#include "Editor/Core/EditorActions.h"
#include "Editor/Settings/EditorSettings.h"
#include "Editor/Utils/ActorCreationMenu.h"

using namespace UI::Panels;
using namespace UI::Widgets;
using namespace UI::Widgets::Menu;
using namespace Core::ECS::Components;

Editor::Panels::MenuBar::MenuBar()
{
	CreateFileMenu();
	CreateBuildMenu();
	CreateWindowMenu();
	CreateActorsMenu();
	CreateResourcesMenu();
	CreateSettingsMenu();
	CreateLayoutMenu();
	CreateHelpMenu();
}

void Editor::Panels::MenuBar::HandleShortcuts(float p_deltaTime)
{
	auto& inputManager = *EDITOR_CONTEXT(inputManager);

	if (inputManager.GetKeyState(Windowing::Inputs::EKey::KEY_LEFT_CONTROL) == Windowing::Inputs::EKeyState::KEY_DOWN)
	{
		if (inputManager.IsKeyPressed(Windowing::Inputs::EKey::KEY_N))
			EDITOR_EXEC(LoadEmptyScene());

		if (inputManager.IsKeyPressed(Windowing::Inputs::EKey::KEY_S))
		{
			if (inputManager.GetKeyState(Windowing::Inputs::EKey::KEY_LEFT_SHIFT) == Windowing::Inputs::EKeyState::KEY_UP)
				EDITOR_EXEC(SaveSceneChanges());
			else
				EDITOR_EXEC(SaveAs());
		}
	}
}

void Editor::Panels::MenuBar::CreateFileMenu()
{
	auto& fileMenu = CreateWidget<MenuList>("File");
	fileMenu.CreateWidget<MenuItem>("New Scene", "CTRL + N").ClickedEvent					+= EDITOR_BIND(LoadEmptyScene);
	fileMenu.CreateWidget<MenuItem>("Save Scene", "CTRL + S").ClickedEvent					+= EDITOR_BIND(SaveSceneChanges);
	fileMenu.CreateWidget<MenuItem>("Save Scene As...", "CTRL + SHIFT + S").ClickedEvent	+= EDITOR_BIND(SaveAs);
	fileMenu.CreateWidget<MenuItem>("Exit", "ALT + F4").ClickedEvent						+= [] { EDITOR_CONTEXT(window)->SetShouldClose(true); };
}

void Editor::Panels::MenuBar::CreateBuildMenu()
{
	auto& buildMenu = CreateWidget<MenuList>("Build");
	buildMenu.CreateWidget<MenuItem>("Build game").ClickedEvent					+=	EDITOR_BIND(Build, false, false);
	buildMenu.CreateWidget<MenuItem>("Build game and run").ClickedEvent			+=	EDITOR_BIND(Build, true, false);
	buildMenu.CreateWidget<Visual::Separator>();
	buildMenu.CreateWidget<MenuItem>("Temporary build").ClickedEvent			+=	EDITOR_BIND(Build, true, true);
}

void Editor::Panels::MenuBar::CreateWindowMenu()
{
	m_windowMenu = &CreateWidget<MenuList>("Window");
	m_windowMenu->CreateWidget<MenuItem>("Close all").ClickedEvent	+= std::bind(&MenuBar::OpenEveryWindows, this, false);
	m_windowMenu->CreateWidget<MenuItem>("Open all").ClickedEvent		+= std::bind(&MenuBar::OpenEveryWindows, this, true);
	m_windowMenu->CreateWidget<Visual::Separator>();

	/* When the menu is opened, we update which window is marked as "Opened" or "Closed" */
	m_windowMenu->ClickedEvent += std::bind(&MenuBar::UpdateToggleableItems, this);
}

void Editor::Panels::MenuBar::CreateActorsMenu()
{
	auto& actorsMenu = CreateWidget<MenuList>("Actors");
    Utils::ActorCreationMenu::GenerateActorCreationMenu(actorsMenu);
}

void Editor::Panels::MenuBar::CreateResourcesMenu()
{
	auto& resourcesMenu = CreateWidget<MenuList>("Resources");
	resourcesMenu.CreateWidget<MenuItem>("Compile shaders").ClickedEvent += EDITOR_BIND(CompileShaders);
	resourcesMenu.CreateWidget<MenuItem>("Save materials").ClickedEvent += EDITOR_BIND(SaveMaterials);
}

void Editor::Panels::MenuBar::CreateSettingsMenu()
{
	auto& settingsMenu = CreateWidget<MenuList>("Settings");
	settingsMenu.CreateWidget<MenuItem>("Spawn actors at origin", "", true, true).ValueChangedEvent		+= EDITOR_BIND(SetActorSpawnAtOrigin, std::placeholders::_1);
	settingsMenu.CreateWidget<MenuItem>("Vertical Synchronization", "", true, true).ValueChangedEvent	+= [this](bool p_value) { EDITOR_CONTEXT(device)->SetVsync(p_value); };
	auto& cameraSpeedMenu = settingsMenu.CreateWidget<MenuList>("Camera Speed");
	cameraSpeedMenu.CreateWidget<UI::Widgets::Sliders::SliderInt>(1, 50, 15, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Scene View").ValueChangedEvent += EDITOR_BIND(SetSceneViewCameraSpeed, std::placeholders::_1);
	cameraSpeedMenu.CreateWidget<UI::Widgets::Sliders::SliderInt>(1, 50, 15, UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Asset View").ValueChangedEvent += EDITOR_BIND(SetAssetViewCameraSpeed, std::placeholders::_1);
	auto& cameraPositionMenu = settingsMenu.CreateWidget<MenuList>("Reset Camera");
	cameraPositionMenu.CreateWidget<MenuItem>("Scene View").ClickedEvent += EDITOR_BIND(ResetSceneViewCameraPosition);
	cameraPositionMenu.CreateWidget<MenuItem>("Asset View").ClickedEvent += EDITOR_BIND(ResetAssetViewCameraPosition);

	auto& viewColors = settingsMenu.CreateWidget<MenuList>("View Colors");
	auto& sceneViewBackground = viewColors.CreateWidget<MenuList>("Scene View Background");
	auto& sceneViewBackgroundPicker = sceneViewBackground.CreateWidget<Selection::ColorEdit>(false, UI::Types::Color{ 0.098f, 0.098f, 0.098f });
	sceneViewBackgroundPicker.ColorChangedEvent += [this](const auto & color)
	{
		EDITOR_PANEL(Panels::SceneView, "Scene View").GetCamera().SetClearColor({ color.r, color.g, color.b });
	};
	sceneViewBackground.CreateWidget<MenuItem>("Reset").ClickedEvent += [this, &sceneViewBackgroundPicker]
	{
		EDITOR_PANEL(Panels::SceneView, "Scene View").GetCamera().SetClearColor({ 0.098f, 0.098f, 0.098f });
		sceneViewBackgroundPicker.color = { 0.098f, 0.098f, 0.098f };
	};

	auto& sceneViewGrid = viewColors.CreateWidget<MenuList>("Scene View Grid");
    auto& sceneViewGridPicker = sceneViewGrid.CreateWidget<Selection::ColorEdit>(false, UI::Types::Color(0.176f, 0.176f, 0.176f));
	sceneViewGridPicker.ColorChangedEvent += [this](const auto & color)
	{
		EDITOR_PANEL(Panels::SceneView, "Scene View").SetGridColor({ color.r, color.g, color.b });
	};
	sceneViewGrid.CreateWidget<MenuItem>("Reset").ClickedEvent += [this, &sceneViewGridPicker]
	{
		EDITOR_PANEL(Panels::SceneView, "Scene View").SetGridColor(Maths::FVector3(0.176f, 0.176f, 0.176f));
		sceneViewGridPicker.color = UI::Types::Color(0.176f, 0.176f, 0.176f);
	};

	auto& assetViewBackground = viewColors.CreateWidget<MenuList>("Asset View Background");
	auto& assetViewBackgroundPicker = assetViewBackground.CreateWidget<Selection::ColorEdit>(false, UI::Types::Color{ 0.098f, 0.098f, 0.098f });
	assetViewBackgroundPicker.ColorChangedEvent += [this](const auto & color)
	{
		EDITOR_PANEL(Panels::AssetView, "Asset View").GetCamera().SetClearColor({ color.r, color.g, color.b });
	};
	assetViewBackground.CreateWidget<MenuItem>("Reset").ClickedEvent += [this, &assetViewBackgroundPicker]
	{
		EDITOR_PANEL(Panels::AssetView, "Asset View").GetCamera().SetClearColor({ 0.098f, 0.098f, 0.098f });
		assetViewBackgroundPicker.color = { 0.098f, 0.098f, 0.098f };
	};

	auto& assetViewGrid = viewColors.CreateWidget<MenuList>("Asset View Grid");
	auto& assetViewGridPicker = assetViewGrid.CreateWidget<Selection::ColorEdit>(false, UI::Types::Color(0.176f, 0.176f, 0.176f));
	assetViewGridPicker.ColorChangedEvent += [this](const auto & color)
	{
		EDITOR_PANEL(Panels::AssetView, "Asset View").SetGridColor({ color.r, color.g, color.b });
	};
	assetViewGrid.CreateWidget<MenuItem>("Reset").ClickedEvent += [this, &assetViewGridPicker]
	{
		EDITOR_PANEL(Panels::AssetView, "Asset View").SetGridColor(Maths::FVector3(0.176f, 0.176f, 0.176f));
		assetViewGridPicker.color = UI::Types::Color(0.176f, 0.176f, 0.176f);
	};

	auto& sceneViewBillboardScaleMenu = settingsMenu.CreateWidget<MenuList>("3D Icons Scales");
	auto& lightBillboardScaleSlider = sceneViewBillboardScaleMenu.CreateWidget<Sliders::SliderInt>(0, 100, static_cast<int>(Settings::EditorSettings::LightBillboardScale * 100.0f), UI::Widgets::Sliders::ESliderOrientation::HORIZONTAL, "Lights");
	lightBillboardScaleSlider.ValueChangedEvent += [this](int p_value) { Settings::EditorSettings::LightBillboardScale = p_value / 100.0f; };
	lightBillboardScaleSlider.format = "%d %%";

	auto& snappingMenu = settingsMenu.CreateWidget<MenuList>("Snapping");
	snappingMenu.CreateWidget<Drags::DragFloat>(0.001f, 999999.0f, Settings::EditorSettings::TranslationSnapUnit, 0.05f, "Translation Unit").ValueChangedEvent += [this](float p_value) { Settings::EditorSettings::TranslationSnapUnit = p_value; };
	snappingMenu.CreateWidget<Drags::DragFloat>(0.001f, 999999.0f, Settings::EditorSettings::RotationSnapUnit, 1.0f, "Rotation Unit").ValueChangedEvent += [this](float p_value) { Settings::EditorSettings::RotationSnapUnit = p_value; };
	snappingMenu.CreateWidget<Drags::DragFloat>(0.001f, 999999.0f, Settings::EditorSettings::ScalingSnapUnit, 0.05f, "Scaling Unit").ValueChangedEvent += [this](float p_value) { Settings::EditorSettings::ScalingSnapUnit = p_value; };

	auto& debuggingMenu = settingsMenu.CreateWidget<MenuList>("Debugging");
	debuggingMenu.CreateWidget<MenuItem>("Show geometry bounds", "", true, Settings::EditorSettings::ShowGeometryBounds).ValueChangedEvent += [this](bool p_value) { Settings::EditorSettings::ShowGeometryBounds = p_value; };
	debuggingMenu.CreateWidget<MenuItem>("Show lights bounds", "", true, Settings::EditorSettings::ShowLightBounds).ValueChangedEvent += [this](bool p_value) { Settings::EditorSettings::ShowLightBounds = p_value; };
	auto& subMenu = debuggingMenu.CreateWidget<MenuList>("Frustum culling visualizer...");
	subMenu.CreateWidget<MenuItem>("For geometry", "", true, Settings::EditorSettings::ShowGeometryFrustumCullingInSceneView).ValueChangedEvent += [this](bool p_value) { Settings::EditorSettings::ShowGeometryFrustumCullingInSceneView = p_value; };
	subMenu.CreateWidget<MenuItem>("For lights", "", true, Settings::EditorSettings::ShowLightFrustumCullingInSceneView).ValueChangedEvent += [this](bool p_value) { Settings::EditorSettings::ShowLightFrustumCullingInSceneView = p_value; };
}

void Editor::Panels::MenuBar::CreateLayoutMenu() 
{
	auto& layoutMenu = CreateWidget<MenuList>("Layout");
	layoutMenu.CreateWidget<MenuItem>("Reset").ClickedEvent += EDITOR_BIND(ResetLayout);
}

void Editor::Panels::MenuBar::CreateHelpMenu()
{
    auto& helpMenu = CreateWidget<MenuList>("Help");
    helpMenu.CreateWidget<MenuItem>("GitHub").ClickedEvent += [] {::Tools::Utils::SystemCalls::OpenURL("https://github.com/271812697/opengl"); };

}

void Editor::Panels::MenuBar::RegisterPanel(const std::string& p_name, UI::Panels::PanelWindow& p_panel)
{
	auto& menuItem = m_windowMenu->CreateWidget<MenuItem>(p_name, "", true, true);
	menuItem.ValueChangedEvent += std::bind(&UI::Panels::PanelWindow::SetOpened, &p_panel, std::placeholders::_1);

	m_panels.emplace(p_name, std::make_pair(std::ref(p_panel), std::ref(menuItem)));
}

void Editor::Panels::MenuBar::UpdateToggleableItems()
{
	for (auto&[name, panel] : m_panels)
		panel.second.get().checked = panel.first.get().IsOpened();
}

void Editor::Panels::MenuBar::OpenEveryWindows(bool p_state)
{
	for (auto&[name, panel] : m_panels)
		panel.first.get().SetOpened(p_state);
}
