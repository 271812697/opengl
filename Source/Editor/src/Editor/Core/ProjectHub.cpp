/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#define _CRT_SECURE_NO_WARNINGS

#include <filesystem>
#include <fstream>

#include "Editor/Core/ProjectHub.h"

#include <UI/Widgets/Texts/Text.h>
#include <UI/Widgets/Visual/Separator.h>
#include <UI/Widgets/Layout/Columns.h>
#include <UI/Widgets/Layout/Spacing.h>
#include <UI/Widgets/Layout/Group.h>
#include <UI/Widgets/Buttons/Button.h>
#include <UI/Widgets/InputFields/InputText.h>

#include"tools/PathParser.h"

#include <Window/SaveFileDialog.h>
#include <Window/OpenFileDialog.h>
#include <Window/MessageBox.h>

#define PROJECTS_FILE std::string(std::string(getenv("APPDATA")) + "\\erloadTech\\Editor\\projects.ini")

class ProjectHubPanel : public UI::Panels::PanelWindow
{
public:
	ProjectHubPanel(bool& p_readyToGo, std::string& p_path, std::string& p_projectName) :
		PanelWindow("Project Hub", true),
		m_readyToGo(p_readyToGo),
		m_path(p_path),
		m_projectName(p_projectName)
	{
		resizable = false;
		movable = false;
		titleBar = false;

		std::filesystem::create_directories(std::string(getenv("APPDATA")) + "\\erloadTech\\Editor\\");
		SetSize({ 1000, 580 });
		SetPosition({ 0.f, 0.f });

		auto& openProjectButton = CreateWidget<UI::Widgets::Buttons::Button>("Open Project");
		auto& newProjectButton = CreateWidget<UI::Widgets::Buttons::Button>("New Project");
		auto& pathField = CreateWidget<UI::Widgets::InputFields::InputText>("");
		m_goButton = &CreateWidget<UI::Widgets::Buttons::Button>("GO");

		pathField.ContentChangedEvent += [this, &pathField](std::string p_content)
		{
			pathField.content = ::Tools::Utils::PathParser::MakeWindowsStyle(p_content);

			if (pathField.content != "" && pathField.content.back() != '\\')
				pathField.content += '\\';

			UpdateGoButton(pathField.content);
		};

		UpdateGoButton("");

		openProjectButton.idleBackgroundColor = { 0.7f, 0.5f, 0.f };
		newProjectButton.idleBackgroundColor = { 0.f, 0.5f, 0.0f };

		openProjectButton.ClickedEvent += [this]
		{
			Windowing::Dialogs::OpenFileDialog dialog("Open project");
			dialog.AddFileType("erload Project", "*.ovproject");
			dialog.Show();

			std::string ovProjectPath = dialog.GetSelectedFilePath();
			std::string rootFolderPath = Tools::Utils::PathParser::GetContainingFolder(ovProjectPath);

			if (dialog.HasSucceeded())
			{
				RegisterProject(rootFolderPath);
				OpenProject(rootFolderPath);
			}
		};

		newProjectButton.ClickedEvent += [this, &pathField]
		{
			Windowing::Dialogs::SaveFileDialog dialog("New project location");
			dialog.DefineExtension("erload Project", "..");
			dialog.Show();
			if (dialog.HasSucceeded())
			{
				std::string result = dialog.GetSelectedFilePath();
				pathField.content = std::string(result.data(), result.data() + result.size() - std::string("..").size()); // remove auto extension
				pathField.content += "\\";
				UpdateGoButton(pathField.content);
			}
		};

		m_goButton->ClickedEvent += [this, &pathField]
		{
			CreateProject(pathField.content);
			RegisterProject(pathField.content);
			OpenProject(pathField.content);
		};

		openProjectButton.lineBreak = false;
		newProjectButton.lineBreak = false;
		pathField.lineBreak = false;

		for (uint8_t i = 0; i < 4; ++i)
			CreateWidget<UI::Widgets::Layout::Spacing>();

		CreateWidget<UI::Widgets::Visual::Separator>();

		for (uint8_t i = 0; i < 4; ++i)
			CreateWidget<UI::Widgets::Layout::Spacing>();

		auto& columns = CreateWidget<UI::Widgets::Layout::Columns<2>>();

		columns.widths = { 750, 500 };

		std::string line;
		std::ifstream myfile(PROJECTS_FILE);
		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				if (std::filesystem::exists(line)) // TODO: Delete line from the file
				{
					auto& text = columns.CreateWidget<UI::Widgets::Texts::Text>(line);
					auto& actions = columns.CreateWidget<UI::Widgets::Layout::Group>();
					auto& openButton = actions.CreateWidget<UI::Widgets::Buttons::Button>("Open");
					auto& deleteButton = actions.CreateWidget<UI::Widgets::Buttons::Button>("Delete");

					openButton.idleBackgroundColor = { 0.7f, 0.5f, 0.f };
					deleteButton.idleBackgroundColor = { 0.5f, 0.f, 0.f };

					openButton.ClickedEvent += [this, line]
					{
						OpenProject(line);
					};

					std::string toErase = line;
					deleteButton.ClickedEvent += [this, &text, &actions, toErase]
					{
						text.Destroy();
						actions.Destroy();

						std::string line;
						std::ifstream fin(PROJECTS_FILE);
						std::ofstream temp("temp");

						while (getline(fin, line))
							if (line != toErase)
								temp << line << std::endl;

						temp.close();
						fin.close();

						std::filesystem::remove(PROJECTS_FILE);
						std::filesystem::rename("temp", PROJECTS_FILE);
					};

					openButton.lineBreak = false;
					deleteButton.lineBreak;
				}
			}
			myfile.close();
		}
	}

	void UpdateGoButton(const std::string& p_path)
	{
		bool validPath = p_path != "";
		m_goButton->idleBackgroundColor = validPath ? UI::Types::Color{ 0.f, 0.5f, 0.0f } : UI::Types::Color{ 0.1f, 0.1f, 0.1f };
		m_goButton->disabled = !validPath;
	}

	void CreateProject(const std::string& p_path)
	{
		if (!std::filesystem::exists(p_path))
		{
			std::filesystem::create_directory(p_path);
			std::filesystem::create_directory(p_path + "Assets\\");
			std::filesystem::create_directory(p_path + "Scripts\\");
			std::ofstream projectFile(p_path + '\\' + Tools::Utils::PathParser::GetElementName(std::string(p_path.data(), p_path.data() + p_path.size() - 1)) + ".ovproject");
		}
	}

	void RegisterProject(const std::string& p_path)
	{
		bool pathAlreadyRegistered = false;

		{
			std::string line;
			std::ifstream myfile(PROJECTS_FILE);
			if (myfile.is_open())
			{
				while (getline(myfile, line))
				{
					if (line == p_path)
					{
						pathAlreadyRegistered = true;
						break;
					}
				}
				myfile.close();
			}
		}

		if (!pathAlreadyRegistered)
		{
			std::ofstream projectsFile(PROJECTS_FILE, std::ios::app);
			projectsFile << p_path << std::endl;
		}
	}
	
	void OpenProject(const std::string& p_path)
	{
		m_readyToGo = std::filesystem::exists(p_path);
		if (!m_readyToGo)
		{
			using namespace Windowing::Dialogs;
			MessageBox errorMessage("Project not found", "The selected project does not exists", MessageBox::EMessageType::ERROR, MessageBox::EButtonLayout::OK);
		}
		else
		{
			m_path = p_path;
			m_projectName = Tools::Utils::PathParser::GetElementName(m_path);
			Close();
		}
	}

	void Draw() override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 50.f, 50.f });
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);

		UI::Panels::PanelWindow::Draw();

		ImGui::PopStyleVar(2);
	}

