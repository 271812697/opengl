
#include <filesystem>
#include <iostream>
#include <fstream>

#include <Debug/Logger.h>

#include <Core/ECS/Components/CPhysicalBox.h>
#include <Core/ECS/Components/CPhysicalSphere.h>
#include <Core/ECS/Components/CPhysicalCapsule.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CAudioSource.h>

#include <Window/OpenFileDialog.h>
#include <Window/SaveFileDialog.h>
#include <Window/MessageBox.h>

#include <tools/PathParser.h>
#include <tools/String.h>
#include"tools/SystemCallls.h"
#include "Editor/Core/EditorActions.h"
#include "Editor/Panels/SceneView.h"
#include "Editor/Panels/AssetView.h"
#include "Editor/Panels/GameView.h"
#include "Editor/Panels/Inspector.h"
#include "Editor/Panels/ProjectSettings.h"
#include "Editor/Panels/MaterialEditor.h"

Editor::Core::EditorActions::EditorActions(Context& p_context, EditorRenderer& p_editorRenderer, PanelsManager& p_panelsManager) :
	m_context(p_context), 
	m_renderer(p_editorRenderer),
	m_panelsManager(p_panelsManager)
{
	::Core::Global::ServiceLocator::Provide<Editor::Core::EditorActions>(*this);

	m_context.sceneManager.CurrentSceneSourcePathChangedEvent += [this](const std::string& p_newPath)
	{
		std::string titleExtra = " - " + (p_newPath.empty() ? "Untitled Scene" : GetResourcePath(p_newPath));
		m_context.window->SetTitle(m_context.windowSettings.title + titleExtra);
	};
}

void Editor::Core::EditorActions::LoadEmptyScene()
{
	if (GetCurrentEditorMode() != EEditorMode::EDIT)
		StopPlaying();

	m_context.sceneManager.LoadEmptyLightedScene();
	OVLOG_INFO("New scene created");
}

void Editor::Core::EditorActions::SaveCurrentSceneTo(const std::string& p_path)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLNode* node = doc.NewElement("root");
	doc.InsertFirstChild(node);
	m_context.sceneManager.StoreCurrentSceneSourcePath(p_path);
	m_context.sceneManager.GetCurrentScene()->OnSerialize(doc, node);
	doc.SaveFile(p_path.c_str());
}

void Editor::Core::EditorActions::LoadSceneFromDisk(const std::string& p_path, bool p_absolute)
{
	if (GetCurrentEditorMode() != EEditorMode::EDIT)
		StopPlaying();

	m_context.sceneManager.LoadScene(p_path, p_absolute);
	OVLOG_INFO("Scene loaded from disk: " + m_context.sceneManager.GetCurrentSceneSourcePath());
	m_panelsManager.GetPanelAs<Editor::Panels::SceneView>("Scene View").Focus();
}

bool Editor::Core::EditorActions::IsCurrentSceneLoadedFromDisk() const
{
	return m_context.sceneManager.IsCurrentSceneLoadedFromDisk();
}

void Editor::Core::EditorActions::SaveSceneChanges()
{
	if (IsCurrentSceneLoadedFromDisk())
	{
		SaveCurrentSceneTo(m_context.sceneManager.GetCurrentSceneSourcePath());
		OVLOG_INFO("Current scene saved to: " + m_context.sceneManager.GetCurrentSceneSourcePath());
	}
	else
	{
		SaveAs();
	}
}

void Editor::Core::EditorActions::SaveAs()
{
	Windowing::Dialogs::SaveFileDialog dialog("New Scene");
	dialog.SetInitialDirectory(m_context.projectAssetsPath + "New Scene");
	dialog.DefineExtension("Scene", ".scene");
	dialog.Show();

	if (dialog.HasSucceeded())
	{
		if (dialog.IsFileExisting())
		{
			Windowing::Dialogs::MessageBox message("File already exists!", "The file \"" + dialog.GetSelectedFileName() + "\" already exists.\n\nUsing this file as the new home for your scene will erase any content stored in this file.\n\nAre you ok with that?", Windowing::Dialogs::MessageBox::EMessageType::WARNING, Windowing::Dialogs::MessageBox::EButtonLayout::YES_NO, true);
			switch (message.GetUserAction())
			{
			case Windowing::Dialogs::MessageBox::EUserAction::YES: break;
			case Windowing::Dialogs::MessageBox::EUserAction::NO: return;
			}
		}

		SaveCurrentSceneTo(dialog.GetSelectedFilePath());
		OVLOG_INFO("Current scene saved to: " + dialog.GetSelectedFilePath());
	}
}

void Editor::Core::EditorActions::RefreshScripts()
{
	m_context.scriptInterpreter->RefreshAll();
	m_panelsManager.GetPanelAs<Panels::Inspector>("Inspector").Refresh();
	if (m_context.scriptInterpreter->IsOk())
		OVLOG_INFO("Scripts interpretation succeeded!");
}

