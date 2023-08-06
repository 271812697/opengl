
#include <filesystem>


#include "tools/String.h"

#include <Rendering/Utils/Defines.h>

#include "Editor/Core/ProjectHub.h"
#include "Editor/Core/Application.h"

#undef APIENTRY
#include "Windows.h"
#include "Opengl/core/log.h"

FORCE_DEDICATED_GPU


void UpdateWorkingDirectory(const std::string& p_executablePath)
{
	if (!IsDebuggerPresent())
	{
		std::filesystem::current_path(Tools::Utils::PathParser::GetContainingFolder(p_executablePath));
	}
}

int main(int argc, char** argv);
static void TryRun(const std::string& projectPath, const std::string& projectName);

#ifndef _DEBUG
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	main(__argc, __argv);
}
#endif

int main(int argc, char** argv)
{
	UpdateWorkingDirectory(argv[0]);

	bool ready = false;
	std::string projectPath;
	std::string projectName;

	{
		Editor::Core::ProjectHub hub;

		if (argc < 2)
		{
			// No project file given as argument ==> Open the ProjectHub
			std::tie(ready, projectPath, projectName) = hub.Run();
		}
		else
		{
			// Project file given as argument ==> Open the project
			std::string projectFile = argv[1];

			if (Tools::Utils::PathParser::GetExtension(projectFile) == "ovproject")
			{
				ready = true;
				projectPath = Tools::Utils::PathParser::GetContainingFolder(projectFile);
				projectName = Tools::Utils::PathParser::GetElementName(projectFile);
				Tools::Utils::String::Replace(projectName, ".ovproject", "");
			}

			hub.RegisterProject(projectPath);
		}
	}

	if (ready)
		TryRun(projectPath, projectName);

	return EXIT_SUCCESS;
}

static void TryRun(const std::string& projectPath, const std::string& projectName)
{
	auto errorEvent =
		[](Windowing::Context::EDeviceError, std::string errMsg)
		{
			errMsg = "requires OpenGL 4.3 or newer.\r\n" + errMsg;
			MessageBox(0, errMsg.c_str(), "please", MB_OK | MB_ICONSTOP);
		};

	std::unique_ptr<Editor::Core::Application> app;
	core::Log::Init();
	try
	{
		auto listenerId = Windowing::Context::Device::ErrorEvent += errorEvent;
		app = std::make_unique<Editor::Core::Application>(projectPath, projectName);
		Windowing::Context::Device::ErrorEvent -= listenerId;
	}
	catch (...) {}

	if (app)
		app->Run();
	core::Log::Shutdown();
}
