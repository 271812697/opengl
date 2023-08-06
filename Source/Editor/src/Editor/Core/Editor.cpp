
#include <Analytics/Profiling/ProfilerSpy.h>
#include <Physics/Core/PhysicsEngine.h>

#include "Editor/Core/Editor.h"
#include "Editor/Panels/MenuBar.h"
#include "Editor/Panels/AssetBrowser.h"
#include "Editor/Panels/HardwareInfo.h"
#include "Editor/Panels/Profiler.h"
#include "Editor/Panels/Console.h"
#include "Editor/Panels/Inspector.h"
#include "Editor/Panels/Hierarchy.h"
#include "Editor/Panels/SceneView.h"
#include "Editor/Panels/GameView.h"
#include "Editor/Panels/AssetView.h"
#include "Editor/Panels/Toolbar.h"
#include "Editor/Panels/MaterialEditor.h"
#include "Editor/Panels/ProjectSettings.h"
#include "Editor/Panels/AssetProperties.h"

using namespace Core::ResourceManagement;
using namespace Editor::Panels;
using namespace Rendering::Resources::Loaders;
using namespace Rendering::Resources::Parsers;

Editor::Core::Editor::Editor(Context& p_context) : 
	m_context(p_context),
	m_editorRenderer(p_context),
	m_panelsManager(m_canvas),
	m_editorActions(m_context, m_editorRenderer, m_panelsManager)
{
	SetupUI();

	m_context.sceneManager.LoadEmptyLightedScene();
}

Editor::Core::Editor::~Editor()
{
	m_context.sceneManager.UnloadCurrentScene();
}