std::optional<std::string> Editor::Core::EditorActions::SelectBuildFolder()
{
	Windowing::Dialogs::SaveFileDialog dialog("Build location");
	dialog.DefineExtension("Game Build", "..");
	dialog.Show();
	if (dialog.HasSucceeded())
	{
		std::string result = dialog.GetSelectedFilePath();
		result = std::string(result.data(), result.data() + result.size() - std::string("..").size()) + "\\"; // remove auto extension
		if (!std::filesystem::exists(result))
			return result;
		else
		{
			Windowing::Dialogs::MessageBox message("Folder already exists!", "The folder \"" + result + "\" already exists.\n\nPlease select another location and try again", Windowing::Dialogs::MessageBox::EMessageType::WARNING, Windowing::Dialogs::MessageBox::EButtonLayout::OK, true);
			return {};
		}
	}
	else
	{
		return {};
	}
}

void Editor::Core::EditorActions::Build(bool p_autoRun, bool p_tempFolder)
{
	std::string destinationFolder;

	if (p_tempFolder)
	{
		destinationFolder = std::string(getenv("APPDATA")) + "\\erloadTech\\Editor\\TempBuild\\";
		try
		{
			std::filesystem::remove_all(destinationFolder);
		}
		catch (std::filesystem::filesystem_error error)
		{
			Windowing::Dialogs::MessageBox message("Temporary build failed", "The temporary folder is currently being used by another process", Windowing::Dialogs::MessageBox::EMessageType::ERROR, Windowing::Dialogs::MessageBox::EButtonLayout::OK, true);
			return;
		}
	}
	else if (auto res = SelectBuildFolder(); res.has_value())
		destinationFolder = res.value();
	else
		return; // Operation cancelled (No folder selected)

	BuildAtLocation(m_context.projectSettings.Get<bool>("dev_build") ? "Development" : "Shipping", destinationFolder, p_autoRun);
}

void Editor::Core::EditorActions::BuildAtLocation(const std::string & p_configuration, const std::string p_buildPath, bool p_autoRun)
{
	std::string buildPath(p_buildPath);
	std::string executableName = m_context.projectSettings.Get<std::string>("executable_name") + ".exe";

	bool failed = false;

	OVLOG_INFO("Preparing to build at location: \"" + buildPath + "\"");

	std::filesystem::remove_all(buildPath);

	if (std::filesystem::create_directory(buildPath))
	{
		OVLOG_INFO("Build directory created");

		if (std::filesystem::create_directory(buildPath + "Data\\"))
		{
			OVLOG_INFO("Data directory created");

			if (std::filesystem::create_directory(buildPath + "Data\\User\\"))
			{
				OVLOG_INFO("Data\\User directory created");

				std::error_code err;

				std::filesystem::copy(m_context.projectFilePath, buildPath + "Data\\User\\Game.ini", err);

				if (!err)
				{
					OVLOG_INFO("Data\\User\\Game.ini file generated");
		
					std::filesystem::copy(m_context.projectAssetsPath, buildPath + "Data\\User\\Assets\\", std::filesystem::copy_options::recursive, err);

					if (!std::filesystem::exists(buildPath + "Data\\User\\Assets\\" + (m_context.projectSettings.Get<std::string>("start_scene"))))
					{
						OVLOG_ERROR("Failed to find Start Scene at expected path. Verify your Project Setings.");
						Windowing::Dialogs::MessageBox message("Build Failure", "An error occured during the building of your game.\nCheck the console for more information", Windowing::Dialogs::MessageBox::EMessageType::ERROR, Windowing::Dialogs::MessageBox::EButtonLayout::OK, true);
						std::filesystem::remove_all(buildPath);
						return;						
					}

					if (!err)
					{
						OVLOG_INFO("Data\\User\\Assets\\ directory copied");

						std::filesystem::copy(m_context.projectScriptsPath, buildPath + "Data\\User\\Scripts\\", std::filesystem::copy_options::recursive, err);

						if (!err)
						{
							OVLOG_INFO("Data\\User\\Scripts\\ directory copied");

							std::filesystem::copy(m_context.engineAssetsPath, buildPath + "Data\\Engine\\", std::filesystem::copy_options::recursive, err);

							if (!err)
							{
								OVLOG_INFO("Data\\Engine\\ directory copied");
							}
							else
							{
								OVLOG_ERROR("Data\\Engine\\ directory failed to copy");
								failed = true;
							}
						}
						else
						{
							OVLOG_ERROR("Data\\User\\Scripts\\ directory failed to copy");
							failed = true;
						}
					}
					else
					{
						OVLOG_ERROR("Data\\User\\Assets\\ directory failed to copy");
						failed = true;
					}
				}
				else
				{
					OVLOG_ERROR("Data\\User\\Game.ini file failed to generate");
					failed = true;
				}

				std::string builderFolder = "Builder\\" + p_configuration + "\\";

				if (std::filesystem::exists(builderFolder))
				{
					std::error_code err;

					std::filesystem::copy(builderFolder, buildPath, err);

					if (!err)
					{
						OVLOG_INFO("Builder data (Dlls and executatble) copied");

						std::filesystem::rename(buildPath + "Game.exe", buildPath + executableName, err);

						if (!err)
						{
							OVLOG_INFO("Game executable renamed to " + executableName);

							if (p_autoRun)
							{
								std::string exePath = buildPath + executableName;
								OVLOG_INFO("Launching the game at location: \"" + exePath + "\"");
								if (std::filesystem::exists(exePath))
									Tools::Utils::SystemCalls::OpenFile(exePath, buildPath);
								else
								{
									OVLOG_ERROR("Failed to start the game: Executable not found");
									failed = true;
								}
							}
						}
						else
						{
							OVLOG_ERROR("Game executable failed to rename");
							failed = true;
						}
					}
					else
					{
						OVLOG_ERROR("Builder data (Dlls and executatble) failed to copy");
						failed = true;
					}
				}
				else
				{
					const std::string buildConfiguration = p_configuration == "Development" ? "Debug" : "Release";
					OVLOG_ERROR("Builder folder for \"" + p_configuration + "\" not found. Verify you have compiled Engine source code in '" + buildConfiguration + "' configuration.");
					failed = true;
				}
			}
		}
	}
	else
	{
		OVLOG_ERROR("Build directory failed to create");
		failed = true;
	}

	if (failed)
	{
		std::filesystem::remove_all(buildPath);
		Windowing::Dialogs::MessageBox message("Build Failure", "An error occured during the building of your game.\nCheck the console for more information", Windowing::Dialogs::MessageBox::EMessageType::ERROR, Windowing::Dialogs::MessageBox::EButtonLayout::OK, true);
	}
}