private:
	bool& m_readyToGo;
	std::string& m_path;
	std::string& m_projectName;
	UI::Widgets::Buttons::Button* m_goButton = nullptr;
};

Editor::Core::ProjectHub::ProjectHub()
{
	SetupContext();
	m_mainPanel = std::make_unique<ProjectHubPanel>(m_readyToGo, m_projectPath, m_projectName);

	m_uiManager->SetCanvas(m_canvas);
	m_canvas.AddPanel(*m_mainPanel);
}

std::tuple<bool, std::string, std::string> Editor::Core::ProjectHub::Run()
{
	m_renderer->SetClearColor(0.f, 0.f, 0.f, 1.f);

	while (!m_window->ShouldClose())
	{
		m_renderer->Clear();
		m_device->PollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		m_uiManager->Render();
		ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
       
		m_window->SwapBuffers();

		if (!m_mainPanel->IsOpened())
			m_window->SetShouldClose(true);
	}

	return { m_readyToGo, m_projectPath, m_projectName };
}

void Editor::Core::ProjectHub::SetupContext()
{
	/* Settings */
	Windowing::Settings::DeviceSettings deviceSettings;
	Windowing::Settings::WindowSettings windowSettings;
	windowSettings.title = "Project Hub";
	windowSettings.width = 1000;
	windowSettings.height = 580;
	windowSettings.maximized = false;
	windowSettings.resizable = false;
	windowSettings.decorated = true;

	/* Window creation */
	m_device = std::make_unique<Windowing::Context::Device>(deviceSettings);
	m_window = std::make_unique<Windowing::Window>(*m_device, windowSettings);
	m_window->MakeCurrentContext();
    /* center the window */
	auto[monWidth, monHeight] = m_device->GetMonitorSize();
	auto[winWidth, winHeight] = m_window->GetSize();
	m_window->SetPosition(monWidth / 2 - winWidth / 2, monHeight / 2 - winHeight / 2);
    

	/* Graphics context creation */
	m_driver = std::make_unique<Rendering::Context::Driver>(Rendering::Settings::DriverSettings{ false });
	m_renderer = std::make_unique<Rendering::Core::Renderer>(*m_driver);
	m_renderer->SetCapability(Rendering::Settings::ERenderingCapability::MULTISAMPLE, true);

	m_uiManager = std::make_unique<UI::Core::UIManager>(m_window->GetGlfwWindow(), UI::Styling::EStyle::CUSTOM);
	m_uiManager->LoadFont("Ruda_Big", "res\\Data\\Editor\\Fonts\\Ruda-Bold.ttf", 18);
	m_uiManager->UseFont("Ruda_Big");
	m_uiManager->EnableEditorLayoutSave(false);
	m_uiManager->EnableDocking(false);
}

void Editor::Core::ProjectHub::RegisterProject(const std::string& p_path)
{
	static_cast<ProjectHubPanel*>(m_mainPanel.get())->RegisterProject(p_path);
}