void Editor::Core::Editor::SetupUI()
{
	UI::Settings::PanelWindowSettings settings;
	settings.closable = true;
	settings.collapsable = true;
	settings.dockable = true;

	m_panelsManager.CreatePanel<::Editor::Panels::MenuBar>("Menu Bar");
	m_panelsManager.CreatePanel<::Editor::Panels::AssetBrowser>("Asset Browser", true, settings, m_context.engineAssetsPath, m_context.projectAssetsPath, m_context.projectScriptsPath);
	m_panelsManager.CreatePanel<::Editor::Panels::HardwareInfo>("Hardware Info", false, settings, 0.2f, 50);
	m_panelsManager.CreatePanel<::Editor::Panels::Profiler>("Profiler", true, settings, 0.25f);
	m_panelsManager.CreatePanel<::Editor::Panels::Console>("Console", true, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::Hierarchy>("Hierarchy", true, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::Inspector>("Inspector", true, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::SceneView>("Scene View", true, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::GameView>("Game View", true, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::AssetView>("Asset View", false, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::Toolbar>("Toolbar", true, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::MaterialEditor>("Material Editor", false, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::ProjectSettings>("Project Settings", false, settings);
	m_panelsManager.CreatePanel<::Editor::Panels::AssetProperties>("Asset Properties", false, settings);

	m_canvas.MakeDockspace(true);
	m_context.uiManager->SetCanvas(m_canvas);
}

void Editor::Core::Editor::PreUpdate()
{
	PROFILER_SPY("Editor Pre-Update");

	m_context.device->PollEvents();
	m_context.renderer->SetClearColor(0.f, 0.f, 0.f);
	m_context.renderer->Clear();
}

void Editor::Core::Editor::Update(float p_deltaTime)
{   
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
	HandleGlobalShortcuts();
	UpdateCurrentEditorMode(p_deltaTime);
	PrepareRendering(p_deltaTime);
	UpdateEditorPanels(p_deltaTime);
	RenderViews(p_deltaTime);   
	RenderEditorUI(p_deltaTime);
	ImGui::Render();
	m_editorActions.ExecuteDelayedActions();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
  

}

void Editor::Core::Editor::HandleGlobalShortcuts()
{
	// If the [Del] key is pressed while an actor is selected and the Scene View or Hierarchy is focused
	if (m_context.inputManager->IsKeyPressed(Windowing::Inputs::EKey::KEY_DELETE) && EDITOR_EXEC(IsAnyActorSelected()) && (EDITOR_PANEL(SceneView, "Scene View").IsFocused() || EDITOR_PANEL(Hierarchy, "Hierarchy").IsFocused()))
	{
		EDITOR_EXEC(DestroyActor(EDITOR_EXEC(GetSelectedActor())));
       
	}
}

void Editor::Core::Editor::UpdateCurrentEditorMode(float p_deltaTime)
{
	if (auto editorMode = m_editorActions.GetCurrentEditorMode(); editorMode == EditorActions::EEditorMode::PLAY || editorMode == EditorActions::EEditorMode::FRAME_BY_FRAME)
		UpdatePlayMode(p_deltaTime);
	else
		UpdateEditMode(p_deltaTime);

	{
		PROFILER_SPY("Scene garbage collection");
		m_context.sceneManager.GetCurrentScene()->CollectGarbages();
		m_context.sceneManager.Update();
	}
}

void Editor::Core::Editor::UpdatePlayMode(float p_deltaTime)
{
	auto currentScene = m_context.sceneManager.GetCurrentScene();
	bool simulationApplied = false;

	{
		PROFILER_SPY("Physics Update");
		simulationApplied = m_context.physicsEngine->Update(p_deltaTime);
	}

	if (simulationApplied)
	{
		PROFILER_SPY("FixedUpdate");
		currentScene->FixedUpdate(p_deltaTime);
	}

	{
		PROFILER_SPY("Update");
		currentScene->Update(p_deltaTime);
	}

	{
		PROFILER_SPY("LateUpdate");
		currentScene->LateUpdate(p_deltaTime);
	}

	{
		PROFILER_SPY("Audio Update");
		m_context.audioEngine->Update();
	}

    //ImGui::GetIO().WantCaptureMouse = (m_context.window->GetCursorMode() == Windowing::Cursor::ECursorMode::DISABLED);

	if (m_editorActions.GetCurrentEditorMode() == EditorActions::EEditorMode::FRAME_BY_FRAME)
		m_editorActions.PauseGame();

	if (m_context.inputManager->IsKeyPressed(Windowing::Inputs::EKey::KEY_ESCAPE))
		m_editorActions.StopPlaying();
}

void Editor::Core::Editor::UpdateEditMode(float p_deltaTime)
{
	if (m_context.inputManager->IsKeyPressed(Windowing::Inputs::EKey::KEY_F5))
		m_editorActions.StartPlaying();
}

void Editor::Core::Editor::UpdateEditorPanels(float p_deltaTime)
{
	auto& menuBar = m_panelsManager.GetPanelAs<::Editor::Panels::MenuBar>("Menu Bar");
	auto& profiler = m_panelsManager.GetPanelAs<::Editor::Panels::Profiler>("Profiler");
	auto& hardwareInfo = m_panelsManager.GetPanelAs<::Editor::Panels::HardwareInfo>("Hardware Info");
	auto& sceneView = m_panelsManager.GetPanelAs<::Editor::Panels::SceneView>("Scene View");

	menuBar.HandleShortcuts(p_deltaTime);

	if (m_elapsedFrames == 1) // Let the first frame happen and then make the scene view the first seen view
		sceneView.Focus();

	if (profiler.IsOpened())
	{
		PROFILER_SPY("Profiler Update");
		profiler.Update(p_deltaTime);
	}

	if (hardwareInfo.IsOpened())
	{
		PROFILER_SPY("Hardware Info Update");
		hardwareInfo.Update(p_deltaTime);
	}
}

void Editor::Core::Editor::PrepareRendering(float p_deltaTime)
{
	PROFILER_SPY("Engine UBO Update");
	m_context.engineUBO->SetSubData(m_context.device->GetElapsedTime(), 3 * sizeof(Maths::FMatrix4) + sizeof(Maths::FVector3));
}

void Editor::Core::Editor::RenderViews(float p_deltaTime)
{
	auto& assetView = m_panelsManager.GetPanelAs<::Editor::Panels::AssetView>("Asset View");
	auto& sceneView = m_panelsManager.GetPanelAs<::Editor::Panels::SceneView>("Scene View");
	auto& gameView = m_panelsManager.GetPanelAs<::Editor::Panels::GameView>("Game View");

	{
		PROFILER_SPY("Editor Views Update");

		assetView.Update(p_deltaTime);
		gameView.Update(p_deltaTime);
		sceneView.Update(p_deltaTime);
	}

	if (assetView.IsOpened())
	{
		PROFILER_SPY("Asset View Rendering");

		m_context.simulatedLightSSBO->Bind(0);
		assetView.Render();
		m_context.simulatedLightSSBO->Unbind();
	}

	m_context.lightSSBO->Bind(0);

	if (gameView.IsOpened())
	{
		PROFILER_SPY("Game View Rendering");

		gameView.Render();
	}

	if (sceneView.IsOpened())
	{
		PROFILER_SPY("Scene View Rendering");

		sceneView.Render();
	}

	m_context.lightSSBO->Unbind();
}

void Editor::Core::Editor::RenderEditorUI(float p_deltaTime)
{

	PROFILER_SPY("Editor UI Rendering");

    m_editorRenderer.RenderUI();
}

void Editor::Core::Editor::PostUpdate()
{
	PROFILER_SPY("Editor Post-Update");

	m_context.window->SwapBuffers();
	m_context.inputManager->ClearEvents();
	++m_elapsedFrames;
}