void Editor::Core::EditorActions::DelayAction(std::function<void()> p_action, uint32_t p_frames)
{
	m_delayedActions.emplace_back(p_frames + 1, p_action);
}

void Editor::Core::EditorActions::ExecuteDelayedActions()
{
	std::for_each(m_delayedActions.begin(), m_delayedActions.end(), [](std::pair<uint32_t, std::function<void()>> & p_element)
	{
		--p_element.first;

		if (p_element.first == 0)
			p_element.second();
	});

	m_delayedActions.erase(std::remove_if(m_delayedActions.begin(), m_delayedActions.end(), [](std::pair<uint32_t, std::function<void()>> & p_element)
	{
		return p_element.first == 0;
	}), m_delayedActions.end());
}

Editor::Core::Context& Editor::Core::EditorActions::GetContext()
{
	return m_context;
}

Editor::Core::EditorRenderer & Editor::Core::EditorActions::GetRenderer()
{
	return m_renderer;
}

Editor::Core::PanelsManager& Editor::Core::EditorActions::GetPanelsManager()
{
	return m_panelsManager;
}

void Editor::Core::EditorActions::SetActorSpawnAtOrigin(bool p_value)
{
	if (p_value)
		m_actorSpawnMode = EActorSpawnMode::ORIGIN;
	else
		m_actorSpawnMode = EActorSpawnMode::FRONT;
}

void Editor::Core::EditorActions::SetActorSpawnMode(EActorSpawnMode p_value)
{
	m_actorSpawnMode = p_value;
}

void Editor::Core::EditorActions::ResetLayout()
{
    DelayAction([this]() {m_context.uiManager->ResetLayout("Config\\layout.ini"); });
}

void Editor::Core::EditorActions::SetSceneViewCameraSpeed(int p_speed)
{
	EDITOR_PANEL(Panels::SceneView, "Scene View").GetCameraController().SetSpeed((float)p_speed);
}

int Editor::Core::EditorActions::GetSceneViewCameraSpeed()
{
	return (int)EDITOR_PANEL(Panels::SceneView, "Scene View").GetCameraController().GetSpeed();
}

void Editor::Core::EditorActions::SetAssetViewCameraSpeed(int p_speed)
{
	EDITOR_PANEL(Panels::AssetView, "Asset View").GetCameraController().SetSpeed((float)p_speed);
}

int Editor::Core::EditorActions::GetAssetViewCameraSpeed()
{
	return (int)EDITOR_PANEL(Panels::AssetView, "Asset View").GetCameraController().GetSpeed();
}

void Editor::Core::EditorActions::ResetSceneViewCameraPosition()
{
	EDITOR_PANEL(Panels::SceneView, "Scene View").GetCameraController().SetPosition({ -10.0f, 4.0f, 10.0f });
	// TODO
	// EDITOR_PANEL(Panels::SceneView, "Scene View").GetCamera().SetPitch(-10.0f);
	// EDITOR_PANEL(Panels::SceneView, "Scene View").GetCamera().SetYaw(-45.0f);
	// EDITOR_PANEL(Panels::SceneView, "Scene View").GetCamera().SetRoll(0.0f);
}

