

#include "Device.h"

#include <stdexcept>

Tools::Eventing::Event<Windowing::Context::EDeviceError, std::string> Windowing::Context::Device::ErrorEvent;

Windowing::Context::Device::Device(const Settings::DeviceSettings& p_deviceSettings)
{
	BindErrorCallback();

	int initializationCode = glfwInit();

	if (initializationCode == GLFW_FALSE)
	{
		throw std::runtime_error("Failed to Init GLFW");
		glfwTerminate();
	}
	else
	{
		CreateCursors();

		if (p_deviceSettings.debugProfile)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, p_deviceSettings.contextMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, p_deviceSettings.contextMinorVersion);

		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, p_deviceSettings.samples);

		m_isAlive = true;
	}
}

Windowing::Context::Device::~Device()
{
	if (m_isAlive)
	{
		DestroyCursors();
		glfwTerminate();
	}
}

std::pair<int16_t, int16_t> Windowing::Context::Device::GetMonitorSize() const
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	return std::pair<int16_t, int16_t>(static_cast<int16_t>(mode->width), static_cast<int16_t>(mode->height));
}

GLFWcursor * Windowing::Context::Device::GetCursorInstance(Cursor::ECursorShape p_cursorShape) const
{
	return m_cursors.at(p_cursorShape);
}

bool Windowing::Context::Device::HasVsync() const
{
	return m_vsync;
}

void Windowing::Context::Device::SetVsync(bool p_value)
{
	glfwSwapInterval(p_value ? 1 : 0);
	m_vsync = p_value;
}

void Windowing::Context::Device::PollEvents() const
{
	glfwPollEvents();
}

float Windowing::Context::Device::GetElapsedTime() const
{
	return static_cast<float>(glfwGetTime());
}

void Windowing::Context::Device::BindErrorCallback()
{
	auto errorCallback = [](int p_code, const char* p_description)
	{
		ErrorEvent.Invoke(static_cast<EDeviceError>(p_code), p_description);
	};

	glfwSetErrorCallback(errorCallback);
}

void Windowing::Context::Device::CreateCursors()
{
	m_cursors[Cursor::ECursorShape::ARROW] = glfwCreateStandardCursor(static_cast<int>(Cursor::ECursorShape::ARROW));
	m_cursors[Cursor::ECursorShape::IBEAM] = glfwCreateStandardCursor(static_cast<int>(Cursor::ECursorShape::IBEAM));
	m_cursors[Cursor::ECursorShape::CROSSHAIR] = glfwCreateStandardCursor(static_cast<int>(Cursor::ECursorShape::CROSSHAIR));
	m_cursors[Cursor::ECursorShape::HAND] = glfwCreateStandardCursor(static_cast<int>(Cursor::ECursorShape::HAND));
	m_cursors[Cursor::ECursorShape::HRESIZE] = glfwCreateStandardCursor(static_cast<int>(Cursor::ECursorShape::HRESIZE));
	m_cursors[Cursor::ECursorShape::VRESIZE] = glfwCreateStandardCursor(static_cast<int>(Cursor::ECursorShape::VRESIZE));
}

void Windowing::Context::Device::DestroyCursors()
{
	glfwDestroyCursor(m_cursors[Cursor::ECursorShape::ARROW]);
	glfwDestroyCursor(m_cursors[Cursor::ECursorShape::IBEAM]);
	glfwDestroyCursor(m_cursors[Cursor::ECursorShape::CROSSHAIR]);
	glfwDestroyCursor(m_cursors[Cursor::ECursorShape::HAND]);
	glfwDestroyCursor(m_cursors[Cursor::ECursorShape::HRESIZE]);
	glfwDestroyCursor(m_cursors[Cursor::ECursorShape::VRESIZE]);
}
