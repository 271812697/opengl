
#include <tools/Clock.h>

#include "Editor/Core/Application.h"

Editor::Core::Application::Application(const std::string& p_projectPath, const std::string& p_projectName) :
	m_context(p_projectPath, p_projectName),
	m_editor(m_context)
{
}

Editor::Core::Application::~Application()
{
}

void Editor::Core::Application::Run()
{
	Tools::Time::Clock clock;

	while (IsRunning())
	{
		m_editor.PreUpdate();
		m_editor.Update(clock.GetDeltaTime());
		m_editor.PostUpdate();

		clock.Update();
	}
}

bool Editor::Core::Application::IsRunning() const
{
	return !m_context.window->ShouldClose();
}