void Editor::Core::EditorActions::ResetAssetViewCameraPosition()
{
	EDITOR_PANEL(Panels::AssetView, "Asset View").GetCameraController().SetPosition({ -10.0f, 4.0f, 10.0f });
	// TODO
	// EDITOR_PANEL(Panels::AssetView, "Asset View").GetCamera().SetPitch(-10.0f);
	// EDITOR_PANEL(Panels::AssetView, "Asset View").GetCamera().SetYaw(-45.0f);
	// EDITOR_PANEL(Panels::AssetView, "Asset View").GetCamera().SetRoll(0.0f);
}

Editor::Core::EditorActions::EEditorMode Editor::Core::EditorActions::GetCurrentEditorMode() const
{
	return m_editorMode;
}

void Editor::Core::EditorActions::SetEditorMode(EEditorMode p_newEditorMode)
{
	m_editorMode = p_newEditorMode;
	EditorModeChangedEvent.Invoke(m_editorMode);
}

void Editor::Core::EditorActions::StartPlaying()
{
	if (m_editorMode == EEditorMode::EDIT)
	{
		m_context.scriptInterpreter->RefreshAll();
		EDITOR_PANEL(Panels::Inspector, "Inspector").Refresh();

		if (m_context.scriptInterpreter->IsOk())
		{
			PlayEvent.Invoke();
			m_sceneBackup.Clear();
			tinyxml2::XMLNode* node = m_sceneBackup.NewElement("root");
			m_sceneBackup.InsertFirstChild(node);
			m_context.sceneManager.GetCurrentScene()->OnSerialize(m_sceneBackup, node);
			m_panelsManager.GetPanelAs<Editor::Panels::GameView>("Game View").Focus();
			m_context.sceneManager.GetCurrentScene()->Play();
			SetEditorMode(EEditorMode::PLAY);
		}
	}
	else
	{
		m_context.audioEngine->Unsuspend();
		SetEditorMode(EEditorMode::PLAY);
	}
}

void Editor::Core::EditorActions::PauseGame()
{
	SetEditorMode(EEditorMode::PAUSE);
	m_context.audioEngine->Suspend();
}

void Editor::Core::EditorActions::StopPlaying()
{
	if (m_editorMode != EEditorMode::EDIT)
	{
		//ImGui::GetIO().DisableMouseUpdate = false;
       // ImGui::GetIO().WantCaptureMouse = false;
		m_context.window->SetCursorMode(Windowing::Cursor::ECursorMode::NORMAL);
		SetEditorMode(EEditorMode::EDIT);
		bool loadedFromDisk = m_context.sceneManager.IsCurrentSceneLoadedFromDisk();
		std::string sceneSourcePath = m_context.sceneManager.GetCurrentSceneSourcePath();

		int64_t focusedActorID = -1;

		if (auto targetActor = EDITOR_PANEL(Panels::Inspector, "Inspector").GetTargetActor())
			focusedActorID = targetActor->GetID();

		m_context.sceneManager.LoadSceneFromMemory(m_sceneBackup);
		if (loadedFromDisk)
			m_context.sceneManager.StoreCurrentSceneSourcePath(sceneSourcePath); // To bo able to save or reload the scene whereas the scene is loaded from memory (Supposed to have no path)
		m_sceneBackup.Clear();
		EDITOR_PANEL(Panels::SceneView, "Scene View").Focus();
		if (auto actorInstance = m_context.sceneManager.GetCurrentScene()->FindActorByID(focusedActorID))
			EDITOR_PANEL(Panels::Inspector, "Inspector").FocusActor(*actorInstance);
	}
}

void Editor::Core::EditorActions::NextFrame()
{
	if (m_editorMode == EEditorMode::PLAY || m_editorMode == EEditorMode::PAUSE)
		SetEditorMode(EEditorMode::FRAME_BY_FRAME);
}

Maths::FVector3 Editor::Core::EditorActions::CalculateActorSpawnPoint(float p_distanceToCamera)
{
	auto& sceneView = m_panelsManager.GetPanelAs<Editor::Panels::SceneView>("Scene View");
	return sceneView.GetCameraPosition() + sceneView.GetCameraRotation() * Maths::FVector3::Forward * p_distanceToCamera;
}

::Core::ECS::Actor & Editor::Core::EditorActions::CreateEmptyActor(bool p_focusOnCreation, ::Core::ECS::Actor* p_parent, const std::string& p_name)
{
    const auto currentScene = m_context.sceneManager.GetCurrentScene();
	auto& instance = p_name.empty() ? currentScene->CreateActor() : currentScene->CreateActor(p_name);

	if (p_parent)
		instance.SetParent(*p_parent);

	if (m_actorSpawnMode == EActorSpawnMode::FRONT)
		instance.transform.SetLocalPosition(CalculateActorSpawnPoint(10.0f));

	if (p_focusOnCreation)
		SelectActor(instance);

	OVLOG_INFO("Actor created");

	return instance;
}

Core::ECS::Actor & Editor::Core::EditorActions::CreateActorWithModel(const std::string& p_path, bool p_focusOnCreation, ::Core::ECS::Actor* p_parent, const std::string& p_name)
{
	auto& instance = CreateEmptyActor(false, p_parent, p_name);

	auto& modelRenderer = instance.AddComponent<::Core::ECS::Components::CModelRenderer>();

	const auto model = m_context.modelManager[p_path];
	if (model)
		modelRenderer.SetModel(model);

	auto& materialRenderer = instance.AddComponent<::Core::ECS::Components::CMaterialRenderer>();
    const auto material = m_context.materialManager[":Materials\\Default.mat"];
	if (material)
		materialRenderer.FillWithMaterial(*material);

	if (p_focusOnCreation)
		SelectActor(instance);

	return instance;
}

bool Editor::Core::EditorActions::DestroyActor(::Core::ECS::Actor & p_actor)
{
	p_actor.MarkAsDestroy();
	OVLOG_INFO("Actor destroyed");
	return true;
}

std::string FindDuplicatedActorUniqueName(Core::ECS::Actor& p_duplicated, Core::ECS::Actor& p_newActor, Core::SceneSystem::Scene& p_scene)
{
    const auto parent = p_newActor.GetParent();
    const auto adjacentActors = parent ? parent->GetChildren() : p_scene.GetActors();

    auto availabilityChecker = [&parent, &adjacentActors](std::string target) -> bool
    {
        const auto isActorNameTaken = [&target, parent](auto actor) { return (parent || !actor->GetParent()) && actor->GetName() == target; };
        return std::find_if(adjacentActors.begin(), adjacentActors.end(), isActorNameTaken) == adjacentActors.end();
    };

    return Tools::Utils::String::GenerateUnique(p_duplicated.GetName(), availabilityChecker);
}

void Editor::Core::EditorActions::DuplicateActor(::Core::ECS::Actor & p_toDuplicate, ::Core::ECS::Actor* p_forcedParent, bool p_focus)
{
	tinyxml2::XMLDocument doc;
	tinyxml2::XMLNode* actorsRoot = doc.NewElement("actors");
	p_toDuplicate.OnSerialize(doc, actorsRoot);
	auto& newActor = CreateEmptyActor(false);
	int64_t idToUse = newActor.GetID();
	tinyxml2::XMLElement* currentActor = actorsRoot->FirstChildElement("actor");
	newActor.OnDeserialize(doc, currentActor);
	
	newActor.SetID(idToUse);

	if (p_forcedParent)
		newActor.SetParent(*p_forcedParent);
	else
	{
        auto currentScene = m_context.sceneManager.GetCurrentScene();

        if (newActor.GetParentID() > 0)
        {
            if (auto found = currentScene->FindActorByID(newActor.GetParentID()); found)
            {
                newActor.SetParent(*found);
            }
        }

        const auto uniqueName = FindDuplicatedActorUniqueName(p_toDuplicate, newActor, *currentScene);
        newActor.SetName(uniqueName);
	}

	if (p_focus)
		SelectActor(newActor);

	for (auto& child : p_toDuplicate.GetChildren())
		DuplicateActor(*child, &newActor, false);
}

void Editor::Core::EditorActions::SelectActor(::Core::ECS::Actor & p_target)
{
	EDITOR_PANEL(Panels::Inspector, "Inspector").FocusActor(p_target);
}

void Editor::Core::EditorActions::UnselectActor()
{
	EDITOR_PANEL(Panels::Inspector, "Inspector").UnFocus();
}

bool Editor::Core::EditorActions::IsAnyActorSelected() const
{
	return EDITOR_PANEL(Panels::Inspector, "Inspector").GetTargetActor();
}

Core::ECS::Actor & Editor::Core::EditorActions::GetSelectedActor() const
{
	return *EDITOR_PANEL(Panels::Inspector, "Inspector").GetTargetActor();
}

void Editor::Core::EditorActions::MoveToTarget(::Core::ECS::Actor& p_target)
{
	EDITOR_PANEL(Panels::SceneView, "Scene View").GetCameraController().MoveToTarget(p_target);
}

void Editor::Core::EditorActions::CompileShaders()
{
	for (auto shader : m_context.shaderManager.GetResources())
		Rendering::Resources::Loaders::ShaderLoader::Recompile(*shader.second, GetRealPath(shader.second->path));
}

void Editor::Core::EditorActions::SaveMaterials()
{
	for (auto& [id, material] : m_context.materialManager.GetResources())
		::Core::Resources::Loaders::MaterialLoader::Save(*material, GetRealPath(material->path));
}

bool Editor::Core::EditorActions::ImportAsset(const std::string& p_initialDestinationDirectory)
{
	using namespace Windowing::Dialogs;

	std::string modelFormats = "*.fbx;*.obj;";
	std::string textureFormats = "*.png;*.jpeg;*.jpg;*.tga";
	std::string shaderFormats = "*.glsl;";
	std::string soundFormats = "*.mp3;*.ogg;*.wav;";

	OpenFileDialog selectAssetDialog("Select an asset to import");
	selectAssetDialog.AddFileType("Any supported format", modelFormats + textureFormats + shaderFormats + soundFormats);
	selectAssetDialog.AddFileType("Model (.fbx, .obj)", modelFormats);
	selectAssetDialog.AddFileType("Texture (.png, .jpeg, .jpg, .tga)", textureFormats);
	selectAssetDialog.AddFileType("Shader (.glsl)", shaderFormats);
	selectAssetDialog.AddFileType("Sound (.mp3, .ogg, .wav)", soundFormats);
	selectAssetDialog.Show();

	if (selectAssetDialog.HasSucceeded())
	{
		std::string source = selectAssetDialog.GetSelectedFilePath();
		std::string extension = '.' + Tools::Utils::PathParser::GetExtension(source);
		std::string filename = selectAssetDialog.GetSelectedFileName();

		SaveFileDialog saveLocationDialog("Where to import?");
		saveLocationDialog.SetInitialDirectory(p_initialDestinationDirectory + filename);
		saveLocationDialog.DefineExtension(extension, extension);
		saveLocationDialog.Show();

		if (saveLocationDialog.HasSucceeded())
		{
			std::string destination = saveLocationDialog.GetSelectedFilePath();

			if (!std::filesystem::exists(destination) || MessageBox("File already exists", "The destination you have selected already exists, importing this file will erase the previous file content, are you sure about that?", MessageBox::EMessageType::WARNING, MessageBox::EButtonLayout::OK_CANCEL).GetUserAction() == MessageBox::EUserAction::OK)
			{
				std::filesystem::copy(source, destination, std::filesystem::copy_options::overwrite_existing);
				OVLOG_INFO("Asset \"" + destination + "\" imported");
				return true;
			}
		}
	}

	return false;
}

bool Editor::Core::EditorActions::ImportAssetAtLocation(const std::string& p_destination)
{
	using namespace Windowing::Dialogs;

	std::string modelFormats = "*.fbx;*.obj;";
	std::string textureFormats = "*.png;*.jpeg;*.jpg;*.tga;";
	std::string shaderFormats = "*.glsl;";
	std::string soundFormats = "*.mp3;*.ogg;*.wav;";

	OpenFileDialog selectAssetDialog("Select an asset to import");
	selectAssetDialog.AddFileType("Any supported format", modelFormats + textureFormats + shaderFormats + soundFormats);
	selectAssetDialog.AddFileType("Model (.fbx, .obj)", modelFormats);
	selectAssetDialog.AddFileType("Texture (.png, .jpeg, .jpg, .tga)", textureFormats);
	selectAssetDialog.AddFileType("Shader (.glsl)", shaderFormats);
	selectAssetDialog.AddFileType("Sound (.mp3, .ogg, .wav)", soundFormats);
	selectAssetDialog.Show();

	if (selectAssetDialog.HasSucceeded())
	{
		std::string source = selectAssetDialog.GetSelectedFilePath();
		std::string destination = p_destination + selectAssetDialog.GetSelectedFileName();

		if (!std::filesystem::exists(destination) || MessageBox("File already exists", "The destination you have selected already exists, importing this file will erase the previous file content, are you sure about that?", MessageBox::EMessageType::WARNING, MessageBox::EButtonLayout::OK_CANCEL).GetUserAction() == MessageBox::EUserAction::OK)
		{
			std::filesystem::copy(source, destination, std::filesystem::copy_options::overwrite_existing);
			OVLOG_INFO("Asset \"" + destination + "\" imported");
			return true;
		}
	}

	return false;
}

// Duplicate from AResourceManager.h
std::string Editor::Core::EditorActions::GetRealPath(const std::string & p_path)
{
	std::string result;

	if (p_path[0] == ':') // The path is an engine path
	{
		result = m_context.engineAssetsPath + std::string(p_path.data() + 1, p_path.data() + p_path.size());
	}
	else // The path is a project path
	{
		result = m_context.projectAssetsPath + p_path;
	}

	return result;
}

std::string Editor::Core::EditorActions::GetResourcePath(const std::string & p_path, bool p_isFromEngine)
{
	std::string result = p_path;

	if (Tools::Utils::String::Replace(result, p_isFromEngine ? m_context.engineAssetsPath : m_context.projectAssetsPath, ""))
	{
		if (p_isFromEngine)
			result = ':' + result;
	}

	return result;
}

std::string Editor::Core::EditorActions::GetScriptPath(const std::string & p_path)
{
	std::string result = p_path;

	Tools::Utils::String::Replace(result, m_context.projectScriptsPath, "");
	Tools::Utils::String::Replace(result, ".lua", "");

	return result;
}

void Editor::Core::EditorActions::PropagateFolderRename(std::string p_previousName, std::string p_newName)
{
	p_previousName = Tools::Utils::PathParser::MakeNonWindowsStyle(p_previousName);
	p_newName = Tools::Utils::PathParser::MakeNonWindowsStyle(p_newName);

	for (auto& p : std::filesystem::recursive_directory_iterator(p_newName))
	{
		if (!p.is_directory())
		{
			std::string newFileName = Tools::Utils::PathParser::MakeNonWindowsStyle(p.path().string());
			std::string previousFileName;

			for (char c : newFileName)
			{
				previousFileName += c;
				if (previousFileName == p_newName)
					previousFileName = p_previousName;
			}

			PropagateFileRename(Tools::Utils::PathParser::MakeWindowsStyle(previousFileName), Tools::Utils::PathParser::MakeWindowsStyle(newFileName));
		}
	}
}

void Editor::Core::EditorActions::PropagateFolderDestruction(std::string p_folderPath)
{
	for (auto& p : std::filesystem::recursive_directory_iterator(p_folderPath))
	{
		if (!p.is_directory())
		{
			PropagateFileRename(Tools::Utils::PathParser::MakeWindowsStyle(p.path().string()), "?");
		}
	}
}

void Editor::Core::EditorActions::PropagateScriptRename(std::string p_previousName, std::string p_newName)
{
	p_previousName = GetScriptPath(p_previousName);
	p_newName = GetScriptPath(p_newName);

	if (auto currentScene = m_context.sceneManager.GetCurrentScene())
		for (auto actor : currentScene->GetActors())
			if (actor->RemoveBehaviour(p_previousName))
				actor->AddBehaviour(p_newName);

	PropagateFileRenameThroughSavedFilesOfType(p_previousName, p_newName, Tools::Utils::PathParser::EFileType::SCENE);

	EDITOR_PANEL(Panels::Inspector, "Inspector").Refresh();
}

void Editor::Core::EditorActions::PropagateFileRename(std::string p_previousName, std::string p_newName)
{
	p_previousName = GetResourcePath(p_previousName);
	p_newName = GetResourcePath(p_newName);

	if (p_newName != "?")
	{
		/* If not a real rename is asked (Not delete) */

		if (::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>().MoveResource(p_previousName, p_newName))
		{
			Rendering::Resources::Model* resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>()[p_newName];
			*reinterpret_cast<std::string*>(reinterpret_cast<char*>(resource) + offsetof(Rendering::Resources::Model, path)) = p_newName;
		}

		if (::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::TextureManager>().MoveResource(p_previousName, p_newName))
		{
			Rendering::Resources::Texture* resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::TextureManager>()[p_newName];
			*reinterpret_cast<std::string*>(reinterpret_cast<char*>(resource) + offsetof(Rendering::Resources::Texture, path)) = p_newName;
		}

		if (::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ShaderManager>().MoveResource(p_previousName, p_newName))
		{
			Rendering::Resources::Shader* resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ShaderManager>()[p_newName];
			*reinterpret_cast<std::string*>(reinterpret_cast<char*>(resource) + offsetof(Rendering::Resources::Shader, path)) = p_newName;
		}

		if (::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>().MoveResource(p_previousName, p_newName))
		{
			::Core::Resources::Material* resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>()[p_newName];
			*reinterpret_cast<std::string*>(reinterpret_cast<char*>(resource) + offsetof(::Core::Resources::Material, path)) = p_newName;
		}

		if (::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::SoundManager>().MoveResource(p_previousName, p_newName))
		{
			Audio::Resources::Sound* resource = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::SoundManager>()[p_newName];
			*reinterpret_cast<std::string*>(reinterpret_cast<char*>(resource) + offsetof(Audio::Resources::Sound, path)) = p_newName;
		}
	}
	else
	{
		if (auto texture = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::TextureManager>().GetResource(p_previousName, false))
		{
			for (auto[name, instance] : ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>().GetResources())
				if (instance)
					for (auto&[name, value] : instance->GetUniformsData())
						if (value.has_value() && value.type() == typeid(Rendering::Resources::Texture*))
							if (std::any_cast<Rendering::Resources::Texture*>(value) == texture)
								value = static_cast<Rendering::Resources::Texture*>(nullptr);

			auto& assetView = EDITOR_PANEL(Panels::AssetView, "Asset View");
			auto assetViewRes = assetView.GetResource();
			if (auto pval = std::get_if<Rendering::Resources::Texture*>(&assetViewRes); pval && *pval)
				assetView.SetResource(static_cast<Rendering::Resources::Texture*>(nullptr));

			::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::TextureManager>().UnloadResource(p_previousName);
		}
		
		if (auto shader = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ShaderManager>().GetResource(p_previousName, false))
		{
			auto& materialEditor = EDITOR_PANEL(Panels::MaterialEditor, "Material Editor");

			for (auto[name, instance] : ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>().GetResources())
				if (instance && instance->GetShader() == shader)
					instance->SetShader(nullptr);

			::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ShaderManager>().UnloadResource(p_previousName);
		}

		if (auto model = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>().GetResource(p_previousName, false))
		{
			auto& assetView = EDITOR_PANEL(Panels::AssetView, "Asset View");
			auto assetViewRes = assetView.GetResource();
			if (auto pval = std::get_if<Rendering::Resources::Model*>(&assetViewRes); pval && *pval)
				assetView.SetResource(static_cast<Rendering::Resources::Model*>(nullptr));

			if (auto currentScene = m_context.sceneManager.GetCurrentScene())
				for (auto actor : currentScene->GetActors())
					if (auto modelRenderer = actor->GetComponent<::Core::ECS::Components::CModelRenderer>(); modelRenderer && modelRenderer->GetModel() == model)
						modelRenderer->SetModel(nullptr);

			::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>().UnloadResource(p_previousName);
		}

		if (auto material = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>().GetResource(p_previousName, false))
		{
			auto& materialEditor = EDITOR_PANEL(Panels::MaterialEditor, "Material Editor");
			
			if (materialEditor.GetTarget() == material)
				materialEditor.RemoveTarget();

			auto& assetView = EDITOR_PANEL(Panels::AssetView, "Asset View");
			auto assetViewRes = assetView.GetResource();
			if (auto pval = std::get_if<::Core::Resources::Material*>(&assetViewRes); pval && *pval)
				assetView.SetResource(static_cast<::Core::Resources::Material*>(nullptr));

			if (auto currentScene = m_context.sceneManager.GetCurrentScene())
				for (auto actor : currentScene->GetActors())
					if (auto materialRenderer = actor->GetComponent<::Core::ECS::Components::CMaterialRenderer>(); materialRenderer)
						materialRenderer->RemoveMaterialByInstance(*material);

			::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::MaterialManager>().UnloadResource(p_previousName);
		}

		if (auto sound = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::SoundManager>().GetResource(p_previousName, false))
		{
			if (auto currentScene = m_context.sceneManager.GetCurrentScene())
				for (auto actor : currentScene->GetActors())
					if (auto audioSource = actor->GetComponent<::Core::ECS::Components::CAudioSource>(); audioSource && audioSource->GetSound() == sound)
						audioSource->SetSound(nullptr);

			::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::SoundManager>().UnloadResource(p_previousName);
		}
	}

	switch (Tools::Utils::PathParser::GetFileType(p_previousName))
	{
	case Tools::Utils::PathParser::EFileType::MATERIAL:
		PropagateFileRenameThroughSavedFilesOfType(p_previousName, p_newName, Tools::Utils::PathParser::EFileType::SCENE);
		break;
	case Tools::Utils::PathParser::EFileType::MODEL:
		PropagateFileRenameThroughSavedFilesOfType(p_previousName, p_newName, Tools::Utils::PathParser::EFileType::SCENE);
		break;
	case Tools::Utils::PathParser::EFileType::SHADER:
		PropagateFileRenameThroughSavedFilesOfType(p_previousName, p_newName, Tools::Utils::PathParser::EFileType::MATERIAL);
		break;
	case Tools::Utils::PathParser::EFileType::TEXTURE:
		PropagateFileRenameThroughSavedFilesOfType(p_previousName, p_newName, Tools::Utils::PathParser::EFileType::MATERIAL);
		break;
	case Tools::Utils::PathParser::EFileType::SOUND:
		PropagateFileRenameThroughSavedFilesOfType(p_previousName, p_newName, Tools::Utils::PathParser::EFileType::SCENE);
		break;
	}

	EDITOR_PANEL(Panels::Inspector, "Inspector").Refresh();
	EDITOR_PANEL(Panels::MaterialEditor, "Material Editor").Refresh();
}

void Editor::Core::EditorActions::PropagateFileRenameThroughSavedFilesOfType(const std::string& p_previousName, const std::string& p_newName, Tools::Utils::PathParser::EFileType p_fileType)
{
	for (auto& entry : std::filesystem::recursive_directory_iterator(m_context.projectAssetsPath))
	{
		if (Tools::Utils::PathParser::GetFileType(entry.path().string()) == p_fileType)
		{
			using namespace std;

			{
				ifstream in(entry.path().string().c_str());
				ofstream out("TEMP");
				string wordToReplace(">" + p_previousName + "<");
				string wordToReplaceWith(">" + p_newName + "<");

				string line;
				size_t len = wordToReplace.length();
				while (getline(in, line))
				{
					if (Tools::Utils::String::Replace(line, wordToReplace, wordToReplaceWith))
						OVLOG_INFO("Asset retargeting: \"" + p_previousName + "\" to \"" + p_newName + "\" in \"" + entry.path().string() + "\"");
					out << line << '\n';
				}

				out.close(); in.close();
			}

			std::filesystem::copy_file("TEMP", entry.path(), std::filesystem::copy_options::overwrite_existing);
			std::filesystem::remove("TEMP");
		}
	}
}
